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
 * @file Subscriber.hpp
 *
 * Defines basic functionality of UAVCAN v1 subscriber class
 *
 * @author Jacob Crabill <jacob@flyvoly.com>
 */

#pragma once

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/atomic.h>
#include <px4_platform_common/defines.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/module_params.h>

#include <lib/parameters/param.h>

#include "o1heap/o1heap.h"

#include <canard.h>
#include <canard_dsdl.h>

#include <uavcan/_register/Access_1_0.h>
#include <uavcan/_register/List_1_0.h>
#include <uavcan/_register/Value_1_0.h>
#include <uavcan/primitive/Empty_1_0.h>

//Quick and Dirty PNP imlementation only V1 for now as well
#include <uavcan/node/ID_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_2_0.h>

// DS-15 Specification Messages
#include <reg/drone/physics/kinematics/geodetic/Point_0_1.h>
#include <reg/drone/service/battery/Parameters_0_1.h>
#include <reg/drone/service/battery/Status_0_1.h>

#include "CanardInterface.hpp"
#include "ParamManager.hpp"

class UavcanSubscription
{
public:
	UavcanSubscription(CanardInstance &ins, UavcanParamManager &pmgr, const char *uavcan_pname) :
		_canard_instance(ins), _param_manager(pmgr), _uavcan_param(uavcan_pname) { };

	virtual void subscribe() = 0;
	virtual void unsubscribe() { canardRxUnsubscribe(&_canard_instance, CanardTransferKindMessage, _port_id); };

	virtual void callback(const CanardTransfer &msg) = 0;

	CanardPortID id() { return _port_id; };

	void updateParam()
	{
		// Set _port_id from _uavcan_param

		uavcan_register_Value_1_0 value;
		_param_manager.GetParamByName(_uavcan_param, value);
		int32_t new_id = value.integer32.value.elements[0];

		if (_port_id != new_id) {
			if (new_id == 0) {
				// Cancel subscription
				unsubscribe();

			} else {
				if (_port_id > 0) {
					// Already active; unsubscribe first
					unsubscribe();
				}

				// Subscribe on the new port ID
				_port_id = (CanardPortID)new_id;
				PX4_INFO("Subscribing %s on port %d", _uavcan_param, _port_id);
				subscribe();
			}
		}
	};

	void printInfo()
	{
		if (_port_id > 0) {
			PX4_INFO("Subscribed %s on port %d", _uavcan_param, _port_id);
		}
	}

protected:
	CanardInstance &_canard_instance;
	UavcanParamManager &_param_manager;
	CanardRxSubscription _canard_sub;
	const char *_uavcan_param; // Port ID parameter
	/// TODO: 'type' parameter? uavcan.pub.PORT_NAME.type (see 384.Access.1.0.uavcan)

	CanardPortID _port_id {0};
};

class UavcanGpsSubscription : public UavcanSubscription
{
public:
	UavcanGpsSubscription(CanardInstance &ins, UavcanParamManager &pmgr, const char *uavcan_pname) :
		UavcanSubscription(ins, pmgr, uavcan_pname) { };

	void subscribe() override;

	void callback(const CanardTransfer &msg) override;

private:

};

class UavcanBmsSubscription : public UavcanSubscription
{
public:
	UavcanBmsSubscription(CanardInstance &ins, UavcanParamManager &pmgr, const char *uavcan_pname) :
		UavcanSubscription(ins, pmgr, uavcan_pname) { };

	void subscribe() override;

	void callback(const CanardTransfer &msg) override;

private:

};
