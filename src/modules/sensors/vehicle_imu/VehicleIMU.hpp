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

#include <containers/List.hpp>
#include <lib/conversion/rotation.h>
#include <lib/mathlib/math/Limits.hpp>
#include <lib/matrix/matrix/math.hpp>
#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/module_params.h>
#include <px4_platform_common/px4_work_queue/WorkItem.hpp>
#include <uORB/PublicationMulti.hpp>
#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionCallback.hpp>
#include <uORB/topics/parameter_update.h>
#include <uORB/topics/sensor_accel_integrated.h>
#include <uORB/topics/sensor_correction.h>
#include <uORB/topics/sensor_gyro_integrated.h>
#include <uORB/topics/vehicle_imu.h>

class VehicleIMU : public ModuleParams, public px4::WorkItem
{
public:
	VehicleIMU() = delete;
	VehicleIMU(uint8_t accel_index = 0, uint8_t gyro_index = 0);

	~VehicleIMU() override;

	bool Start();
	void Stop();

	void PrintStatus();

private:
	void Run() override;

	void ParametersUpdate(bool force = false);
	void SensorCorrectionsUpdate(bool force = false);

	static constexpr int MAX_SENSOR_COUNT = 3;

	DEFINE_PARAMETERS(
		(ParamInt<px4::params::SENS_BOARD_ROT>) _param_sens_board_rot,

		(ParamFloat<px4::params::SENS_BOARD_X_OFF>) _param_sens_board_x_off,
		(ParamFloat<px4::params::SENS_BOARD_Y_OFF>) _param_sens_board_y_off,
		(ParamFloat<px4::params::SENS_BOARD_Z_OFF>) _param_sens_board_z_off
	)

	uORB::PublicationMulti<vehicle_imu_s> _vehicle_imu_pub{ORB_ID(vehicle_imu)};

	uORB::Subscription _params_sub{ORB_ID(parameter_update)};
	uORB::Subscription _sensor_correction_sub{ORB_ID(sensor_correction)};

	uORB::SubscriptionCallbackWorkItem _sensor_accel_integrated_sub;
	uORB::SubscriptionCallbackWorkItem _sensor_gyro_integrated_sub;

	matrix::Dcmf _board_rotation;

	matrix::Vector3f _accel_offset{0.f, 0.f, 0.f};
	matrix::Vector3f _gyro_offset{0.f, 0.f, 0.f};
	matrix::Vector3f _accel_scale{1.f, 1.f, 1.f};
	matrix::Vector3f _gyro_scale{1.f, 1.f, 1.f};

	char *_name{nullptr};

	int8_t _corrections_selected_accel_instance{-1};
	int8_t _corrections_selected_gyro_instance{-1};

	uint32_t _accel_device_id{0};
	uint32_t _gyro_device_id{0};
};
