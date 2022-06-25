/****************************************************************************
 *
 *   Copyright (c) 2018 PX4 Development Team. All rights reserved.
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
* @file FlightTestInput.cpp
*
* @author Justin Nguyen	<jnguyenblue2804@gmail.com>
*
*/

// using namespace time_literals;

#include "FlightTestInput.hpp"


FlightTestInput::FlightTestInput() :
	ModuleParams(nullptr)
{
	// _flight_test_input = {};
	_vehicle_status = {};
	_commander_state = {};
}

FlightTestInput::~FlightTestInput()
{
}


void FlightTestInput::update(float dt)
{
	/* check vehicle status for changes to publication state */
	vehicle_status_poll();
	commander_state_poll();

	// if (_parameter_update_sub.updated()) {
	// 	parameter_update_s param_update;
	// 	_parameter_update_sub.copy(&param_update);

	// 	// If any parameter updated, call updateParams() to check if
	// 	// this class attributes need updating (and do so).
	// 	updateParams();
	// }

	switch (_state) {
	case TEST_INPUT_OFF:

		_state = TEST_INPUT_WAIT;

		break;

	case TEST_INPUT_WAIT:

		if (_param_fti_enable.get() == 1) {
			_state = TEST_INPUT_INIT;
		}

		return;

	case TEST_INPUT_INIT:
		// Initialize sweep variables and store current autopilot mode

		mavlink_log_info(&_mavlink_log_pub, "#Flight test input injection starting");

		// abort sweep if any system mode (main_state or nav_state) change
		_main_state = _commander_state.main_state;
		_nav_state = _vehicle_status.nav_state;

		_time_running = 0;
		_raw_output = 0;

		// sine sum for frequency sweep calculation
		_sweep_sine_input = 0;

		_state = TEST_INPUT_RUNNING;
		break;

	case TEST_INPUT_RUNNING:

		// only run if main state and nav state are unchanged and sweep mode is valid
		if ((_main_state == _commander_state.main_state)
		    && (_nav_state == _vehicle_status.nav_state)
		    && (_param_fti_enable.get() == 1)
		    && (_param_fti_mode.get() == 0 || _param_fti_mode.get() == 1)
		   ) {

			if (_param_fti_mode.get() == 0)
			{
				// Frequency sweep mode
				computeSweep(dt);
			} else if (_param_fti_mode.get() == 1) {
			// 	// Doublet mode
			// 	computeDoublet(dt);
			}

			// increment time
			_time_running += dt;

		} else {
			mavlink_log_info(&_mavlink_log_pub, "#Flight test input aborted");
			_state = TEST_INPUT_COMPLETE;
		}

		break;

	case TEST_INPUT_COMPLETE:

		_raw_output = 0;

		// _flight_test_input.sweep_time_segment_pct = 0;
		// _flight_test_input.sweep_frequency = 0;
		// _flight_test_input.sweep_amplitude = 0;

		// _flight_test_input.doublet_amplitude = 0;

		// _flight_test_input.injection_input = 0;
		// _flight_test_input.injection_output = 0;

		// only return to off state once param is reset to 0
		if (_param_fti_enable.get() == 0) {
			mavlink_log_info(&_mavlink_log_pub, "#Flight test input resetting");
			_state = TEST_INPUT_OFF;
		}

		break;
	}

	// _flight_test_input.timestamp = hrt_absolute_time();
	// _flight_test_input.state = _state;
	// _flight_test_input.mode = _param_fti_mode.get();
	// _flight_test_input.injection_point = _injection_point.get();
	// _flight_test_input.raw_output = _raw_output;

	// if (_flight_test_input_pub != nullptr) {
	// 	orb_publish(ORB_ID(flight_test_input), _flight_test_input_pub, &_flight_test_input);

	// } else {
	// 	_flight_test_input_pub = orb_advertise(ORB_ID(flight_test_input), &_flight_test_input);
	// }
}

void FlightTestInput::computeSweep(float dt)
{
	if (_time_running <= _param_fti_fs_duration.get()) {
		// Compute Current Sweep
		float time_segment_pct = (_time_running) / math::constrain(_param_fti_fs_duration.get(), 0.01f, 1000.0f);
		float amplitude = (_param_fti_fs_frq_end.get() - _param_fti_fs_frq_begin.get()) * time_segment_pct +
				  _param_fti_fs_frq_begin.get();
		float frequency = (_param_fti_fs_frq_end.get() - _param_fti_fs_frq_begin.get()) * powf(time_segment_pct,
				  _param_fti_fs_ramp.get()) + _param_fti_fs_frq_begin.get();

		// Compute Frequency Sweep Segment
		_sweep_sine_input += 2 * M_PI_F * frequency * dt;

		if (_sweep_sine_input > (2 * M_PI_F)) {
			_sweep_sine_input -= 2 * M_PI_F;
		}

		_raw_output = amplitude * sinf(_sweep_sine_input);

		// _flight_test_input.sweep_time_segment_pct = time_segment_pct;
		// _flight_test_input.sweep_amplitude = amplitude;
		// _flight_test_input.sweep_frequency = frequency;

	} else {
		mavlink_log_info(&_mavlink_log_pub, "#Frequency Sweep complete");
		_state = TEST_INPUT_COMPLETE;
	}
}

float FlightTestInput::inject(const uint8_t injection_point, const float inject_input)
{
	float inject_output = inject_input;

	if ((_state == TEST_INPUT_RUNNING) && (injection_point == _param_fti_injxn_point.get())) {
		// update logging
		// _flight_test_input.injection_input = inject_input;
		// _flight_test_input.injection_output = inject_input + _raw_output;

		inject_output = inject_input + _raw_output;
	}

	return inject_output;
}

void FlightTestInput::vehicle_status_poll()
{
	if (_vehicle_status_sub <= 0) {
		_vehicle_status_sub = orb_subscribe(ORB_ID(vehicle_status));
	}

	bool vehicle_status_updated;
	orb_check(_vehicle_status_sub, &vehicle_status_updated);

	if (vehicle_status_updated) {
		orb_copy(ORB_ID(vehicle_status), _vehicle_status_sub, &_vehicle_status);
	}
}

void FlightTestInput::commander_state_poll()
{
	if (_commander_state_sub <= 0) {
		_commander_state_sub = orb_subscribe(ORB_ID(commander_state));
	}

	bool commander_state_updated;
	orb_check(_commander_state_sub, &commander_state_updated);

	if (commander_state_updated) {
		orb_copy(ORB_ID(commander_state), _commander_state_sub, &_commander_state);
	}
}
