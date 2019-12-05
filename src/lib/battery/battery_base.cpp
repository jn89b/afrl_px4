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
 * @file battery_base.cpp
 *
 * Library calls for battery functionality.
 *
 * @author Julian Oes <julian@oes.ch>
 * @author Timothy Scott <timothy@auterion.com>
 */

#include "battery_base.h"
#include <mathlib/mathlib.h>
#include <cstring>
#include <px4_platform_common/defines.h>

BatteryBase::BatteryBase() :
	ModuleParams(nullptr),
	_warning(battery_status_s::BATTERY_WARNING_NONE),
	_last_timestamp(0)
{
}

void
BatteryBase::reset()
{
	memset(&_battery_status, 0, sizeof(_battery_status));
	_battery_status.current_a = -1.f;
	_battery_status.remaining = 1.f;
	_battery_status.scale = 1.f;
	_battery_status.cell_count = _get_bat_n_cells();
	// TODO: check if it is sane to reset warning to NONE
	_battery_status.warning = battery_status_s::BATTERY_WARNING_NONE;
	_battery_status.connected = false;
}

void
BatteryBase::updateBatteryStatusRawADC(int32_t voltage_raw, int32_t current_raw, hrt_abstime timestamp,
				       bool selected_source, int priority,
				       float throttle_normalized,
				       bool armed)
{

	float voltage_v = (voltage_raw * _get_cnt_v_volt()) * _get_v_div();
	float current_a = ((current_raw * _get_cnt_v_curr()) - _get_v_offs_cur()) * _get_a_per_v();

	updateBatteryStatus(voltage_v, current_a, timestamp, selected_source, priority, throttle_normalized, armed);
}

void
BatteryBase::updateBatteryStatus(float voltage_v, float current_a, hrt_abstime timestamp,
				 bool selected_source, int priority,
				 float throttle_normalized,
				 bool armed)
{
	updateParams();

	bool connected = voltage_v > BOARD_ADC_OPEN_CIRCUIT_V &&
			 (BOARD_ADC_OPEN_CIRCUIT_V <= BOARD_VALID_UV || is_valid());

	reset();
	_battery_status.timestamp = timestamp;
	filterVoltage(voltage_v);
	filterThrottle(throttle_normalized);
	filterCurrent(current_a);
	sumDischarged(timestamp, current_a);
	estimateRemaining(_voltage_filtered_v, _current_filtered_a, _throttle_filtered, armed);
	computeScale();

	if (_battery_initialized) {
		determineWarning(connected);
	}

	if (_voltage_filtered_v > 2.1f) {
		_battery_initialized = true;
		_battery_status.voltage_v = voltage_v;
		_battery_status.voltage_filtered_v = _voltage_filtered_v;
		_battery_status.scale = _scale;
		_battery_status.current_a = current_a;
		_battery_status.current_filtered_a = _current_filtered_a;
		_battery_status.discharged_mah = _discharged_mah;
		_battery_status.warning = _warning;
		_battery_status.remaining = _remaining;
		_battery_status.connected = connected;
		_battery_status.system_source = selected_source;
		_battery_status.priority = priority;
	}

	_battery_status.timestamp = timestamp;

	if (_get_source() == 0) {
		orb_publish_auto(ORB_ID(battery_status), &_orbAdvert, &_battery_status, &_orbInstance, ORB_PRIO_DEFAULT);
	}

	battery_status->temperature = NAN;
}

void
BatteryBase::filterVoltage(float voltage_v)
{
	if (!_battery_initialized) {
		_voltage_filtered_v = voltage_v;
	}

	// TODO: inspect that filter performance
	const float filtered_next = _voltage_filtered_v * 0.99f + voltage_v * 0.01f;

	if (PX4_ISFINITE(filtered_next)) {
		_voltage_filtered_v = filtered_next;
	}
}

void
BatteryBase::filterCurrent(float current_a)
{
	if (!_battery_initialized) {
		_current_filtered_a = current_a;
	}

	// ADC poll is at 100Hz, this will perform a low pass over approx 500ms
	const float filtered_next = _current_filtered_a * 0.98f + current_a * 0.02f;

	if (PX4_ISFINITE(filtered_next)) {
		_current_filtered_a = filtered_next;
	}
}

void BatteryBase::filterThrottle(float throttle)
{
	if (!_battery_initialized) {
		_throttle_filtered = throttle;
	}

	const float filtered_next = _throttle_filtered * 0.99f + throttle * 0.01f;

	if (PX4_ISFINITE(filtered_next)) {
		_throttle_filtered = filtered_next;
	}
}

void
BatteryBase::sumDischarged(hrt_abstime timestamp, float current_a)
{
	// Not a valid measurement
	if (current_a < 0.f) {
		// Because the measurement was invalid we need to stop integration
		// and re-initialize with the next valid measurement
		_last_timestamp = 0;
		return;
	}

	// Ignore first update because we don't know dt.
	if (_last_timestamp != 0) {
		const float dt = (timestamp - _last_timestamp) / 1e6;
		// mAh since last loop: (current[A] * 1000 = [mA]) * (dt[s] / 3600 = [h])
		_discharged_mah_loop = (current_a * 1e3f) * (dt / 3600.f);
		_discharged_mah += _discharged_mah_loop;
	}

	_last_timestamp = timestamp;
}

void
BatteryBase::estimateRemaining(float voltage_v, float current_a, float throttle, bool armed)
{
	// remaining battery capacity based on voltage
	float cell_voltage = voltage_v / _get_bat_n_cells();

	// correct battery voltage locally for load drop to avoid estimation fluctuations
	if (_get_bat_r_internal() >= 0.f) {
		cell_voltage += _get_bat_r_internal() * current_a;

	} else {
		// assume linear relation between throttle and voltage drop
		cell_voltage += throttle * _get_bat_v_load_drop();
	}

	_remaining_voltage = math::gradual(cell_voltage, _get_bat_v_empty(), _get_bat_v_charged(), 0.f, 1.f);

	// choose which quantity we're using for final reporting
	if (_get_bat_capacity() > 0.f) {
		// if battery capacity is known, fuse voltage measurement with used capacity
		if (!_battery_initialized) {
			// initialization of the estimation state
			_remaining = _remaining_voltage;

		} else {
			// The lower the voltage the more adjust the estimate with it to avoid deep discharge
			const float weight_v = 3e-4f * (1 - _remaining_voltage);
			_remaining = (1 - weight_v) * _remaining + weight_v * _remaining_voltage;
			// directly apply current capacity slope calculated using current
			_remaining -= _discharged_mah_loop / _get_bat_capacity();
			_remaining = math::max(_remaining, 0.f);
		}

	} else {
		// else use voltage
		_remaining = _remaining_voltage;
	}
}

void
BatteryBase::determineWarning(bool connected)
{
	if (connected) {
		// propagate warning state only if the state is higher, otherwise remain in current warning state
		if (_remaining < _get_bat_emergen_thr()) {
			_warning = battery_status_s::BATTERY_WARNING_EMERGENCY;

		} else if (_remaining < _get_bat_crit_thr()) {
			_warning = battery_status_s::BATTERY_WARNING_CRITICAL;

		} else if (_remaining < _get_bat_low_thr()) {
			_warning = battery_status_s::BATTERY_WARNING_LOW;

		} else {
			_warning = battery_status_s::BATTERY_WARNING_NONE;
		}
	}
}

void
BatteryBase::computeScale()
{
	const float voltage_range = (_get_bat_v_charged() - _get_bat_v_empty());

	// reusing capacity calculation to get single cell voltage before drop
	const float bat_v = _get_bat_v_empty() + (voltage_range * _remaining_voltage);

	_scale = _get_bat_v_charged() / bat_v;

	if (_scale > 1.3f) { // Allow at most 30% compensation
		_scale = 1.3f;

	} else if (!PX4_ISFINITE(_scale) || _scale < 1.f) { // Shouldn't ever be more than the power at full battery
		_scale = 1.f;
	}
}

float
BatteryBase::_get_cnt_v_volt()
{
	float val = _get_cnt_v_volt_raw();

	if (val < 0.0f) {
		return 3.3f / 4096.0f;

	} else {
		return val;
	}
}

float
BatteryBase::_get_cnt_v_curr()
{
	float val = _get_cnt_v_curr_raw();

	if (val < 0.0f) {
		return 3.3f / 4096.0f;

	} else {
		return val;
	}
}

float
BatteryBase::_get_v_div()
{
	float val = _get_v_div_raw();

	if (val <= 0.0f) {
		return BOARD_BATTERY1_V_DIV;

	} else {
		return val;
	}
}

float
BatteryBase::_get_a_per_v()
{
	float val = _get_a_per_v_raw();

	if (val <= 0.0f) {
		return BOARD_BATTERY1_A_PER_V;

	} else {
		return val;
	}
}
