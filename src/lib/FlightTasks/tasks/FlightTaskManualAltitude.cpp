/****************************************************************************
 *
 *   Copyright (c) 2017 PX4 Development Team. All rights reserved.
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
 * @file FlightTaskManual.hpp
 *
 * Flight task for manual controlled altitude.
 *
 */

#include "FlightTaskManualAltitude.hpp"
#include <mathlib/mathlib.h>
#include <float.h>
#include <lib/geo/geo.h>

using namespace matrix;

FlightTaskManualAltitude::FlightTaskManualAltitude(control::SuperBlock *parent, const char *name) :
	FlightTaskManual(parent, name),
	_vel_max_down(parent, "MPC_Z_VEL_MAX_DN", false),
	_vel_max_up(parent, "MPC_Z_VEL_MAX_UP", false),
	_yaw_rate_scaling(parent, "MPC_MAN_Y_MAX", false)
{}

bool FlightTaskManualAltitude::activate()
{
	_pos_sp_z = _position(2);
	_yaw_sp = _yaw;
	return FlightTaskManual::activate();
}

bool FlightTaskManualAltitude::update()
{
	/* map stick to velocity */
	const float vel_max_z = (_sticks(2) > 0.0f) ? _vel_max_down.get() : _vel_max_up.get();
	float vel_sp_z = vel_max_z * _sticks_expo(2);

	const float yaw_rate_sp = _sticks(3) * _yaw_rate_scaling;

	if(fabsf(_sticks(2)) < FLT_EPSILON) {
		_setPositionSetpoint(Vector3f(NAN, NAN, _pos_sp_z));
		_setVelocitySetpoint(Vector3f(NAN, NAN, NAN));

	} else {
		_setPositionSetpoint(Vector3f(NAN, NAN, NAN));
		_setVelocitySetpoint(Vector3f(NAN, NAN, vel_sp_z));
		_pos_sp_z = _position(2) + vel_sp_z * _deltatime;
	}

	if(fabsf(_sticks(3) < FLT_EPSILON)) {
		_setYawspeedSetpoint(NAN);
		_setYawSetpoint(_yaw_sp);
	} else {
		_setYawspeedSetpoint(yaw_rate_sp);
		_setYawSetpoint(NAN);
		_yaw_sp = _wrap_pi(_yaw_sp + yaw_rate_sp * _deltatime)
	}

	return true;
}
