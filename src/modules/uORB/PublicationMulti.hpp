/****************************************************************************
 *
 *   Copyright (c) 2019 PX4 Development Team. All rights reserved.
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

/**
 * @file Publication.hpp
 *
 */

#pragma once

#include <px4_platform_common/defines.h>
#include <systemlib/err.h>
#include <uORB/uORB.h>

#include "Publication.hpp"

namespace uORB
{

/**
 * Base publication multi wrapper class
 */
template<typename T, uint8_t QSIZE = 1>
class PublicationMulti : public PublicationBase
{
public:

	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param priority The priority for multi pub/sub, 0 means don't publish as multi
	 */
	PublicationMulti(ORB_ID id, ORB_PRIO priority = ORB_PRIO_DEFAULT) :
		PublicationBase(id),
		_priority(priority)
	{}

	PublicationMulti(const orb_metadata *meta, ORB_PRIO priority = ORB_PRIO_DEFAULT) :
		PublicationBase(static_cast<ORB_ID>(meta->o_id)),
		_priority(priority)
	{}

	bool advertise()
	{
		if (!advertised()) {
			int instance = 0;
			_handle = orb_advertise_multi_queue(get_topic(), nullptr, &instance, _priority, QSIZE);
		}

		return advertised();
	}

	/**
	 * Publish the struct
	 * @param data The uORB message struct we are updating.
	 */
	bool publish(const T &data)
	{
		if (!advertised()) {
			advertise();
		}

		return (orb_publish(get_topic(), _handle, &data) == PX4_OK);
	}

protected:
	const ORB_PRIO _priority;
};

/**
 * The publication multi class with data embedded.
 */
template<typename T>
class PublicationMultiData : public PublicationMulti<T>
{
public:
	/**
	 * Constructor
	 *
	 * @param meta The uORB metadata (usually from the ORB_ID() macro) for the topic.
	 * @param priority The priority for multi pub
	 */
	PublicationMultiData(ORB_ID id, ORB_PRIO priority = ORB_PRIO_DEFAULT) :
		PublicationMulti<T>(id, priority)
	{}
	PublicationMultiData(const orb_metadata *meta, ORB_PRIO priority = ORB_PRIO_DEFAULT) :
		PublicationMulti<T>(meta, priority)
	{}

	T	&get() { return _data; }
	void	set(const T &data) { _data = data; }

	// Publishes the embedded struct.
	bool	update() { return PublicationMulti<T>::publish(_data); }
	bool	update(const T &data)
	{
		_data = data;
		return PublicationMulti<T>::publish(_data);
	}

private:
	T _data{};
};


template<class T>
using PublicationQueuedMulti = PublicationMulti<T, T::ORB_QUEUE_LENGTH>;

} // namespace uORB
