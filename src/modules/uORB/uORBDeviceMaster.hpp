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

#pragma once

#include <stdint.h>
#include "uORBCommon.hpp"
#include <lib/cdev/CDev.hpp>

namespace uORB
{
class DeviceNode;
class DeviceMaster;
class Manager;
}

#ifdef __PX4_NUTTX
#include <string.h>
#include <stdlib.h>
#include "ORBMap.hpp"

#else

#include <string>
#include <map>

#endif /* __PX4_NUTTX */

/**
 * Master control device for ObjDev.
 *
 * Used primarily to create new objects via the ORBIOCCREATE
 * ioctl.
 */
class uORB::DeviceMaster : public cdev::CDev
{
public:
	virtual int   ioctl(cdev::file_t *filp, int cmd, unsigned long arg);

	/**
	 * Public interface for getDeviceNodeLocked(). Takes care of synchronization.
	 * @return node if exists, nullptr otherwise
	 */
	uORB::DeviceNode *getDeviceNode(const char *node_name);

	/**
	 * Print statistics for each existing topic.
	 * @param reset if true, reset statistics afterwards
	 */
	void printStatistics(bool reset);

	/**
	 * Continuously print statistics, like the unix top command for processes.
	 * Exited when the user presses the enter key.
	 * @param topic_filter list of topic filters: if set, each string can be a substring for topics to match.
	 *        Or it can be '-a', which means to print all topics instead of only currently publishing ones.
	 * @param num_filters
	 */
	void showTop(char **topic_filter, int num_filters);

private:
	// Private constructor, uORB::Manager takes care of its creation
	DeviceMaster();
	virtual ~DeviceMaster() = default;

	struct DeviceNodeStatisticsData {
		DeviceNode *node;
		uint8_t instance;
		uint32_t last_lost_msg_count;
		unsigned int last_pub_msg_count;
		uint32_t lost_msg_delta;
		unsigned int pub_msg_delta;
		DeviceNodeStatisticsData *next = nullptr;
	};

	void addNewDeviceNodes(DeviceNodeStatisticsData **first_node, int &num_topics, size_t &max_topic_name_length,
			       char **topic_filter, int num_filters);

	friend class uORB::Manager;

	/**
	 * Find a node give its name.
	 * _lock must already be held when calling this.
	 * @return node if exists, nullptr otherwise
	 */
	uORB::DeviceNode *getDeviceNodeLocked(const char *node_name);

#ifdef __PX4_NUTTX
	ORBMap _node_map;
#else
	std::map<std::string, uORB::DeviceNode *> _node_map;
#endif
	hrt_abstime       _last_statistics_output;
};
