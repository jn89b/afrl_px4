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
 * @file PositionControl.cpp
 */

#include "PositionControl.hpp"
#include <float.h>
#include <mathlib/mathlib.h>
#include "Utility/ControlMath.hpp"

using namespace matrix;

PositionControl::PositionControl(ModuleParams *parent) :
ModuleParams(parent)
{}

void PositionControl::updateState(const vehicle_local_position_s &state, const Vector3f &vel_dot)
{
	_pos = Vector3f(&state.x);
	_vel = Vector3f(&state.vx);
	_yaw = state.yaw;
	_vel_dot = vel_dot;
}

void PositionControl::updateSetpoint(const vehicle_local_position_setpoint_s &setpoint)
{
	_pos_sp = Vector3f(&setpoint.x);
	_vel_sp = Vector3f(&setpoint.vx);
	_acc_sp = Vector3f(&setpoint.acc_x);
	_thr_sp = Vector3f(setpoint.thrust);
	_yaw_sp = setpoint.yaw;
	_yawspeed_sp = setpoint.yawspeed;
	_interfaceMapping();

	// If full manual is required (thrust already generated), don't run position/velocity
	// controller and just return thrust.
	_skip_controller = false;

	if (PX4_ISFINITE(setpoint.thrust[0]) && PX4_ISFINITE(setpoint.thrust[1]) && PX4_ISFINITE(setpoint.thrust[2])) {
		_skip_controller = true;
	}
}

void PositionControl::generateThrustYawSetpoint(const float &dt)
{
	if (_skip_controller) {
		// Already received a valid thrust set-point.
		// Limit the thrust vector.
		float thr_mag = _thr_sp.length();

		if (thr_mag > MPC_THR_MAX.get()) {
			_thr_sp = _thr_sp.normalized() * MPC_THR_MAX.get();

		} else if (thr_mag < MPC_THR_MIN.get() && thr_mag > FLT_EPSILON) {
			_thr_sp = _thr_sp.normalized() * MPC_THR_MIN.get();
		}

		// Just set the set-points equal to the current vehicle state.
		_pos_sp = _pos;
		_vel_sp = _vel;
		_acc_sp = _acc;

	} else {
		_positionController();
		_velocityController(dt);
	}
}

void PositionControl::_interfaceMapping()
{
	// Respects FlightTask interface, where NAN-set-points are of no interest
	// and do not require control. A valide position and velocity setpoint will
	// be mapped to a desired position setpoint with a feed-forward term.
	for (int i = 0; i <= 2; i++) {

		if (PX4_ISFINITE(_pos_sp(i))) {
			// Position control is required

			if (!PX4_ISFINITE(_vel_sp(i))) {
				// Velocity is not used as feedforward term.
				_vel_sp(i) = 0.0f;
			}

			// thrust setpoint is not supported in position control
			_thr_sp(i) = 0.0f;

		} else if (PX4_ISFINITE(_vel_sp(i))) {

			// Velocity controller is active without position control.
			// Set the desired position set-point equal to current position
			// such that error is zero.
			_pos_sp(i) = _pos(i);
			// thrust setpoint is not supported in position control
			_thr_sp(i) = 0.0f;

		} else if (PX4_ISFINITE(_thr_sp(i))) {

			// Thrust setpoint was generated from sticks directly.
			// Set all other set-points equal MC states.
			_pos_sp(i) = _pos(i);
			_vel_sp(i) = _vel(i);
			// Reset the Integral term.
			_thr_int(i) = 0.0f;
			// Don't require velocity derivative.
			_vel_dot(i) = 0.0f;

		} else {
			PX4_WARN("TODO: add safety");
		}
	}

	if (!PX4_ISFINITE(_yawspeed_sp)) {
		// Set the yawspeed to 0 since not used.
		_yawspeed_sp = 0.0f;
	}

	if (!PX4_ISFINITE(_yaw_sp)) {
		// Set the yaw-sp equal the current yaw.
		// That is the best we can do and it also
		// agrees with FlightTask-interface definition.
		_yaw_sp = _yaw;
	}
}

void PositionControl::_positionController()
{
	// P-position controller
	Vector3f vel_sp_position = (_pos_sp - _pos).emult(Vector3f(MPC_XY_P.get(), MPC_XY_P.get(), MPC_Z_P.get()));
	_vel_sp = vel_sp_position + _vel_sp;

	// Constrain horizontal velocity by prioritizing the velocity component along the
	// the desired position setpoint over the feed-forward term.
	Vector2f vel_sp_xy = ControlMath::constrainXY(Vector2f(&vel_sp_position(0)),
			     Vector2f(&(_vel_sp - vel_sp_position)(0)), MPC_XY_VEL_MAX.get());
	_vel_sp(0) = vel_sp_xy(0);
	_vel_sp(1) = vel_sp_xy(1);
	// Constrain velocity in z-direction.
	_vel_sp(2) = math::constrain(_vel_sp(2), -_constraints.vel_max_z_up, MPC_Z_VEL_MAX_DN.get());
}

void PositionControl::_velocityController(const float &dt)
{

	// Generate desired thrust setpoint.
	// PID
	// u_des = P(vel_err) + D(vel_err_dot) + I(vel_integral)
	// Umin <= u_des <= Umax
	//
	// Anti-Windup:
	// u_des = _thr_sp; r = _vel_sp; y = _vel
	// u_des >= Umax and r - y >= 0 => Saturation = true
	// u_des >= Umax and r - y <= 0 => Saturation = false
	// u_des <= Umin and r - y <= 0 => Saturation = true
	// u_des <= Umin and r - y >= 0 => Saturation = false
	//
	// 	Notes:
	// - PID implementation is in NED-frame
	// - control output in D-direction has priority over NE-direction
	// - the equilibrium point for the PID is at hover-thrust
	// - the maximum tilt cannot exceed 90 degrees. This means that it is
	// 	 not possible to have a desired thrust direction pointing in the positive
	// 	 D-direction (= downward)
	// - the desired thrust in D-direction is limited by the thrust limits
	// - the desired thrust in NE-direction is limited by the thrust excess after
	// 	 consideration of the desired thrust in D-direction. In addition, the thrust in
	// 	 NE-direction is also limited by the maximum tilt.

	Vector3f vel_err = _vel_sp - _vel;

	// Consider thrust in D-direction.
	float thrust_desired_D = MPC_Z_VEL_P.get() * vel_err(2) +  MPC_Z_VEL_D.get() * _vel_dot(2) + _thr_int(2) - MPC_THR_HOVER.get();

	// The Thrust limits are negated and swapped due to NED-frame.
	float uMax = -MPC_THR_MIN.get();
	float uMin = -MPC_THR_MAX.get();

	// Apply Anti-Windup in D-direction.
	bool stop_integral_D = (thrust_desired_D >= uMax && vel_err(2) >= 0.0f) ||
			       (thrust_desired_D <= uMin && vel_err(2) <= 0.0f);

	if (!stop_integral_D) {
		_thr_int(2) += vel_err(2) * MPC_Z_VEL_I.get() * dt;

	}

	// Saturate thrust setpoint in D-direction.
	_thr_sp(2) = math::constrain(thrust_desired_D, uMin, uMax);

	if (fabsf(_thr_sp(0)) + fabsf(_thr_sp(1))  > FLT_EPSILON) {
		// Thrust set-point in NE-direction is already provided. Only
		// scaling by the maximum tilt is required.
		float thr_xy_max = fabsf(_thr_sp(2)) * tanf(MPC_MAN_TILT_MAX.get());
		_thr_sp(0) *= thr_xy_max;
		_thr_sp(1) *= thr_xy_max;

	} else {
		// PID-velocity controller for NE-direction.
		Vector2f thrust_desired_NE;
		thrust_desired_NE(0) = MPC_XY_VEL_P.get() * vel_err(0) + MPC_XY_VEL_D.get() * _vel_dot(0) + _thr_int(0);
		thrust_desired_NE(1) = MPC_XY_VEL_P.get() * vel_err(1) + MPC_XY_VEL_D.get() * _vel_dot(1) + _thr_int(1);

		// Get maximum allowed thrust in NE based on tilt and excess thrust.
		float thrust_max_NE_tilt = fabsf(_thr_sp(2)) * tanf(_constraints.tilt_max);
		float thrust_max_NE = sqrtf(MPC_THR_MAX.get() * MPC_THR_MAX.get() - _thr_sp(2) * _thr_sp(2));
		thrust_max_NE = math::min(thrust_max_NE_tilt, thrust_max_NE);

		// Get the direction of (r-y) in NE-direction.
		float direction_NE = Vector2f(&vel_err(0)) * Vector2f(&_vel_sp(0));

		// Apply Anti-Windup in NE-direction.
		bool stop_integral_NE = (thrust_desired_NE * thrust_desired_NE >= thrust_max_NE * thrust_max_NE &&
					 direction_NE >= 0.0f);

		if (!stop_integral_NE) {
			_thr_int(0) += vel_err(0) * MPC_XY_VEL_I.get() * dt;
			_thr_int(1) += vel_err(1) * MPC_XY_VEL_I.get() * dt;
		}

		// Saturate thrust in NE-direction.
		_thr_sp(0) = thrust_desired_NE(0);
		_thr_sp(1) = thrust_desired_NE(1);

		if (thrust_desired_NE * thrust_desired_NE > thrust_max_NE * thrust_max_NE) {
			float mag = thrust_desired_NE.length();
			_thr_sp(0) = thrust_desired_NE(0) / mag * thrust_max_NE;
			_thr_sp(1) = thrust_desired_NE(1) / mag * thrust_max_NE;

		}
	}
}

void PositionControl::updateConstraints(const Controller::Constraints &constraints)
{
	_constraints = constraints;

	// Check if adjustable constraints are below global constraints. If they are not stricter than global
	// constraints, then just use global constraints for the limits.

	if (!PX4_ISFINITE(constraints.tilt_max) || !(constraints.tilt_max < MPC_TILTMAX_AIR.get())) {
		_constraints.tilt_max = MPC_TILTMAX_AIR.get();
	}

	if (!PX4_ISFINITE(constraints.vel_max_z_up) || !(constraints.vel_max_z_up < MPC_Z_VEL_MAX_UP.get())) {
		_constraints.vel_max_z_up = MPC_Z_VEL_MAX_UP.get();
	}
}

void PositionControl::overwriteParams()
{
	// Tilt needs to be in radians
	MPC_TILTMAX_AIR.set(math::radians(MPC_TILTMAX_AIR.get()));
	MPC_MAN_TILT_MAX.set(math::radians(MPC_MAN_TILT_MAX.get()));
}
