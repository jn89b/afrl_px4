/****************************************************************************
 *
 *   Copyright (c) 2013-2020 PX4 Development Team. All rights reserved.
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
 * @file accelerometer_calibration.cpp
 *
 * Implementation of accelerometer calibration.
 *
 * Transform acceleration vector to true orientation, scale and offset
 *
 * ===== Model =====
 * accel_corr = accel_T * (accel_raw - accel_offs)
 *
 * accel_corr[3] - fully corrected acceleration vector in body frame
 * accel_T[3][3] - accelerometers transform matrix, rotation and scaling transform
 * accel_raw[3]  - raw acceleration vector
 * accel_offs[3] - acceleration offset vector
 *
 * ===== Calibration =====
 *
 * Reference vectors
 * accel_corr_ref[6][3] = [  g  0  0 ]     // nose up
 *                        | -g  0  0 |     // nose down
 *                        |  0  g  0 |     // left side down
 *                        |  0 -g  0 |     // right side down
 *                        |  0  0  g |     // on back
 *                        [  0  0 -g ]     // level
 * accel_raw_ref[6][3]
 *
 * accel_corr_ref[i] = accel_T * (accel_raw_ref[i] - accel_offs), i = 0...5
 *
 * 6 reference vectors * 3 axes = 18 equations
 * 9 (accel_T) + 3 (accel_offs) = 12 unknown constants
 *
 * Find accel_offs
 *
 * accel_offs[i] = (accel_raw_ref[i*2][i] + accel_raw_ref[i*2+1][i]) / 2
 *
 * Find accel_T
 *
 * 9 unknown constants
 * need 9 equations -> use 3 of 6 measurements -> 3 * 3 = 9 equations
 *
 * accel_corr_ref[i*2] = accel_T * (accel_raw_ref[i*2] - accel_offs), i = 0...2
 *
 * Solve separate system for each row of accel_T:
 *
 * accel_corr_ref[j*2][i] = accel_T[i] * (accel_raw_ref[j*2] - accel_offs), j = 0...2
 *
 * A * x = b
 *
 * x = [ accel_T[0][i] ]
 *     | accel_T[1][i] |
 *     [ accel_T[2][i] ]
 *
 * b = [ accel_corr_ref[0][i] ]	// One measurement per side is enough
 *     | accel_corr_ref[2][i] |
 *     [ accel_corr_ref[4][i] ]
 *
 * a[i][j] = accel_raw_ref[i][j] - accel_offs[j], i = 0;2;4, j = 0...2
 *
 * Matrix A is common for all three systems:
 * A = [ a[0][0]  a[0][1]  a[0][2] ]
 *     | a[2][0]  a[2][1]  a[2][2] |
 *     [ a[4][0]  a[4][1]  a[4][2] ]
 *
 * x = A^-1 * b
 *
 * accel_T = A^-1 * g
 * g = 9.80665
 *
 * ===== Rotation =====
 *
 * Calibrating using model:
 * accel_corr = accel_T_r * (rot * accel_raw - accel_offs_r)
 *
 * Actual correction:
 * accel_corr = rot * accel_T * (accel_raw - accel_offs)
 *
 * Known: accel_T_r, accel_offs_r, rot
 * Unknown: accel_T, accel_offs
 *
 * Solution:
 * accel_T_r * (rot * accel_raw - accel_offs_r) = rot * accel_T * (accel_raw - accel_offs)
 * rot^-1 * accel_T_r * (rot * accel_raw - accel_offs_r) = accel_T * (accel_raw - accel_offs)
 * rot^-1 * accel_T_r * rot * accel_raw - rot^-1 * accel_T_r * accel_offs_r = accel_T * accel_raw - accel_T * accel_offs)
 * => accel_T = rot^-1 * accel_T_r * rot
 * => accel_offs = rot^-1 * accel_offs_r
 *
 * @author Anton Babushkin <anton.babushkin@me.com>
 */

#include "accelerometer_calibration.h"
#include "calibration_messages.h"
#include "calibration_routines.h"
#include "commander_helper.h"

#include <px4_platform_common/defines.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/time.h>

#include <drivers/drv_hrt.h>
#include <lib/mathlib/mathlib.h>
#include <lib/ecl/geo/geo.h>
#include <matrix/math.hpp>
#include <lib/conversion/rotation.h>
#include <lib/parameters/param.h>
#include <systemlib/err.h>
#include <systemlib/mavlink_log.h>
#include <uORB/topics/sensor_accel.h>
#include <uORB/topics/sensor_correction.h>
#include <uORB/topics/vehicle_attitude.h>
#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionBlocking.hpp>

using namespace time_literals;
using namespace matrix;
using math::radians;

static constexpr char sensor_name[] {"accel"};

static constexpr unsigned MAX_ACCEL_SENS = 3;

static calibrate_return do_accel_calibration_measurements(orb_advert_t *mavlink_log_pub,
		Vector3f(&accel_offs)[MAX_ACCEL_SENS],
		Matrix3f(&accel_T)[MAX_ACCEL_SENS], unsigned active_sensors);

static calibrate_return read_accelerometer_avg(float (&accel_avg)[MAX_ACCEL_SENS][detect_orientation_side_count][3],
		unsigned orient, unsigned samples_num);

static calibrate_return calculate_calibration_values(unsigned sensor,
		float (&accel_ref)[MAX_ACCEL_SENS][detect_orientation_side_count][3], Matrix3f(&accel_T)[MAX_ACCEL_SENS],
		Vector3f(&accel_offs)[MAX_ACCEL_SENS]);

/// Data passed to calibration worker routine
typedef struct  {
	orb_advert_t	*mavlink_log_pub{nullptr};
	unsigned	done_count{0};
	float		accel_ref[MAX_ACCEL_SENS][detect_orientation_side_count][3] {};
} accel_worker_data_t;

int do_accel_calibration(orb_advert_t *mavlink_log_pub)
{
	calibration_log_info(mavlink_log_pub, CAL_QGC_STARTED_MSG, sensor_name);

	int res = PX4_OK;

	int32_t device_id[MAX_ACCEL_SENS] {};
	int device_prio_max = 0;
	int32_t device_id_primary = 0;
	unsigned active_sensors = 0;

	// We should not try to subscribe if the topic doesn't actually exist and can be counted.
	const unsigned orb_accel_count = orb_group_count(ORB_ID(sensor_accel));

	// Warn that we will not calibrate more than max_accels accelerometers
	if (orb_accel_count > MAX_ACCEL_SENS) {
		calibration_log_critical(mavlink_log_pub, "Detected %u accels, but will calibrate only %u", orb_accel_count,
					 MAX_ACCEL_SENS);
	}

	for (uint8_t cur_accel = 0; cur_accel < orb_accel_count && cur_accel < MAX_ACCEL_SENS; cur_accel++) {
		uORB::SubscriptionData<sensor_accel_s> accel_sub{ORB_ID(sensor_accel), cur_accel};
		device_id[cur_accel] = accel_sub.get().device_id;

		if (device_id[cur_accel] != 0) {
			// Get priority
			int32_t prio = accel_sub.get_priority();

			if (prio > device_prio_max) {
				device_prio_max = prio;
				device_id_primary = device_id[cur_accel];
			}

			active_sensors++;

		} else {
			calibration_log_critical(mavlink_log_pub, "Accel #%u no device id, abort", cur_accel);
			return PX4_ERROR;
		}
	}

	/* measure and calculate offsets & scales */
	Vector3f accel_offs[MAX_ACCEL_SENS] {};
	Matrix3f accel_T[MAX_ACCEL_SENS] {};
	calibrate_return cal_return = do_accel_calibration_measurements(mavlink_log_pub, accel_offs, accel_T, active_sensors);

	if (cal_return != calibrate_return_ok) {
		// Cancel message already displayed, nothing left to do
		return PX4_ERROR;
	}

	if (active_sensors == 0) {
		calibration_log_critical(mavlink_log_pub, CAL_ERROR_SENSOR_MSG);
		return PX4_ERROR;
	}

	/* measurements completed successfully, rotate calibration values */
	int32_t board_rotation_int = 0;
	param_get(param_find("SENS_BOARD_ROT"), &board_rotation_int);
	const Dcmf board_rotation = get_rot_matrix((enum Rotation)board_rotation_int);
	const Dcmf board_rotation_t = board_rotation.transpose();

	param_set_no_notification(param_find("CAL_ACC_PRIME"), &device_id_primary);

	for (unsigned uorb_index = 0; uorb_index < MAX_ACCEL_SENS; uorb_index++) {

		if (uorb_index < active_sensors) {
			/* handle individual sensors, one by one */
			const Vector3f accel_offs_rotated = board_rotation_t *accel_offs[uorb_index];
			const Matrix3f accel_T_rotated = board_rotation_t *accel_T[uorb_index] * board_rotation;

			PX4_INFO("found offset %d: x: %.6f, y: %.6f, z: %.6f", uorb_index,
				 (double)accel_offs_rotated(0), (double)accel_offs_rotated(1), (double)accel_offs_rotated(2));

			PX4_INFO("found scale %d: x: %.6f, y: %.6f, z: %.6f", uorb_index,
				 (double)accel_T_rotated(0, 0), (double)accel_T_rotated(1, 1), (double)accel_T_rotated(2, 2));

			char str[30] {};

			// calibration offsets
			float x_offset = accel_offs_rotated(0);
			sprintf(str, "CAL_ACC%u_XOFF", uorb_index);
			param_set_no_notification(param_find(str), &x_offset);

			float y_offset = accel_offs_rotated(1);
			sprintf(str, "CAL_ACC%u_YOFF", uorb_index);
			param_set_no_notification(param_find(str), &y_offset);

			float z_offset = accel_offs_rotated(2);
			sprintf(str, "CAL_ACC%u_ZOFF", uorb_index);
			param_set_no_notification(param_find(str), &z_offset);


			// calibration scale
			float x_scale = accel_T_rotated(0, 0);
			sprintf(str, "CAL_ACC%u_XSCALE", uorb_index);
			param_set_no_notification(param_find(str), &x_scale);

			float y_scale = accel_T_rotated(1, 1);
			sprintf(str, "CAL_ACC%u_YSCALE", uorb_index);
			param_set_no_notification(param_find(str), &y_scale);

			float z_scale = accel_T_rotated(2, 2);
			sprintf(str, "CAL_ACC%u_ZSCALE", uorb_index);
			param_set_no_notification(param_find(str), &z_scale);

			// calibration device ID
			sprintf(str, "CAL_ACC%u_ID", uorb_index);
			param_set_no_notification(param_find(str), &device_id[uorb_index]);

		} else {
			char str[30] {};

			// reset calibration offsets
			sprintf(str, "CAL_ACC%u_XOFF", uorb_index);
			param_reset(param_find(str));
			sprintf(str, "CAL_ACC%u_YOFF", uorb_index);
			param_reset(param_find(str));
			sprintf(str, "CAL_ACC%u_ZOFF", uorb_index);
			param_reset(param_find(str));

			// reset calibration scale
			sprintf(str, "CAL_ACC%u_XSCALE", uorb_index);
			param_reset(param_find(str));
			sprintf(str, "CAL_ACC%u_YSCALE", uorb_index);
			param_reset(param_find(str));
			sprintf(str, "CAL_ACC%u_ZSCALE", uorb_index);
			param_reset(param_find(str));

			// reset calibration device ID
			sprintf(str, "CAL_ACC%u_ID", uorb_index);
			param_reset(param_find(str));
		}
	}

	param_notify_changes();

	if (res == PX4_OK) {
		/* if there is a any preflight-check system response, let the barrage of messages through */
		px4_usleep(200000);

		calibration_log_info(mavlink_log_pub, CAL_QGC_DONE_MSG, sensor_name);

	} else {
		calibration_log_critical(mavlink_log_pub, CAL_QGC_FAILED_MSG, sensor_name);
	}

	/* give this message enough time to propagate */
	px4_usleep(600000);

	return res;
}

static calibrate_return accel_calibration_worker(detect_orientation_return orientation, int cancel_sub, void *data)
{
	const unsigned samples_num = 750;
	accel_worker_data_t *worker_data = (accel_worker_data_t *)(data);

	calibration_log_info(worker_data->mavlink_log_pub, "[cal] Hold still, measuring %s side",
			     detect_orientation_str(orientation));

	read_accelerometer_avg(worker_data->accel_ref, orientation, samples_num);

	calibration_log_info(worker_data->mavlink_log_pub, "[cal] %s side result: [%8.4f %8.4f %8.4f]",
			     detect_orientation_str(orientation),
			     (double)worker_data->accel_ref[0][orientation][0],
			     (double)worker_data->accel_ref[0][orientation][1],
			     (double)worker_data->accel_ref[0][orientation][2]);

	worker_data->done_count++;
	calibration_log_info(worker_data->mavlink_log_pub, CAL_QGC_PROGRESS_MSG, 17 * worker_data->done_count);

	return calibrate_return_ok;
}

static calibrate_return do_accel_calibration_measurements(orb_advert_t *mavlink_log_pub,
		Vector3f(&accel_offs)[MAX_ACCEL_SENS], Matrix3f(&accel_T)[MAX_ACCEL_SENS], unsigned active_sensors)
{
	calibrate_return result = calibrate_return_ok;

	accel_worker_data_t worker_data{};
	worker_data.mavlink_log_pub = mavlink_log_pub;

	bool data_collected[detect_orientation_side_count] {};

	if (result == calibrate_return_ok) {
		int cancel_sub = calibrate_cancel_subscribe();
		result = calibrate_from_orientation(mavlink_log_pub, cancel_sub, data_collected, accel_calibration_worker, &worker_data,
						    false);

		calibrate_cancel_unsubscribe(cancel_sub);
	}

	if (result == calibrate_return_ok) {
		/* calculate offsets and transform matrix */
		for (unsigned i = 0; i < active_sensors; i++) {
			result = calculate_calibration_values(i, worker_data.accel_ref, accel_T, accel_offs);

			if (result != calibrate_return_ok) {
				calibration_log_critical(mavlink_log_pub, "ERROR: calibration calculation error");
				break;
			}
		}
	}

	return result;
}

/*
 * Read specified number of accelerometer samples, calculate average and dispersion.
 */
static calibrate_return read_accelerometer_avg(float (&accel_avg)[MAX_ACCEL_SENS][detect_orientation_side_count][3],
		unsigned orient, unsigned samples_num)
{
	/* get total sensor board rotation matrix */
	float board_offset[3] {};
	param_get(param_find("SENS_BOARD_X_OFF"), &board_offset[0]);
	param_get(param_find("SENS_BOARD_Y_OFF"), &board_offset[1]);
	param_get(param_find("SENS_BOARD_Z_OFF"), &board_offset[2]);

	const Dcmf board_rotation_offset{Eulerf{math::radians(board_offset[0]), math::radians(board_offset[1]), math::radians(board_offset[2])}};

	int32_t board_rotation_int = 0;
	param_get(param_find("SENS_BOARD_ROT"), &board_rotation_int);

	const Dcmf board_rotation = board_rotation_offset * get_rot_matrix((enum Rotation)board_rotation_int);

	Vector3f accel_sum[MAX_ACCEL_SENS] {};
	unsigned counts[MAX_ACCEL_SENS] {};

	unsigned errcount = 0;

	// sensor thermal corrections
	uORB::Subscription sensor_correction_sub{ORB_ID(sensor_correction)};
	sensor_correction_s sensor_correction{};
	sensor_correction_sub.copy(&sensor_correction);

	uORB::SubscriptionBlocking<sensor_accel_s> accel_sub[MAX_ACCEL_SENS] {
		{ORB_ID(sensor_accel), 0, 0},
		{ORB_ID(sensor_accel), 0, 1},
		{ORB_ID(sensor_accel), 0, 2},
	};

	/* use the first sensor to pace the readout, but do per-sensor counts */
	while (counts[0] < samples_num) {
		if (accel_sub[0].updatedBlocking(100000)) {
			for (unsigned accel_index = 0; accel_index < MAX_ACCEL_SENS; accel_index++) {
				sensor_accel_s arp;

				if (accel_sub[accel_index].update(&arp)) {
					// fetch optional thermal offset corrections in sensor/board frame
					Vector3f offset{0, 0, 0};
					sensor_correction_sub.update(&sensor_correction);

					if (sensor_correction.timestamp > 0 && arp.device_id != 0) {
						for (uint8_t correction_index = 0; correction_index < MAX_ACCEL_SENS; correction_index++) {
							if (sensor_correction.accel_device_ids[correction_index] == arp.device_id) {
								switch (correction_index) {
								case 0:
									offset = Vector3f{sensor_correction.accel_offset_0};
									break;
								case 1:
									offset = Vector3f{sensor_correction.accel_offset_1};
									break;
								case 2:
									offset = Vector3f{sensor_correction.accel_offset_2};
									break;
								}
							}
						}
					}

					accel_sum[accel_index] += Vector3f{arp.x, arp.y, arp.z} - offset;
					counts[accel_index]++;
				}
			}

		} else {
			errcount++;
			continue;
		}

		if (errcount > samples_num / 10) {
			return calibrate_return_error;
		}
	}

	// rotate sensor measurements from sensor to body frame using board rotation matrix
	for (unsigned s = 0; s < MAX_ACCEL_SENS; s++) {
		accel_sum[s] = board_rotation * accel_sum[s];
	}

	for (unsigned s = 0; s < MAX_ACCEL_SENS; s++) {
		const auto sum = accel_sum[s] / counts[s];
		sum.copyTo(accel_avg[s][orient]);
	}

	return calibrate_return_ok;
}

static calibrate_return calculate_calibration_values(unsigned sensor,
		float (&accel_ref)[MAX_ACCEL_SENS][detect_orientation_side_count][3], Matrix3f(&accel_T)[MAX_ACCEL_SENS],
		Vector3f(&accel_offs)[MAX_ACCEL_SENS])
{
	/* calculate offsets */
	for (unsigned i = 0; i < 3; i++) {
		accel_offs[sensor](i) = (accel_ref[sensor][i * 2][i] + accel_ref[sensor][i * 2 + 1][i]) / 2;
	}

	/* fill matrix A for linear equations system*/
	Matrix3f mat_A;

	for (unsigned i = 0; i < 3; i++) {
		for (unsigned j = 0; j < 3; j++) {
			mat_A(i, j) = accel_ref[sensor][i * 2][j] - accel_offs[sensor](j);
		}
	}

	/* calculate inverse matrix for A */
	const Matrix3f mat_A_inv = mat_A.I();

	/* copy results to accel_T */
	for (unsigned i = 0; i < 3; i++) {
		for (unsigned j = 0; j < 3; j++) {
			/* simplify matrices mult because b has only one non-zero element == g at index i */
			accel_T[sensor](j, i) = mat_A_inv(j, i) * CONSTANTS_ONE_G;
		}
	}

	return calibrate_return_ok;
}

int do_level_calibration(orb_advert_t *mavlink_log_pub)
{
	bool success = false;

	calibration_log_info(mavlink_log_pub, CAL_QGC_STARTED_MSG, "level");

	param_t roll_offset_handle = param_find("SENS_BOARD_X_OFF");
	param_t pitch_offset_handle = param_find("SENS_BOARD_Y_OFF");

	// get old values
	float roll_offset_current = 0.f;
	float pitch_offset_current = 0.f;
	param_get(roll_offset_handle, &roll_offset_current);
	param_get(pitch_offset_handle, &pitch_offset_current);

	int32_t board_rot_current = 0;
	param_get(param_find("SENS_BOARD_ROT"), &board_rot_current);

	const Dcmf board_rotation_offset{Eulerf{radians(roll_offset_current), radians(pitch_offset_current), 0.f}};

	float roll_mean = 0.f;
	float pitch_mean = 0.f;
	unsigned counter = 0;
	bool had_motion = true;
	int num_retries = 0;

	uORB::SubscriptionBlocking<vehicle_attitude_s> att_sub{ORB_ID(vehicle_attitude)};

	while (had_motion && num_retries++ < 50) {
		Vector2f min_angles{100.f, 100.f};
		Vector2f max_angles{-100.f, -100.f};
		roll_mean = 0.0f;
		pitch_mean = 0.0f;
		counter = 0;
		int last_progress_report = -100;
		const hrt_abstime calibration_duration = 500_ms;
		const hrt_abstime start = hrt_absolute_time();

		while (hrt_elapsed_time(&start) < calibration_duration) {

			vehicle_attitude_s att{};

			if (!att_sub.updateBlocking(att, 100000)) {
				// attitude estimator is not running
				calibration_log_critical(mavlink_log_pub, "attitude estimator not running - check system boot");
				calibration_log_critical(mavlink_log_pub, CAL_QGC_FAILED_MSG, "level");
				goto out;
			}

			int progress = 100 * hrt_elapsed_time(&start) / calibration_duration;

			if (progress >= last_progress_report + 20) {
				calibration_log_info(mavlink_log_pub, CAL_QGC_PROGRESS_MSG, progress);
				last_progress_report = progress;
			}

			Eulerf att_euler{Quatf{att.q}};

			// keep min + max angles
			for (int i = 0; i < 2; ++i) {
				if (att_euler(i) < min_angles(i)) { min_angles(i) = att_euler(i); }

				if (att_euler(i) > max_angles(i)) { max_angles(i) = att_euler(i); }
			}

			att_euler(2) = 0.f; // ignore yaw

			att_euler = Eulerf{board_rotation_offset *Dcmf{att_euler}};  // subtract existing board rotation
			roll_mean += att_euler.phi();
			pitch_mean += att_euler.theta();
			++counter;
		}

		// motion detection: check that (max-min angle) is within a threshold.
		// The difference is typically <0.1 deg while at rest
		if (max_angles(0) - min_angles(0) < math::radians(0.5f) &&
		    max_angles(1) - min_angles(1) < math::radians(0.5f)) {

			had_motion = false;
		}
	}

	calibration_log_info(mavlink_log_pub, CAL_QGC_PROGRESS_MSG, 100);

	roll_mean /= counter;
	pitch_mean /= counter;

	if (had_motion) {
		calibration_log_critical(mavlink_log_pub, "motion during calibration");

	} else if (fabsf(roll_mean) > 0.8f) {
		calibration_log_critical(mavlink_log_pub, "excess roll angle");

	} else if (fabsf(pitch_mean) > 0.8f) {
		calibration_log_critical(mavlink_log_pub, "excess pitch angle");

	} else {
		float roll_mean_degrees = math::degrees(roll_mean);
		float pitch_mean_degrees = math::degrees(pitch_mean);
		param_set_no_notification(roll_offset_handle, &roll_mean_degrees);
		param_set_no_notification(pitch_offset_handle, &pitch_mean_degrees);
		param_notify_changes();
		success = true;
	}

out:

	if (success) {
		calibration_log_info(mavlink_log_pub, CAL_QGC_DONE_MSG, "level");
		return 0;

	} else {
		calibration_log_critical(mavlink_log_pub, CAL_QGC_FAILED_MSG, "level");
		return 1;
	}
}
