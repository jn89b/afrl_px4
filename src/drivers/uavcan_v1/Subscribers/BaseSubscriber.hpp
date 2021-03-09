/****************************************************************************
 *
 *   Copyright (c) 2021 PX4 Development Team. All rights reserved.
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
 * @file BaseSubscriber.hpp
 *
 * Defines basic functionality of UAVCAN v1 subscriber class
 *
 * @author Jacob Crabill <jacob@flyvoly.com>
 */

#pragma once

#include <px4_platform_common/px4_config.h>

#include <lib/parameters/param.h>

#include "../CanardInterface.hpp"

class UavcanBaseSubscriber
{
public:
	static constexpr uint16_t CANARD_PORT_ID_UNSET = 65535U;

	UavcanBaseSubscriber(CanardInstance &ins, const char *subject_name, uint8_t instance = 0) :
		_canard_instance(ins), _subject_name(subject_name), _instance(instance) { };

	virtual void subscribe() = 0;
	virtual void unsubscribe() { canardRxUnsubscribe(&_canard_instance, CanardTransferKindMessage, _port_id); };

	virtual void callback(const CanardTransfer &msg) = 0;

	CanardPortID id() { return _port_id; };

	void printInfo()
	{
		if (_port_id != CANARD_PORT_ID_UNSET) {
			PX4_INFO("Subscribed %s.%d on port %d", _subject_name, _instance, _port_id);
		}
	}

protected:
	CanardInstance &_canard_instance;
	CanardRxSubscription _canard_sub;
	const char *_subject_name;
	uint8_t _instance {0};
	/// TODO: 'type' parameter? uavcan.pub.PORT_NAME.type (see 384.Access.1.0.uavcan)

	CanardPortID _port_id {CANARD_PORT_ID_UNSET};
};
