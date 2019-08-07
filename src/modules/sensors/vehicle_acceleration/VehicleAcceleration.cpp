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

#include "VehicleAcceleration.hpp"

#include <px4_log.h>

using namespace matrix;
using namespace time_literals;

VehicleAcceleration::VehicleAcceleration() :
	ModuleParams(nullptr),
	WorkItem(px4::wq_configurations::att_pos_ctrl),
	_cycle_perf(perf_alloc(PC_ELAPSED, "vehicle_acceleration: cycle time")),
	_interval_perf(perf_alloc(PC_INTERVAL, "vehicle_acceleration: interval")),
	_sensor_latency_perf(perf_alloc(PC_ELAPSED, "vehicle_acceleration: sensor latency"))
{
}

VehicleAcceleration::~VehicleAcceleration()
{
	Stop();

	perf_free(_cycle_perf);
	perf_free(_interval_perf);
	perf_free(_sensor_latency_perf);
}

bool
VehicleAcceleration::Start()
{
	// initialize thermal corrections as we might not immediately get a topic update (only non-zero values)
	_scale = Vector3f{1.0f, 1.0f, 1.0f};
	_offset.zero();
	_bias.zero();

	// force initial updates
	ParametersUpdate(true);
	SensorBiasUpdate(true);

	// needed to change the active sensor if the primary stops updating
	_sensor_selection_sub.register_callback();

	return SensorCorrectionsUpdate(true);
}

void
VehicleAcceleration::Stop()
{
	Deinit();

	// clear all registered callbacks
	for (auto sub : _sensor_sub) {
		sub.unregister_callback();
	}

	_sensor_selection_sub.unregister_callback();
}

void
VehicleAcceleration::SensorBiasUpdate(bool force)
{
	if (_sensor_bias_sub.updated() || force) {
		sensor_bias_s bias;

		if (_sensor_bias_sub.copy(&bias)) {
			// TODO: should be checking device ID
			_bias = Vector3f{bias.accel_bias};
		}
	}
}

bool
VehicleAcceleration::SensorCorrectionsUpdate(bool force)
{
	// check if the selected sensor has updated
	if (_sensor_correction_sub.updated() || force) {

		sensor_correction_s corrections{};
		_sensor_correction_sub.copy(&corrections);

		// TODO: should be checking device ID
		if (_selected_sensor == 0) {
			_offset = Vector3f{corrections.accel_offset_0};
			_scale = Vector3f{corrections.accel_scale_0};

		} else if (_selected_sensor == 1) {
			_offset = Vector3f{corrections.accel_offset_1};
			_scale = Vector3f{corrections.accel_scale_1};

		} else if (_selected_sensor == 2) {
			_offset = Vector3f{corrections.accel_offset_2};
			_scale = Vector3f{corrections.accel_scale_2};

		} else {
			_offset = Vector3f{0.0f, 0.0f, 0.0f};
			_scale = Vector3f{1.0f, 1.0f, 1.0f};
		}

		// update the latest sensor selection
		if ((_selected_sensor != corrections.selected_accel_instance) || force) {
			if (corrections.selected_accel_instance < MAX_SENSOR_COUNT) {
				// clear all registered callbacks
				for (auto sub : _sensor_sub) {
					sub.unregister_callback();
				}

				const int sensor_new = corrections.selected_accel_instance;

				if (_sensor_sub[sensor_new].register_callback()) {
					PX4_DEBUG("selected sensor changed %d -> %d", _selected_sensor, sensor_new);
					_selected_sensor = sensor_new;

					return true;
				}
			}
		}
	}

	return false;
}

void
VehicleAcceleration::ParametersUpdate(bool force)
{
	// Check if parameters have changed
	if (_params_sub.updated() || force) {
		// clear update
		parameter_update_s param_update;
		_params_sub.copy(&param_update);

		updateParams();

		// get transformation matrix from sensor/board to body frame
		const matrix::Dcmf board_rotation = get_rot_matrix((enum Rotation)_param_sens_board_rot.get());

		// fine tune the rotation
		const Dcmf board_rotation_offset(Eulerf(
				math::radians(_param_sens_board_x_off.get()),
				math::radians(_param_sens_board_y_off.get()),
				math::radians(_param_sens_board_z_off.get())));

		_board_rotation = board_rotation_offset * board_rotation;
	}
}

void
VehicleAcceleration::Run()
{
	perf_begin(_cycle_perf);
	perf_count(_interval_perf);

	// update corrections first to set _selected_sensor
	SensorCorrectionsUpdate();

	sensor_accel_s sensor_data;

	if (_sensor_sub[_selected_sensor].update(&sensor_data)) {
		perf_set_elapsed(_sensor_latency_perf, hrt_elapsed_time(&sensor_data.timestamp));

		ParametersUpdate();
		SensorBiasUpdate();

		// get the sensor data and correct for thermal errors
		const Vector3f val{sensor_data.x, sensor_data.y, sensor_data.z};

		// apply offsets and scale
		Vector3f accel{(val - _offset).emult(_scale)};

		// rotate corrected measurements from sensor to body frame
		accel = _board_rotation * accel;

		// correct for in-run bias errors
		accel -= _bias;

		vehicle_acceleration_s out{};
		out.timestamp_sample = sensor_data.timestamp;
		accel.copyTo(out.xyz);
		out.timestamp = hrt_absolute_time();

		_vehicle_acceleration_pub.publish(out);
	}

	perf_end(_cycle_perf);
}

void
VehicleAcceleration::PrintStatus()
{
	PX4_INFO("selected sensor: %d", _selected_sensor);

	perf_print_counter(_cycle_perf);
	perf_print_counter(_interval_perf);
	perf_print_counter(_sensor_latency_perf);
}
