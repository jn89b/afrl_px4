/****************************************************************************
 *
 *   Copyright (c) 2020 PX4 Development Team. All rights reserved.
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

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/atomic.h>
#include <px4_platform_common/defines.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/module_params.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>

#include <lib/parameters/param.h>
#include <lib/perf/perf_counter.h>
#include <uORB/Publication.hpp>
#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionCallback.hpp>
#include <uORB/topics/battery_status.h>
#include <uORB/topics/parameter_update.h>
#include <uORB/topics/sensor_gps.h>

#include "o1heap/o1heap.h"

#include <canard.h>
#include <canard_dsdl.h>

#include <reg/drone/srv/battery/Status_0_1.h>
#include <reg/drone/srv/battery/Parameters_0_1.h>
#include <uavcan/node/Heartbeat_1_0.h>

//Quick and Dirty PNP imlementation only V1 for now as well
#include <uavcan/node/ID_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_1_0.h>
#include <uavcan/pnp/NodeIDAllocationData_2_0.h>

//Quick and Dirty UAVCAN register implementation
#include <uavcan/_register/List_1_0.h>
#include <uavcan/_register/Access_1_0.h>

#define PNP1_PORT_ID                                 uavcan_pnp_NodeIDAllocationData_1_0_FIXED_PORT_ID_
#define PNP1_PAYLOAD_SIZE                            uavcan_pnp_NodeIDAllocationData_1_0_EXTENT_BYTES_
#define PNP2_PORT_ID                                 uavcan_pnp_NodeIDAllocationData_2_0_FIXED_PORT_ID_
#define PNP2_PAYLOAD_SIZE                            uavcan_pnp_NodeIDAllocationData_2_0_EXTENT_BYTES_

#include "CanardInterface.hpp"

class UavcanNode : public ModuleParams, public px4::ScheduledWorkItem
{
	/*
	* This memory is allocated for the 01Heap allocator used by
	* libcanard to store incoming/outcoming data
	* Current size of 8192 bytes is arbitrary, should be optimized further
	* when more nodes and messages are on the CAN bus
	*/
	static constexpr unsigned HeapSize = 8192;

	/*
	 * Base interval, has to be complemented with events from the CAN driver
	 * and uORB topics sending data, to decrease response time.
	 */
	static constexpr unsigned ScheduleIntervalMs = 10;

public:

	UavcanNode(CanardInterface *interface, uint32_t node_id);
	~UavcanNode() override;

	static int start(uint32_t node_id, uint32_t bitrate);

	void print_info();

	static UavcanNode *instance() { return _instance; }

	/* The bit rate that can be passed back to the bootloader */
	int32_t active_bitrate{0};

private:
	void Run() override;
	void fill_node_info();

	void *_uavcan_heap{nullptr};

	CanardInterface *const _can_interface;

	CanardInstance _canard_instance;

	px4::atomic_bool _task_should_exit{false};	///< flag to indicate to tear down the CAN driver

	bool _initialized{false};		///< number of actuators currently available

	static UavcanNode *_instance;

	pthread_mutex_t _node_mutex;

	CanardRxSubscription _heartbeat_subscription;
	CanardRxSubscription _pnp_v1_subscription;
	CanardRxSubscription _drone_srv_battery_subscription;
	CanardRxSubscription _drone_srv_gps_subscription;
	CanardRxSubscription _register_access_subscription;
	CanardRxSubscription _register_list_subscription;

	uORB::Subscription _battery_status_sub{ORB_ID(battery_status)};
	uORB::Subscription _parameter_update_sub{ORB_ID(parameter_update)};

	uORB::Publication<battery_status_s> _battery_status_pub{ORB_ID(battery_status)};
	uORB::Publication<sensor_gps_s> _sensor_gps_pub{ORB_ID(sensor_gps)};

	perf_counter_t _cycle_perf{perf_alloc(PC_ELAPSED, MODULE_NAME": cycle time")};
	perf_counter_t _interval_perf{perf_alloc(PC_INTERVAL, MODULE_NAME": cycle interval")};

	// uavcan::node::Heartbeat_1_0
	uint8_t _uavcan_node_heartbeat_buffer[uavcan_node_Heartbeat_1_0_EXTENT_BYTES_];
	hrt_abstime _uavcan_node_heartbeat_last{0};
	CanardTransferID _uavcan_node_heartbeat_transfer_id{0};

	/* Temporary hardcoded port IDs used by the register interface
	* for demo purposes untill we have nice interface (QGC or latter)
	* to configure the nodes
	*/
	const uint16_t bms_port_id = 1234;
	const uint16_t gps_port_id = 1235;

	CanardTransferID _uavcan_pnp_nodeidallocation_v1_transfer_id{0};
	hrt_abstime _uavcan_pnp_nodeidallocation_last{0};

	CanardTransferID _uavcan_register_list_request_transfer_id{0};
	CanardTransferID _uavcan_register_access_request_transfer_id{0};
	//Register interface NodeID TODO MVP right have to make a queue
	uint8_t _node_register_setup = CANARD_NODE_ID_UNSET;
	int32_t _node_register_request_index = 0;
	int32_t _node_register_last_received_index = -1;

	// regulated::drone::sensor::BMSStatus_1_0
	uint8_t _regulated_drone_sensor_bmsstatus_buffer[reg_drone_srv_battery_Status_0_1_EXTENT_BYTES_];
	hrt_abstime _regulated_drone_sensor_bmsstatus_last{0};
	CanardTransferID _regulated_drone_sensor_bmsstatus_transfer_id{0};

	DEFINE_PARAMETERS(
		(ParamInt<px4::params::UAVCAN_V1_ENABLE>) _param_uavcan_v1_enable,
		(ParamInt<px4::params::UAVCAN_V1_ID>) _param_uavcan_v1_id,
		(ParamInt<px4::params::UAVCAN_V1_BAUD>) _param_uavcan_v1_baud,
		(ParamInt<px4::params::UAVCAN_V1_BAT_MD>) _param_uavcan_v1_bat_md,
		(ParamInt<px4::params::UAVCAN_V1_BAT_ID>) _param_uavcan_v1_bat_id
	)
};
