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
 * @file TranslationControl.hpp
 *
 * @inputs: position-, velocity-, acceleration-, thrust- setpoints
 * @outputs: thrust vector
 *
 */

#include <matrix/matrix/math.hpp>

#include <uORB/topics/vehicle_local_position.h>
#include <uORB/topics/vehicle_local_position_setpoint.h>
#include <systemlib/param/param.h>

#pragma once

/* Variable constraints based on mode:
 * Eventually this structure should be part of local position message
 */
namespace Controller
{
struct Constraints {
	float tilt_max;
};
}

class TranslationControl
{
public:
	TranslationControl();

	~TranslationControl() {};

	void updateState(const struct vehicle_local_position_s state, const matrix::Vector3f &vel_dot,
			 const matrix::Matrix<float, 3, 3> &R);
	void updateSetpoint(struct vehicle_local_position_setpoint_s setpoint);
	void updateConstraints(const Controller::Constraints &constraints);
	void generateThrustYawSetpoint(const float &dt);

	matrix::Vector3f getThrustSetpoint() {return _thr_sp;}
	float getYawSetpoint() { return _yaw_sp;}
	float getYawspeedSetpoint() {return _yawspeed_sp;}
	float getThrottle() {return _throttle;}
	matrix::Vector3f getVelSp() {return _vel_sp;}
	matrix::Vector3f getPosSp() {return _pos_sp;}

private:

	/* states */
	matrix::Vector3f _pos{};
	matrix::Vector3f _vel{};
	matrix::Vector3f _vel_dot{};
	matrix::Vector3f _acc{};
	matrix::Vector3f _thr{};
	float _yaw{0.0f};
	matrix::Matrix<float, 3, 3> _R{};

	/* setpoints */
	matrix::Vector3f _pos_sp{};
	matrix::Vector3f _vel_sp{};
	matrix::Vector3f _acc_sp{};
	matrix::Vector3f _thr_sp{};
	float _yaw_sp{};
	float _yawspeed_sp{};
	float _throttle{};

	/* other variables */
	matrix::Vector3f _thr_int{};
	float _yaw_sp_int{};
	Controller::Constraints _constraints{};

	/* params handles */
	int _parameter_sub{-1};
	param_t _Pz_h{PARAM_INVALID};
	param_t _Pvz_h{PARAM_INVALID};
	param_t _Ivz_h{PARAM_INVALID};
	param_t _Dvz_h{PARAM_INVALID};
	param_t _Pxy_h{PARAM_INVALID};
	param_t _Pvxy_h{PARAM_INVALID};
	param_t _Ivxy_h{PARAM_INVALID};
	param_t _Dvxy_h{PARAM_INVALID};
	param_t _VelMaxXY_h{PARAM_INVALID};
	param_t _VelMaxZdown_h{PARAM_INVALID};
	param_t _VelMaxZup_h{PARAM_INVALID};
	param_t _ThrHover_h{PARAM_INVALID};
	param_t _ThrMax_h{PARAM_INVALID};
	param_t _ThrMin_h{PARAM_INVALID};
	param_t _YawRateMax_h{PARAM_INVALID};
	param_t _Pyaw_h{PARAM_INVALID}; //only temporary: this will be moved into attitude controller

	/* params */
	matrix::Vector3f Pp, Pv, Iv, Dv = matrix::Vector3f{0.0f, 0.0f, 0.0f};
	float _VelMaxXY{};
	float _VelMaxZ[2]; //index 0: index up; 1: down
	float _ThrHover{0.5f};
	float _ThrLimit[2]; //index 0: max, index 1: min
	float _Pyaw{};
	float _YawRateMax{};

	/* helper methods */
	void _interfaceMapping();
	void _positionController();
	void _velocityController(const float &dt);
	void _yawController(const float &dt);
	void _updateParams();
	void _setParams();
};
