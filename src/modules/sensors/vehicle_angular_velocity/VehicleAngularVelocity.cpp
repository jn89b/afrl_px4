/****************************************************************************
 *
 *   Copyright (c) 2019-2021 PX4 Development Team. All rights reserved.
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

#include "VehicleAngularVelocity.hpp"

#include <px4_platform_common/log.h>

#include <uORB/topics/vehicle_imu_status.h>

using namespace matrix;

namespace sensors
{

VehicleAngularVelocity::VehicleAngularVelocity() :
	ModuleParams(nullptr),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::rate_ctrl)
{
}

VehicleAngularVelocity::~VehicleAngularVelocity()
{
	Stop();
}

bool VehicleAngularVelocity::Start()
{
	// force initial updates
	ParametersUpdate(true);

	// sensor_selection needed to change the active sensor if the primary stops updating
	if (!_sensor_selection_sub.registerCallback()) {
		PX4_ERR("sensor_selection callback registration failed");
		return false;
	}

	if (!SensorSelectionUpdate(true)) {
		_sensor_sub.registerCallback();
	}

	return true;
}

void VehicleAngularVelocity::Stop()
{
	// clear all registered callbacks
	_sensor_sub.unregisterCallback();
	_sensor_selection_sub.unregisterCallback();

	Deinit();
}

bool VehicleAngularVelocity::UpdateSampleRate()
{
	float sample_rate_hz = NAN;
	float publish_rate_hz = NAN;

	for (uint8_t i = 0; i < MAX_SENSOR_COUNT; i++) {
		uORB::SubscriptionData<vehicle_imu_status_s> imu_status{ORB_ID(vehicle_imu_status), i};

		if (imu_status.get().gyro_device_id == _selected_sensor_device_id) {
			sample_rate_hz = imu_status.get().gyro_raw_rate_hz;
			publish_rate_hz = imu_status.get().gyro_rate_hz;
			break;
		}
	}

	// calculate sensor update rate
	if (PX4_ISFINITE(sample_rate_hz) && PX4_ISFINITE(publish_rate_hz)) {
		// check if sample rate error is greater than 1%
		if ((fabsf(sample_rate_hz - _filter_sample_rate_hz) / sample_rate_hz) > 0.01f) {
			PX4_DEBUG("resetting filters, sample rate: %.3f Hz -> %.3f Hz", (double)_filter_sample_rate_hz, (double)sample_rate_hz);
			_reset_filters = true;
			_filter_sample_rate_hz = sample_rate_hz;

			if (_param_imu_gyro_ratemax.get() > 0.f) {
				// determine number of sensor samples that will get closest to the desired rate
				const float configured_interval_us = 1e6f / _param_imu_gyro_ratemax.get();
				const float publish_interval_us = 1e6f / publish_rate_hz;

				const uint8_t samples = roundf(configured_interval_us / publish_interval_us);

				if (_fifo_available) {
					_sensor_fifo_sub.set_required_updates(math::constrain(samples, (uint8_t)1, sensor_gyro_fifo_s::ORB_QUEUE_LENGTH));

				} else {
					_sensor_sub.set_required_updates(math::constrain(samples, (uint8_t)1, sensor_gyro_s::ORB_QUEUE_LENGTH));
				}

				// publish interval (constrained 100 Hz - 8 kHz)
				_publish_interval_min_us = math::constrain((int)roundf(configured_interval_us - (publish_interval_us / 2.f)), 125,
							   10000);

			} else {
				_sensor_sub.set_required_updates(1);
				_sensor_fifo_sub.set_required_updates(1);
				_publish_interval_min_us = 0;
			}
		}

		if (_filter_sample_rate_hz > 0.f) {
			return true;
		}
	}

	return false;
}

void VehicleAngularVelocity::ResetFilters(const Vector3f &angular_velocity, const Vector3f &angular_acceleration)
{
	for (int axis = 0; axis < 3; axis++) {
		// angular velocity low pass
		_lp_filter_velocity[axis].set_cutoff_frequency(_filter_sample_rate_hz, _param_imu_gyro_cutoff.get());
		_lp_filter_velocity[axis].reset(angular_velocity(axis));

		// angular velocity notch
		_notch_filter_velocity[axis].setParameters(_filter_sample_rate_hz, _param_imu_gyro_nf_freq.get(),
				_param_imu_gyro_nf_bw.get());
		_notch_filter_velocity[axis].reset(angular_velocity(axis));

		// angular acceleration low pass
		_lp_filter_acceleration[axis].set_cutoff_frequency(_filter_sample_rate_hz, _param_imu_dgyro_cutoff.get());
		_lp_filter_acceleration[axis].reset(angular_acceleration(axis));
	}

	_reset_filters = false;
}

void VehicleAngularVelocity::SensorBiasUpdate(bool force)
{
	// find corresponding estimated sensor bias
	if (_estimator_selector_status_sub.updated()) {
		estimator_selector_status_s estimator_selector_status;

		if (_estimator_selector_status_sub.copy(&estimator_selector_status)) {
			_estimator_sensor_bias_sub.ChangeInstance(estimator_selector_status.primary_instance);
		}
	}

	if (_estimator_sensor_bias_sub.updated() || force) {
		estimator_sensor_bias_s bias;

		if (_estimator_sensor_bias_sub.copy(&bias) && (bias.gyro_device_id == _selected_sensor_device_id)) {
			_bias = Vector3f{bias.gyro_bias};

		} else {
			_bias.zero();
		}
	}
}

bool VehicleAngularVelocity::SensorSelectionUpdate(bool force)
{
	if (_sensor_selection_sub.updated() || (_selected_sensor_device_id == 0) || force) {
		sensor_selection_s sensor_selection{};
		_sensor_selection_sub.copy(&sensor_selection);

		if (_selected_sensor_device_id != sensor_selection.gyro_device_id) {

			// see if the selected sensor publishes sensor_gyro_fifo
			for (uint8_t i = 0; i < MAX_SENSOR_COUNT; i++) {
				uORB::SubscriptionData<sensor_gyro_fifo_s> sensor_gyro_fifo_sub{ORB_ID(sensor_gyro_fifo), i};

				if ((sensor_gyro_fifo_sub.get().device_id != 0)
				    && (sensor_gyro_fifo_sub.get().device_id == sensor_selection.gyro_device_id)) {
					if (_sensor_fifo_sub.ChangeInstance(i) && _sensor_fifo_sub.registerCallback()) {
						// make sure non-FIFO sub is unregistered
						_sensor_sub.unregisterCallback();

						// record selected sensor
						_selected_sensor_device_id = sensor_selection.gyro_device_id;
						_calibration.set_device_id(sensor_gyro_fifo_sub.get().device_id);

						_reset_filters = true;
						_bias.zero();
						_fifo_available = true;

						return true;
					}
				}
			}

			for (uint8_t i = 0; i < MAX_SENSOR_COUNT; i++) {
				uORB::SubscriptionData<sensor_gyro_s> sensor_gyro_sub{ORB_ID(sensor_gyro), i};

				if ((sensor_gyro_sub.get().device_id != 0)
				    && (sensor_gyro_sub.get().device_id == sensor_selection.gyro_device_id)) {
					if (_sensor_sub.ChangeInstance(i) && _sensor_sub.registerCallback()) {
						// make sure FIFO sub is unregistered
						_sensor_fifo_sub.unregisterCallback();

						// record selected sensor
						_calibration.set_device_id(sensor_gyro_sub.get().device_id);
						_selected_sensor_device_id = sensor_selection.gyro_device_id;

						// clear bias and corrections
						_reset_filters = true;
						_bias.zero();
						_fifo_available = false;

						return true;
					}
				}
			}

			PX4_ERR("unable to find or subscribe to selected sensor (%d)", sensor_selection.gyro_device_id);
			_selected_sensor_device_id = 0;
		}
	}

	return false;
}

void VehicleAngularVelocity::ParametersUpdate(bool force)
{
	// Check if parameters have changed
	if (_parameter_update_sub.updated() || force) {
		// clear update
		parameter_update_s param_update;
		_parameter_update_sub.copy(&param_update);

		updateParams();

		_calibration.ParametersUpdate();

		// gyro low pass cutoff frequency changed
		for (auto &lp : _lp_filter_velocity) {
			if (fabsf(lp.get_cutoff_freq() - _param_imu_gyro_cutoff.get()) > 0.01f) {
				_reset_filters = true;
				break;
			}
		}

		// gyro notch filter frequency or bandwidth changed
		for (auto &nf : _notch_filter_velocity) {
			if ((fabsf(nf.getNotchFreq() - _param_imu_gyro_nf_freq.get()) > 0.01f)
			    || (fabsf(nf.getBandwidth() - _param_imu_gyro_nf_bw.get()) > 0.01f)) {

				_reset_filters = true;
				break;
			}
		}

		// gyro derivative low pass cutoff changed
		for (auto &lp : _lp_filter_acceleration) {
			if (fabsf(lp.get_cutoff_freq() - _param_imu_dgyro_cutoff.get()) > 0.01f) {
				_reset_filters = true;
				break;
			}
		}
	}
}

void VehicleAngularVelocity::Run()
{
	// backup schedule
	ScheduleDelayed(10_ms);

	// update corrections first to set _selected_sensor
	const bool selection_updated = SensorSelectionUpdate();

	_calibration.SensorCorrectionsUpdate(selection_updated);
	SensorBiasUpdate(selection_updated);
	ParametersUpdate();

	if (_fifo_available) {
		// process all outstanding fifo messages
		sensor_gyro_fifo_s sensor_fifo_data;

		while (_sensor_fifo_sub.update(&sensor_fifo_data)) {
			static constexpr int FIFO_SIZE_MAX = sizeof(sensor_fifo_data.x) / sizeof(sensor_fifo_data.x[0]);

			if ((sensor_fifo_data.samples > 0) && (sensor_fifo_data.samples <= FIFO_SIZE_MAX)) {
				const int N = sensor_fifo_data.samples;
				const float dt_s = sensor_fifo_data.dt * 1e-6f;
				const enum Rotation fifo_rotation = static_cast<enum Rotation>(sensor_fifo_data.rotation);

				if (_reset_filters || (fabsf(sensor_fifo_data.scale - _fifo_last_scale) > FLT_EPSILON)) {
					if (UpdateSampleRate()) {
						// in FIFO mode the unscaled raw data is filtered
						_angular_velocity_prev = _angular_velocity / sensor_fifo_data.scale;
						ResetFilters(_angular_velocity_prev, _angular_acceleration / sensor_fifo_data.scale);

						_fifo_last_scale = sensor_fifo_data.scale;
					}

					if (_reset_filters) {
						continue; // not safe to run until filters configured
					}
				}

				Vector3f angular_velocity_unscaled;
				Vector3f angular_acceleration_unscaled;

				int16_t *raw_data_array[] {sensor_fifo_data.x, sensor_fifo_data.y, sensor_fifo_data.z};

				for (int axis = 0; axis < 3; axis++) {
					// copy raw int16 sensor samples to float array for filtering
					float data[FIFO_SIZE_MAX];

					for (int n = 0; n < N; n++) {
						data[n] = raw_data_array[axis][n];
					}

					// Apply general notch filter (IMU_GYRO_NF_FREQ)
					if (_notch_filter_velocity[axis].getNotchFreq() > 0.f) {
						_notch_filter_velocity[axis].apply(data, N);
					}

					// Apply general low-pass filter (IMU_GYRO_CUTOFF)
					_lp_filter_velocity[axis].apply(data, N);

					// save last filtered sample
					angular_velocity_unscaled(axis) = data[N - 1];


					// angular acceleration: Differentiate & apply specific angular acceleration (D-term) low-pass (IMU_DGYRO_CUTOFF)
					float delta_velocity_filtered;

					for (int n = 0; n < N; n++) {
						const float delta_velocity = (data[n] - _angular_velocity_prev(axis));
						delta_velocity_filtered = _lp_filter_acceleration[axis].apply(delta_velocity);
						_angular_velocity_prev(axis) = data[n];
					}

					angular_acceleration_unscaled(axis) = delta_velocity_filtered / dt_s;
				}

				// Angular velocity: rotate sensor frame to board, scale raw data to SI, apply calibration, and remove in-run estimated bias
				rotate_3f(fifo_rotation, angular_velocity_unscaled(0), angular_velocity_unscaled(1), angular_velocity_unscaled(2));
				_angular_velocity = _calibration.Correct(angular_velocity_unscaled * sensor_fifo_data.scale) - _bias;

				// Angular acceleration: rotate sensor frame to board, scale raw data to SI, apply any additional configured rotation
				rotate_3f(fifo_rotation, angular_acceleration_unscaled(0), angular_acceleration_unscaled(1),
					  angular_acceleration_unscaled(2));
				_angular_acceleration = _calibration.rotation() * angular_acceleration_unscaled * sensor_fifo_data.scale;

				// Publish
				if (!_sensor_fifo_sub.updated() && (sensor_fifo_data.timestamp_sample - _last_publish >= _publish_interval_min_us)) {
					Publish(sensor_fifo_data.timestamp_sample);
				}
			}
		}

	} else {
		// process all outstanding messages
		sensor_gyro_s sensor_data;

		while (_sensor_sub.update(&sensor_data)) {
			const float dt_s = math::constrain(((sensor_data.timestamp_sample - _timestamp_sample_last) / 1e6f), 0.0002f, 0.02f);
			_timestamp_sample_last = sensor_data.timestamp_sample;

			if (_reset_filters) {
				if (UpdateSampleRate()) {
					_angular_velocity_prev = _angular_velocity;
					ResetFilters(_angular_velocity, _angular_acceleration);
				}

				if (_reset_filters) {
					continue; // not safe to run until filters configured
				}
			}

			// Apply calibration, rotation, and correct for in-run bias errors
			Vector3f angular_velocity{_calibration.Correct(Vector3f{sensor_data.x, sensor_data.y, sensor_data.z}) - _bias};

			for (int axis = 0; axis < 3; axis++) {
				// Apply general notch filter (IMU_GYRO_NF_FREQ)
				_notch_filter_velocity[axis].apply(&angular_velocity(axis), 1);

				// Apply general low-pass filter (IMU_GYRO_CUTOFF)
				_lp_filter_velocity[axis].apply(&angular_velocity(axis), 1);

				// Differentiate & apply specific angular acceleration (D-term) low-pass (IMU_DGYRO_CUTOFF)
				const float accel = (angular_velocity(axis) - _angular_velocity_prev(axis)) / dt_s;
				_angular_acceleration(axis) = _lp_filter_acceleration[axis].apply(accel);
				_angular_velocity_prev(axis) = angular_velocity(axis);
			}

			_angular_velocity = angular_velocity;

			// Publish
			if (!_sensor_sub.updated() && (sensor_data.timestamp_sample - _last_publish >= _publish_interval_min_us)) {
				Publish(sensor_data.timestamp_sample);
			}
		}
	}
}

void VehicleAngularVelocity::Publish(const hrt_abstime &timestamp_sample)
{
	// Publish vehicle_angular_acceleration
	vehicle_angular_acceleration_s v_angular_acceleration;
	v_angular_acceleration.timestamp_sample = timestamp_sample;
	_angular_acceleration.copyTo(v_angular_acceleration.xyz);
	v_angular_acceleration.timestamp = hrt_absolute_time();
	_vehicle_angular_acceleration_pub.publish(v_angular_acceleration);

	// Publish vehicle_angular_velocity
	vehicle_angular_velocity_s v_angular_velocity;
	v_angular_velocity.timestamp_sample = timestamp_sample;
	_angular_velocity.copyTo(v_angular_velocity.xyz);
	v_angular_velocity.timestamp = hrt_absolute_time();
	_vehicle_angular_velocity_pub.publish(v_angular_velocity);

	// shift last publish time forward, but don't let it get further behind than the interval
	_last_publish = math::constrain(_last_publish + _publish_interval_min_us,
					timestamp_sample - _publish_interval_min_us, timestamp_sample);
}

void VehicleAngularVelocity::PrintStatus()
{
	PX4_INFO("selected sensor: %d, rate: %.1f Hz %s",
		 _selected_sensor_device_id, (double)_filter_sample_rate_hz, _fifo_available ? "FIFO" : "");
	PX4_INFO("estimated bias: [%.4f %.4f %.4f]", (double)_bias(0), (double)_bias(1), (double)_bias(2));

	_calibration.PrintStatus();
}

} // namespace sensors
