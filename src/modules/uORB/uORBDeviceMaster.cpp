/****************************************************************************
 *
 *   Copyright (c) 2012-2016 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "uORBDeviceMaster.hpp"
#include "uORBDeviceNode.hpp"
#include "uORBManager.hpp"
#include "uORBUtils.hpp"

#ifdef ORB_COMMUNICATOR
#include "uORBCommunicator.hpp"
#endif /* ORB_COMMUNICATOR */

#include <px4_sem.hpp>
#include <systemlib/px4_macros.h>

#ifdef __PX4_NUTTX
#define ITERATE_NODE_MAP() \
	for (ORBMap::Node *node_iter = _node_map.top(); node_iter; node_iter = node_iter->next)
#define INIT_NODE_MAP_VARS(node_obj, node_name_str) \
	DeviceNode *node_obj = node_iter->node; \
	const char *node_name_str = node_iter->node_name; \
	UNUSED(node_name_str);

#else
#include <algorithm>
#define ITERATE_NODE_MAP() \
	for (const auto &node_iter : _node_map)
#define INIT_NODE_MAP_VARS(node_obj, node_name_str) \
	DeviceNode *node_obj = node_iter.second; \
	const char *node_name_str = node_iter.first.c_str(); \
	UNUSED(node_name_str);
#endif

uORB::DeviceMaster::DeviceMaster() :
	CDev(TOPIC_MASTER_DEVICE_PATH)
{
	_last_statistics_output = hrt_absolute_time();
}

int
uORB::DeviceMaster::ioctl(cdev::file_t *filp, int cmd, unsigned long arg)
{
	int ret;

	switch (cmd) {
	case ORBIOCADVERTISE: {
			const struct orb_advertdata *adv = (const struct orb_advertdata *)arg;
			const struct orb_metadata *meta = adv->meta;
			char nodepath[orb_maxpath];

			/* construct a path to the node - this also checks the node name */
			ret = uORB::Utils::node_mkpath(nodepath, meta, adv->instance);

			if (ret != PX4_OK) {
				return ret;
			}

			ret = PX4_ERROR;

			/* try for topic groups */
			const unsigned max_group_tries = (adv->instance != nullptr) ? ORB_MULTI_MAX_INSTANCES : 1;
			unsigned group_tries = 0;

			if (adv->instance) {
				/* for an advertiser, this will be 0, but a for subscriber that requests a certain instance,
				 * we do not want to start with 0, but with the instance the subscriber actually requests.
				 */
				group_tries = *adv->instance;

				if (group_tries >= max_group_tries) {
					return -ENOMEM;
				}
			}

			SmartLock smart_lock(_lock);

			do {
				/* if path is modifyable change try index */
				if (adv->instance != nullptr) {
					/* replace the number at the end of the string */
					nodepath[strlen(nodepath) - 1] = '0' + group_tries;
					*(adv->instance) = group_tries;
				}

				/* driver wants a permanent copy of the path, so make one here */
				const char *devpath = strdup(nodepath);

				if (devpath == nullptr) {
					return -ENOMEM;
				}

				/* construct the new node */
				uORB::DeviceNode *node = new uORB::DeviceNode(meta, devpath, adv->priority);

				/* if we didn't get a device, that's bad */
				if (node == nullptr) {
					free((void *)devpath);
					return -ENOMEM;
				}

				/* initialise the node - this may fail if e.g. a node with this name already exists */
				ret = node->init();

				/* if init failed, discard the node and its name */
				if (ret != PX4_OK) {
					delete node;

					if (ret == -EEXIST) {
						/* if the node exists already, get the existing one and check if
						 * something has been published yet. */
						uORB::DeviceNode *existing_node = getDeviceNodeLocked(devpath);

						if ((existing_node != nullptr) && !(existing_node->is_published())) {
							/* nothing has been published yet, lets claim it */
							existing_node->set_priority(adv->priority);
							ret = PX4_OK;

						} else {
							/* otherwise: data has already been published, keep looking */
						}
					}

					/* also discard the name now */
					free((void *)devpath);

				} else {
					// add to the node map;.
#ifdef __PX4_NUTTX
					_node_map.insert(devpath, node);
#else
					_node_map[std::string(devpath)] = node;
#endif
				}

				group_tries++;

			} while (ret != PX4_OK && (group_tries < max_group_tries));

			if (ret != PX4_OK && group_tries >= max_group_tries) {
				ret = -ENOMEM;
			}

			return ret;
		}

	default:
		/* give it to the superclass */
		return CDev::ioctl(filp, cmd, arg);
	}
}

void uORB::DeviceMaster::printStatistics(bool reset)
{
	hrt_abstime current_time = hrt_absolute_time();
	PX4_INFO("Statistics, since last output (%i ms):", (int)((current_time - _last_statistics_output) / 1000));
	_last_statistics_output = current_time;

	PX4_INFO("TOPIC, NR LOST MSGS");
	bool had_print = false;

	lock();
	ITERATE_NODE_MAP() {
		INIT_NODE_MAP_VARS(node, node_name)

		if (node->print_statistics(reset)) {
			had_print = true;
		}
	}

	unlock();

	if (!had_print) {
		PX4_INFO("No lost messages");
	}
}

void uORB::DeviceMaster::addNewDeviceNodes(DeviceNodeStatisticsData **first_node, int &num_topics,
		size_t &max_topic_name_length, char **topic_filter, int num_filters)
{
	DeviceNodeStatisticsData *cur_node;
	num_topics = 0;
	DeviceNodeStatisticsData *last_node = *first_node;

	if (last_node) {
		while (last_node->next) {
			last_node = last_node->next;
		}
	}

	ITERATE_NODE_MAP() {
		INIT_NODE_MAP_VARS(node, node_name)
		++num_topics;

		//check if already added
		cur_node = *first_node;

		while (cur_node && cur_node->node != node) {
			cur_node = cur_node->next;
		}

		if (cur_node) {
			continue;
		}

		if (num_filters > 0 && topic_filter) {
			bool matched = false;

			for (int i = 0; i < num_filters; ++i) {
				if (strstr(node->get_meta()->o_name, topic_filter[i])) {
					matched = true;
				}
			}

			if (!matched) {
				continue;
			}
		}

		if (last_node) {
			last_node->next = new DeviceNodeStatisticsData();
			last_node = last_node->next;

		} else {
			*first_node = last_node = new DeviceNodeStatisticsData();
		}

		if (!last_node) {
			PX4_ERR("mem alloc failed");
			break;
		}

		last_node->node = node;
		int node_name_len = strlen(node_name);
		last_node->instance = (uint8_t)(node_name[node_name_len - 1] - '0');
		size_t name_length = strlen(last_node->node->get_meta()->o_name);

		if (name_length > max_topic_name_length) {
			max_topic_name_length = name_length;
		}

		last_node->last_lost_msg_count = last_node->node->lost_message_count();
		last_node->last_pub_msg_count = last_node->node->published_message_count();
	}
}

#define CLEAR_LINE "\033[K"

void uORB::DeviceMaster::showTop(char **topic_filter, int num_filters)
{

	bool print_active_only = true;

	if (topic_filter && num_filters > 0) {
		if (!strcmp("-a", topic_filter[0])) {
			num_filters = 0;
		}

		print_active_only = false; // print non-active if -a or some filter given
	}

	PX4_INFO_RAW("\033[2J\n"); //clear screen

	lock();

	if (_node_map.empty()) {
		unlock();
		PX4_INFO("no active topics");
		return;
	}

	DeviceNodeStatisticsData *first_node = nullptr;
	DeviceNodeStatisticsData *cur_node = nullptr;
	size_t max_topic_name_length = 0;
	int num_topics = 0;
	addNewDeviceNodes(&first_node, num_topics, max_topic_name_length, topic_filter, num_filters);

	/* a DeviceNode is never deleted, so it's save to unlock here and still access the DeviceNodes */
	unlock();

#ifdef __PX4_QURT //QuRT has no poll()
	int num_runs = 0;
#else
	const int stdin_fileno = 0;

	struct pollfd fds;
	fds.fd = stdin_fileno;
	fds.events = POLLIN;
#endif
	bool quit = false;

	hrt_abstime start_time = hrt_absolute_time();

	while (!quit) {

#ifdef __PX4_QURT

		if (++num_runs > 1) {
			quit = true; //just exit after one output
		}

#else

		/* Sleep 200 ms waiting for user input five times ~ 1s */
		for (int k = 0; k < 5; k++) {
			char c;

			int ret = ::poll(&fds, 1, 0); //just want to check if there is new data available

			if (ret > 0) {

				ret = ::read(stdin_fileno, &c, 1);

				if (ret) {
					quit = true;
					break;
				}
			}

			usleep(200000);
		}

#endif

		if (!quit) {

			//update the stats
			hrt_abstime current_time = hrt_absolute_time();
			float dt = (current_time - start_time) / 1.e6f;
			cur_node = first_node;

			while (cur_node) {
				uint32_t num_lost = cur_node->node->lost_message_count();
				unsigned int num_msgs = cur_node->node->published_message_count();
				cur_node->pub_msg_delta = (num_msgs - cur_node->last_pub_msg_count) / dt;
				cur_node->lost_msg_delta = (num_lost - cur_node->last_lost_msg_count) / dt;
				cur_node->last_lost_msg_count = num_lost;
				cur_node->last_pub_msg_count = num_msgs;
				cur_node = cur_node->next;
			}

			start_time = current_time;


			PX4_INFO_RAW("\033[H"); // move cursor home and clear screen
			PX4_INFO_RAW(CLEAR_LINE "update: 1s, num topics: %i\n", num_topics);
#ifdef __PX4_NUTTX
			PX4_INFO_RAW(CLEAR_LINE "%*-s INST #SUB #MSG #LOST #QSIZE\n", (int)max_topic_name_length - 2, "TOPIC NAME");
#else
			PX4_INFO_RAW(CLEAR_LINE "%*s INST #SUB #MSG #LOST #QSIZE\n", -(int)max_topic_name_length + 2, "TOPIC NAME");
#endif
			cur_node = first_node;

			while (cur_node) {

				if (!print_active_only || cur_node->pub_msg_delta > 0) {
#ifdef __PX4_NUTTX
					PX4_INFO_RAW(CLEAR_LINE "%*-s %2i %4i %4i %5i %i\n", (int)max_topic_name_length,
#else
					PX4_INFO_RAW(CLEAR_LINE "%*s %2i %4i %4i %5i %i\n", -(int)max_topic_name_length,
#endif
						     cur_node->node->get_meta()->o_name, (int)cur_node->instance,
						     (int)cur_node->node->subscriber_count(), cur_node->pub_msg_delta,
						     (int)cur_node->lost_msg_delta, cur_node->node->get_queue_size());
				}

				cur_node = cur_node->next;
			}

			lock();
			addNewDeviceNodes(&first_node, num_topics, max_topic_name_length, topic_filter, num_filters);
			unlock();
		}
	}

	//cleanup
	cur_node = first_node;

	while (cur_node) {
		DeviceNodeStatisticsData *next_node = cur_node->next;
		delete cur_node;
		cur_node = next_node;
	}
}

#undef CLEAR_LINE

uORB::DeviceNode *uORB::DeviceMaster::getDeviceNode(const char *nodepath)
{
	lock();
	uORB::DeviceNode *node = getDeviceNodeLocked(nodepath);
	unlock();
	//We can safely return the node that can be used by any thread, because
	//a DeviceNode never gets deleted.
	return node;
}


#ifdef __PX4_NUTTX
uORB::DeviceNode *uORB::DeviceMaster::getDeviceNodeLocked(const char *nodepath)
{
	uORB::DeviceNode *rc = nullptr;

	if (_node_map.find(nodepath)) {
		rc = _node_map.get(nodepath);
	}

	return rc;
}

#else

uORB::DeviceNode *uORB::DeviceMaster::getDeviceNodeLocked(const char *nodepath)
{
	uORB::DeviceNode *rc = nullptr;
	std::string np(nodepath);

	auto iter = _node_map.find(np);

	if (iter != _node_map.end()) {
		rc = iter->second;
	}

	return rc;
}
#endif
