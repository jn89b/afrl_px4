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

#include "VehicleIMU.hpp"

#include <px4_platform_common/log.h>

#include <float.h>

using namespace matrix;
using namespace time_literals;

using math::constrain;

namespace sensors
{

VehicleIMU::VehicleIMU(uint8_t accel_index, uint8_t gyro_index) :
	ModuleParams(nullptr),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::navigation_and_controllers),
	_sensor_accel_sub(this, ORB_ID(sensor_accel), accel_index),
	_sensor_gyro_sub(this, ORB_ID(sensor_gyro), gyro_index),
	_accel_corrections(this, SensorCorrections::SensorType::Accelerometer),
	_gyro_corrections(this, SensorCorrections::SensorType::Gyroscope)
{
	const float configured_interval_us = 1e6f / _param_imu_integ_rate.get();

	_accel_integrator.set_reset_interval(configured_interval_us);
	_accel_integrator.set_reset_samples(sensor_accel_s::ORB_QUEUE_LENGTH);
	_sensor_accel_sub.set_required_updates(1);

	_gyro_integrator.set_reset_interval(configured_interval_us);
	_gyro_integrator.set_reset_samples(sensor_gyro_s::ORB_QUEUE_LENGTH);
	_sensor_gyro_sub.set_required_updates(1);

	// advertise immediately to ensure consistent ordering
	_vehicle_imu_pub.advertise();
	_vehicle_imu_status_pub.advertise();
}

VehicleIMU::~VehicleIMU()
{
	Stop();

	perf_free(_accel_generation_gap_perf);
	perf_free(_accel_update_perf);
	perf_free(_gyro_generation_gap_perf);
	perf_free(_gyro_update_perf);
}

bool VehicleIMU::Start()
{
	// force initial updates
	ParametersUpdate(true);

	return _sensor_gyro_sub.registerCallback() && _sensor_accel_sub.registerCallback();
}

void VehicleIMU::Stop()
{
	// clear all registered callbacks
	_sensor_accel_sub.unregisterCallback();
	_sensor_gyro_sub.unregisterCallback();

	Deinit();
}

void VehicleIMU::ParametersUpdate(bool force)
{
	// Check if parameters have changed
	if (_params_sub.updated() || force) {
		// clear update
		parameter_update_s param_update;
		_params_sub.copy(&param_update);

		updateParams();

		_accel_corrections.ParametersUpdate();
		_gyro_corrections.ParametersUpdate();

		// constrain IMU integration time 1-20 milliseconds (50-1000 Hz)
		int32_t imu_integration_rate_hz = constrain(_param_imu_integ_rate.get(), 50, 1000);

		if (imu_integration_rate_hz != _param_imu_integ_rate.get()) {
			_param_imu_integ_rate.set(imu_integration_rate_hz);
			_param_imu_integ_rate.commit_no_notification();
		}
	}
}

bool VehicleIMU::UpdateIntervalAverage(IntervalAverage &intavg, const hrt_abstime &timestamp_sample)
{
	bool updated = false;

	if ((intavg.timestamp_sample_last > 0) && (timestamp_sample > intavg.timestamp_sample_last)) {
		intavg.interval_sum += (timestamp_sample - intavg.timestamp_sample_last);
		intavg.interval_count++;

		// periodically calculate sensor update rate
		if (intavg.interval_count > 10000 || ((intavg.update_interval <= FLT_EPSILON) && intavg.interval_count > 100)) {

			const float sample_interval_avg = intavg.interval_sum / intavg.interval_count;

			if (PX4_ISFINITE(sample_interval_avg) && (sample_interval_avg > 0.f)) {
				// update if interval has changed by more than 0.5%
				if ((fabsf(intavg.update_interval - sample_interval_avg) / intavg.update_interval) > 0.005f) {

					intavg.update_interval = sample_interval_avg;
					updated = true;
				}
			}

			// reset sample interval accumulator
			intavg.interval_sum = 0.f;
			intavg.interval_count = 0.f;
		}
	}

	intavg.timestamp_sample_last = timestamp_sample;

	return updated;
}

void VehicleIMU::Run()
{
	// backup schedule
	ScheduleDelayed(10_ms);

	ParametersUpdate();
	_accel_corrections.SensorCorrectionsUpdate();
	_gyro_corrections.SensorCorrectionsUpdate();

	bool update_integrator_config = false;

	// integrate queued gyro
	sensor_gyro_s gyro;

	while (_sensor_gyro_sub.update(&gyro)) {
		perf_count_interval(_gyro_update_perf, gyro.timestamp_sample);

		if (_sensor_gyro_sub.get_last_generation() != _gyro_last_generation + 1) {
			perf_count(_gyro_generation_gap_perf);
		}

		_gyro_last_generation = _sensor_gyro_sub.get_last_generation();

		_gyro_corrections.set_device_id(gyro.device_id);
		_gyro_error_count = gyro.error_count;

		const Vector3f gyro_corrected{_gyro_corrections.Correct(Vector3f{gyro.x, gyro.y, gyro.z})};
		_gyro_integrator.put(gyro.timestamp_sample, gyro_corrected);
		_last_timestamp_sample_gyro = gyro.timestamp_sample;

		// collect sample interval average for filters
		if (UpdateIntervalAverage(_gyro_interval, gyro.timestamp_sample)) {
			update_integrator_config = true;
		}

		if (_intervals_configured && _gyro_integrator.integral_ready()) {
			break;
		}
	}

	// update accel, stopping once caught up to the last gyro sample
	sensor_accel_s accel;

	while (_sensor_accel_sub.update(&accel)) {
		perf_count_interval(_accel_update_perf, accel.timestamp_sample);

		if (_sensor_accel_sub.get_last_generation() != _accel_last_generation + 1) {
			perf_count(_accel_generation_gap_perf);
		}

		_accel_last_generation = _sensor_accel_sub.get_last_generation();

		_accel_corrections.set_device_id(accel.device_id);
		_accel_error_count = accel.error_count;

		const Vector3f accel_corrected{_accel_corrections.Correct(Vector3f{accel.x, accel.y, accel.z})};
		_accel_integrator.put(accel.timestamp_sample, accel_corrected);
		_last_timestamp_sample_accel = accel.timestamp_sample;

		// collect sample interval average for filters
		if (UpdateIntervalAverage(_accel_interval, accel.timestamp_sample)) {
			update_integrator_config = true;
		}

		if (accel.clip_counter[0] > 0 || accel.clip_counter[1] > 0 || accel.clip_counter[2] > 0) {

			// rotate sensor clip counts into vehicle body frame
			const Vector3f clipping{_accel_corrections.getBoardRotation() *
				Vector3f{(float)accel.clip_counter[0], (float)accel.clip_counter[1], (float)accel.clip_counter[2]}};

			// round to get reasonble clip counts per axis (after board rotation)
			const uint8_t clip_x = roundf(fabsf(clipping(0)));
			const uint8_t clip_y = roundf(fabsf(clipping(1)));
			const uint8_t clip_z = roundf(fabsf(clipping(2)));

			_delta_velocity_clipping_total[0] += clip_x;
			_delta_velocity_clipping_total[1] += clip_y;
			_delta_velocity_clipping_total[2] += clip_z;

			if (clip_x > 0) {
				_delta_velocity_clipping |= vehicle_imu_s::CLIPPING_X;
			}

			if (clip_y > 0) {
				_delta_velocity_clipping |= vehicle_imu_s::CLIPPING_Y;
			}

			if (clip_z > 0) {
				_delta_velocity_clipping |= vehicle_imu_s::CLIPPING_Z;
			}
		}

		// break once caught up to gyro
		if (_intervals_configured
		    && (_last_timestamp_sample_accel >= (_last_timestamp_sample_gyro - 0.5f * _accel_interval.update_interval))) {

			break;
		}
	}

	// reconfigure integrators if calculated sensor intervals have changed
	if (update_integrator_config) {
		UpdateIntegratorConfiguration();
	}

	// publish if both accel & gyro integrators are ready
	if (_accel_integrator.integral_ready() && _gyro_integrator.integral_ready()) {

		uint32_t accel_integral_dt;
		uint32_t gyro_integral_dt;
		Vector3f delta_angle;
		Vector3f delta_velocity;

		if (_accel_integrator.reset(delta_velocity, accel_integral_dt)
		    && _gyro_integrator.reset(delta_angle, gyro_integral_dt)) {

			UpdateAccelVibrationMetrics(delta_velocity);
			UpdateGyroVibrationMetrics(delta_angle);

			// vehicle_imu_status
			//  publish first so that error counts are available synchronously if needed
			vehicle_imu_status_s status;
			status.accel_device_id = _accel_corrections.get_device_id();
			status.gyro_device_id = _gyro_corrections.get_device_id();
			status.accel_error_count = _accel_error_count;
			status.gyro_error_count = _gyro_error_count;
			status.accel_rate_hz = roundf(1e6f / _accel_interval.update_interval);
			status.gyro_rate_hz = round(1e6f / _gyro_interval.update_interval);
			status.accel_vibration_metric = _accel_vibration_metric;
			status.gyro_vibration_metric = _gyro_vibration_metric;
			status.gyro_coning_vibration = _gyro_coning_vibration;
			status.accel_clipping[0] = _delta_velocity_clipping_total[0];
			status.accel_clipping[1] = _delta_velocity_clipping_total[1];
			status.accel_clipping[2] = _delta_velocity_clipping_total[2];
			status.timestamp = hrt_absolute_time();
			_vehicle_imu_status_pub.publish(status);


			// publish vehicle_imu
			vehicle_imu_s imu;
			imu.timestamp_sample = _last_timestamp_sample_gyro;
			imu.accel_device_id = _accel_corrections.get_device_id();
			imu.gyro_device_id = _gyro_corrections.get_device_id();
			delta_angle.copyTo(imu.delta_angle);
			delta_velocity.copyTo(imu.delta_velocity);
			imu.delta_angle_dt = gyro_integral_dt;
			imu.delta_velocity_dt = accel_integral_dt;
			imu.delta_velocity_clipping = _delta_velocity_clipping;
			imu.timestamp = hrt_absolute_time();
			_vehicle_imu_pub.publish(imu);

			// reset clip counts
			_delta_velocity_clipping = 0;

			return;
		}
	}
}

void VehicleIMU::UpdateIntegratorConfiguration()
{
	if ((_accel_interval.update_interval > 0) && (_gyro_interval.update_interval > 0)) {

		const float configured_interval_us = 1e6f / _param_imu_integ_rate.get();

		// determine number of sensor samples that will get closest to the desired integration interval
		const uint8_t accel_integral_samples = constrain(roundf(configured_interval_us / _accel_interval.update_interval),
						       1.f, (float)sensor_accel_s::ORB_QUEUE_LENGTH);

		const uint8_t gyro_integral_samples = constrain(roundf(configured_interval_us / _gyro_interval.update_interval),
						      1.f, (float)sensor_gyro_s::ORB_QUEUE_LENGTH);

		// let the gyro set the configuration and scheduling
		// accel integrator will be forced to reset when gyro integrator is ready
		_gyro_integrator.set_reset_samples(gyro_integral_samples);
		_accel_integrator.set_reset_samples(1);

		// relaxed minimum integration time required
		_accel_integrator.set_reset_interval(roundf((accel_integral_samples - 0.5f) * _accel_interval.update_interval));
		_gyro_integrator.set_reset_interval(roundf((gyro_integral_samples - 0.5f) * _gyro_interval.update_interval));

		_sensor_accel_sub.set_required_updates(accel_integral_samples);
		_sensor_gyro_sub.set_required_updates(gyro_integral_samples);

		// run when there are enough new gyro samples, unregister accel
		_sensor_accel_sub.unregisterCallback();

		_intervals_configured = true;

		PX4_DEBUG("accel (%d), gyro (%d), accel samples: %d, gyro samples: %d, accel interval: %.1f, gyro interval: %.1f",
			  _accel_corrections.get_device_id(), _gyro_corrections.get_device_id(), accel_integral_samples, gyro_integral_samples,
			  (double)_accel_interval.update_interval, (double)_gyro_interval.update_interval);
	}
}

void VehicleIMU::UpdateAccelVibrationMetrics(const Vector3f &delta_velocity)
{
	// Accel high frequency vibe = filtered length of (delta_velocity - prev_delta_velocity)
	const Vector3f delta_velocity_diff = delta_velocity - _delta_velocity_prev;
	_accel_vibration_metric = 0.99f * _accel_vibration_metric + 0.01f * delta_velocity_diff.norm();

	_delta_velocity_prev = delta_velocity;
}

void VehicleIMU::UpdateGyroVibrationMetrics(const Vector3f &delta_angle)
{
	// Gyro high frequency vibe = filtered length of (delta_angle - prev_delta_angle)
	const Vector3f delta_angle_diff = delta_angle - _delta_angle_prev;
	_gyro_vibration_metric = 0.99f * _gyro_vibration_metric + 0.01f * delta_angle_diff.norm();

	// Gyro delta angle coning metric = filtered length of (delta_angle x prev_delta_angle)
	const Vector3f coning_metric = delta_angle % _delta_angle_prev;
	_gyro_coning_vibration = 0.99f * _gyro_coning_vibration + 0.01f * coning_metric.norm();

	_delta_angle_prev = delta_angle;
}

void VehicleIMU::PrintStatus()
{
	PX4_INFO("Accel ID: %d, interval: %.1f us, Gyro ID: %d, interval: %.1f us",
		 _accel_corrections.get_device_id(), (double)_accel_interval.update_interval,
		 _gyro_corrections.get_device_id(), (double)_gyro_interval.update_interval);

	perf_print_counter(_accel_generation_gap_perf);
	perf_print_counter(_gyro_generation_gap_perf);
	perf_print_counter(_accel_update_perf);
	perf_print_counter(_gyro_update_perf);

	_accel_corrections.PrintStatus();
	_gyro_corrections.PrintStatus();
}

} // namespace sensors
