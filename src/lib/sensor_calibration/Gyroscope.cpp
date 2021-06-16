/****************************************************************************
 *
 *   Copyright (c) 2020, 2021 PX4 Development Team. All rights reserved.
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

#include "Gyroscope.hpp"

#include "Utilities.hpp"

#include <lib/parameters/param.h>

using namespace matrix;
using namespace time_literals;

namespace calibration
{

Gyroscope::Gyroscope()
{
	Reset();
}

Gyroscope::Gyroscope(uint32_t device_id, bool external)
{
	Reset();
	set_device_id(device_id, external);
}

void Gyroscope::set_device_id(uint32_t device_id, bool external)
{
	if (_device_id != device_id || _external != external) {
		set_external(external);
		_device_id = device_id;

		if (_device_id != 0) {
			_calibration_index = FindCalibrationIndex(SensorString(), _device_id);
		}

		ParametersUpdate();
		SensorCorrectionsUpdate(true);
	}
}

void Gyroscope::set_external(bool external)
{
	// update priority default appropriately if not set
	if (_calibration_index < 0 || _priority < 0) {
		if ((_priority < 0) || (_priority > 100)) {
			_priority = external ? DEFAULT_EXTERNAL_PRIORITY : DEFAULT_PRIORITY;

		} else if (!_external && external && (_priority == DEFAULT_PRIORITY)) {
			// internal -> external
			_priority = DEFAULT_EXTERNAL_PRIORITY;

		} else if (_external && !external && (_priority == DEFAULT_EXTERNAL_PRIORITY)) {
			// external -> internal
			_priority = DEFAULT_PRIORITY;
		}
	}

	_external = external;
}

void Gyroscope::SensorCorrectionsUpdate(bool force)
{
	// check if the selected sensor has updated
	if (_sensor_correction_sub.updated() || force) {

		// valid device id required
		if (_device_id == 0) {
			return;
		}

		sensor_correction_s corrections;

		if (_sensor_correction_sub.copy(&corrections)) {
			// find sensor_corrections index
			for (int i = 0; i < MAX_SENSOR_COUNT; i++) {
				if (corrections.gyro_device_ids[i] == _device_id) {
					switch (i) {
					case 0:
						_thermal_offset = Vector3f{corrections.gyro_offset_0};
						return;
					case 1:
						_thermal_offset = Vector3f{corrections.gyro_offset_1};
						return;
					case 2:
						_thermal_offset = Vector3f{corrections.gyro_offset_2};
						return;
					case 3:
						_thermal_offset = Vector3f{corrections.gyro_offset_3};
						return;
					}
				}
			}
		}

		// zero thermal offset if not found
		_thermal_offset.zero();
	}
}

bool Gyroscope::set_offset(const Vector3f &offset)
{
	if (Vector3f(_offset - offset).longerThan(0.01f)) {
		if (PX4_ISFINITE(offset(0)) && PX4_ISFINITE(offset(1)) && PX4_ISFINITE(offset(2))) {
			_offset = offset;
			_calibration_count++;
			return true;
		}
	}

	return false;
}

void Gyroscope::set_rotation(Rotation rotation)
{
	_rotation_enum = rotation;

	// always apply board level adjustments
	_rotation = Dcmf(GetSensorLevelAdjustment()) * get_rot_matrix(rotation);
}

void Gyroscope::ParametersUpdate()
{
	if (_calibration_index >= 0) {

		// CAL_GYROx_ROT
		int32_t rotation_value = GetCalibrationParam(SensorString(), "ROT", _calibration_index);

		if (_external) {
			if ((rotation_value >= ROTATION_MAX) || (rotation_value < 0)) {
				PX4_ERR("External %s %" PRIu32 " (%" PRId8 ") invalid rotation %" PRId32 ", resetting to rotation none",
					SensorString(), _device_id, _calibration_index, rotation_value);
				rotation_value = ROTATION_NONE;
				SetCalibrationParam(SensorString(), "ROT", _calibration_index, rotation_value);
			}

			set_rotation(static_cast<Rotation>(rotation_value));

		} else {
			// internal, CAL_GYROx_ROT -1
			if (rotation_value != -1) {
				PX4_ERR("Internal %s %" PRIu32 " (%" PRId8 ") invalid rotation %" PRId32 " resetting",
					SensorString(), _device_id, _calibration_index, rotation_value);
				SetCalibrationParam(SensorString(), "ROT", _calibration_index, -1);
			}

			// internal sensors follow board rotation
			set_rotation(GetBoardRotation());
		}

		// CAL_GYROx_PRIO
		_priority = GetCalibrationParam(SensorString(), "PRIO", _calibration_index);

		if ((_priority < 0) || (_priority > 100)) {
			// reset to default, -1 is the uninitialized parameter value
			int32_t new_priority = _external ? DEFAULT_EXTERNAL_PRIORITY : DEFAULT_PRIORITY;

			if (_priority != -1) {
				PX4_ERR("%s %" PRIu32 " (%" PRId8 ") invalid priority %" PRId32 ", resetting to %" PRId32, SensorString(), _device_id,
					_calibration_index, _priority, new_priority);
			}

			SetCalibrationParam(SensorString(), "PRIO", _calibration_index, new_priority);
			_priority = new_priority;
		}

		// CAL_GYROx_OFF{X,Y,Z}
		set_offset(GetCalibrationParamsVector3f(SensorString(), "OFF", _calibration_index));

	} else {
		Reset();
	}
}

void Gyroscope::Reset()
{
	if (_external) {
		set_rotation(ROTATION_NONE);

	} else {
		// internal sensors follow board rotation
		set_rotation(GetBoardRotation());
	}

	_offset.zero();

	_thermal_offset.zero();

	_priority = _external ? DEFAULT_EXTERNAL_PRIORITY : DEFAULT_PRIORITY;

	_calibration_index = -1;

	_calibration_count = 0;
}

bool Gyroscope::ParametersSave()
{
	if (_calibration_index >= 0) {
		// save calibration
		bool success = true;
		success &= SetCalibrationParam(SensorString(), "ID", _calibration_index, _device_id);
		success &= SetCalibrationParam(SensorString(), "PRIO", _calibration_index, _priority);
		success &= SetCalibrationParamsVector3f(SensorString(), "OFF", _calibration_index, _offset);

		if (_external) {
			success &= SetCalibrationParam(SensorString(), "ROT", _calibration_index, (int32_t)_rotation_enum);

		} else {
			success &= SetCalibrationParam(SensorString(), "ROT", _calibration_index, -1);
		}

		return success;
	}

	return false;
}

void Gyroscope::PrintStatus()
{
	PX4_INFO("%s %" PRIu32 " EN: %d, offset: [%.4f %.4f %.4f]", SensorString(), device_id(), enabled(),
		 (double)_offset(0), (double)_offset(1), (double)_offset(2));

	if (_thermal_offset.norm() > 0.f) {
		PX4_INFO("%s %" PRIu32 " temperature offset: [%.4f %.4f %.4f]", SensorString(), _device_id,
			 (double)_thermal_offset(0), (double)_thermal_offset(1), (double)_thermal_offset(2));
	}
}

} // namespace calibration
