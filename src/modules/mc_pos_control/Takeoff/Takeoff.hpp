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
 * @file Takeoff.hpp
 *
 * A class handling all takeoff states and a smooth ramp up of the motors.
 */

#pragma once

#include <systemlib/hysteresis/hysteresis.h>
#include <drivers/drv_hrt.h>

using namespace time_literals;

enum class TakeoffState {
	disarmed = 0,
	spoolup,
	ready_for_takeoff,
	rampup,
	flight
};

class Takeoff
{
public:

	Takeoff() = default;
	~Takeoff() = default;

	/**
	 * Update the state for the takeoff.
	 * @param setpoint a vehicle_local_position_setpoint_s structure
	 * @return true if setpoint has updated correctly
	 */
	void updateTakeoffState(const bool armed, const bool landed, const bool want_takeoff,
				const float takeoff_desired_thrust, const bool skip_takeoff);
	float updateThrustRamp(const float dt, const float takeoff_desired_thrust);

	void setTakeoffRampTime(const float seconds) { _takeoff_ramp_time = seconds; }
	void setSpoolupTime(const float seconds) { _spoolup_time_hysteresis.set_hysteresis_time_from(false, (hrt_abstime)(seconds * (float)1_s)); }
	TakeoffState getTakeoffState() { return _takeoff_state; }

	// TODO: make this private as soon as tasks also run while disarmed and updateTakeoffState gets called all the time
	systemlib::Hysteresis _spoolup_time_hysteresis{false}; /**< becomes true MPC_SPOOLUP_TIME seconds after the vehicle was armed */

private:
	TakeoffState _takeoff_state = TakeoffState::disarmed;

	float _takeoff_ramp_time = 0.f;
	float _takeoff_ramp_thrust = 0.f;
};
