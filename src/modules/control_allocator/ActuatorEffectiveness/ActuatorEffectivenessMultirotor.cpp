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

/**
 * @file ActuatorEffectivenessMultirotor.hpp
 *
 * Actuator effectiveness computed from rotors position and orientation
 *
 * @author Julien Lecoeur <julien.lecoeur@gmail.com>
 */

#include "ActuatorEffectivenessMultirotor.hpp"

ActuatorEffectivenessMultirotor::ActuatorEffectivenessMultirotor():
	ModuleParams(nullptr)
{
}

bool
ActuatorEffectivenessMultirotor::getEffectivenessMatrix(matrix::Matrix<float, NUM_AXES, NUM_ACTUATORS> &matrix)
{
	// Check if parameters have changed
	if (_updated || _parameter_update_sub.updated()) {
		// clear update
		parameter_update_s param_update;
		_parameter_update_sub.copy(&param_update);

		updateParams();
		_updated = false;

		// Get multirotor geometry
		MultirotorGeometry geometry = {};
		geometry.rotors[0].position_x = _param_ca_mc_r0_px.get();
		geometry.rotors[0].position_y = _param_ca_mc_r0_py.get();
		geometry.rotors[0].position_z = _param_ca_mc_r0_pz.get();
		geometry.rotors[0].axis_x = _param_ca_mc_r0_ax.get();
		geometry.rotors[0].axis_y = _param_ca_mc_r0_ay.get();
		geometry.rotors[0].axis_z = _param_ca_mc_r0_az.get();
		geometry.rotors[0].thrust_coef = _param_ca_mc_r0_ct.get();
		geometry.rotors[0].moment_ratio = _param_ca_mc_r0_km.get();

		geometry.rotors[1].position_x = _param_ca_mc_r1_px.get();
		geometry.rotors[1].position_y = _param_ca_mc_r1_py.get();
		geometry.rotors[1].position_z = _param_ca_mc_r1_pz.get();
		geometry.rotors[1].axis_x = _param_ca_mc_r1_ax.get();
		geometry.rotors[1].axis_y = _param_ca_mc_r1_ay.get();
		geometry.rotors[1].axis_z = _param_ca_mc_r1_az.get();
		geometry.rotors[1].thrust_coef = _param_ca_mc_r1_ct.get();
		geometry.rotors[1].moment_ratio = _param_ca_mc_r1_km.get();

		geometry.rotors[2].position_x = _param_ca_mc_r2_px.get();
		geometry.rotors[2].position_y = _param_ca_mc_r2_py.get();
		geometry.rotors[2].position_z = _param_ca_mc_r2_pz.get();
		geometry.rotors[2].axis_x = _param_ca_mc_r2_ax.get();
		geometry.rotors[2].axis_y = _param_ca_mc_r2_ay.get();
		geometry.rotors[2].axis_z = _param_ca_mc_r2_az.get();
		geometry.rotors[2].thrust_coef = _param_ca_mc_r2_ct.get();
		geometry.rotors[2].moment_ratio = _param_ca_mc_r2_km.get();

		geometry.rotors[3].position_x = _param_ca_mc_r3_px.get();
		geometry.rotors[3].position_y = _param_ca_mc_r3_py.get();
		geometry.rotors[3].position_z = _param_ca_mc_r3_pz.get();
		geometry.rotors[3].axis_x = _param_ca_mc_r3_ax.get();
		geometry.rotors[3].axis_y = _param_ca_mc_r3_ay.get();
		geometry.rotors[3].axis_z = _param_ca_mc_r3_az.get();
		geometry.rotors[3].thrust_coef = _param_ca_mc_r3_ct.get();
		geometry.rotors[3].moment_ratio = _param_ca_mc_r3_km.get();

		geometry.rotors[4].position_x = _param_ca_mc_r4_px.get();
		geometry.rotors[4].position_y = _param_ca_mc_r4_py.get();
		geometry.rotors[4].position_z = _param_ca_mc_r4_pz.get();
		geometry.rotors[4].axis_x = _param_ca_mc_r4_ax.get();
		geometry.rotors[4].axis_y = _param_ca_mc_r4_ay.get();
		geometry.rotors[4].axis_z = _param_ca_mc_r4_az.get();
		geometry.rotors[4].thrust_coef = _param_ca_mc_r4_ct.get();
		geometry.rotors[4].moment_ratio = _param_ca_mc_r4_km.get();

		geometry.rotors[5].position_x = _param_ca_mc_r5_px.get();
		geometry.rotors[5].position_y = _param_ca_mc_r5_py.get();
		geometry.rotors[5].position_z = _param_ca_mc_r5_pz.get();
		geometry.rotors[5].axis_x = _param_ca_mc_r5_ax.get();
		geometry.rotors[5].axis_y = _param_ca_mc_r5_ay.get();
		geometry.rotors[5].axis_z = _param_ca_mc_r5_az.get();
		geometry.rotors[5].thrust_coef = _param_ca_mc_r5_ct.get();
		geometry.rotors[5].moment_ratio = _param_ca_mc_r5_km.get();

		geometry.rotors[6].position_x = _param_ca_mc_r6_px.get();
		geometry.rotors[6].position_y = _param_ca_mc_r6_py.get();
		geometry.rotors[6].position_z = _param_ca_mc_r6_pz.get();
		geometry.rotors[6].axis_x = _param_ca_mc_r6_ax.get();
		geometry.rotors[6].axis_y = _param_ca_mc_r6_ay.get();
		geometry.rotors[6].axis_z = _param_ca_mc_r6_az.get();
		geometry.rotors[6].thrust_coef = _param_ca_mc_r6_ct.get();
		geometry.rotors[6].moment_ratio = _param_ca_mc_r6_km.get();

		geometry.rotors[7].position_x = _param_ca_mc_r7_px.get();
		geometry.rotors[7].position_y = _param_ca_mc_r7_py.get();
		geometry.rotors[7].position_z = _param_ca_mc_r7_pz.get();
		geometry.rotors[7].axis_x = _param_ca_mc_r7_ax.get();
		geometry.rotors[7].axis_y = _param_ca_mc_r7_ay.get();
		geometry.rotors[7].axis_z = _param_ca_mc_r7_az.get();
		geometry.rotors[7].thrust_coef = _param_ca_mc_r7_ct.get();
		geometry.rotors[7].moment_ratio = _param_ca_mc_r7_km.get();

		_num_actuators = computeEffectivenessMatrix(geometry, matrix);
		return true;
	}

	return false;
}

int
ActuatorEffectivenessMultirotor::computeEffectivenessMatrix(const MultirotorGeometry &geometry,
		matrix::Matrix<float, NUM_AXES, NUM_ACTUATORS> &effectiveness)
{
	int num_actuators = 0;

	effectiveness.setZero();

	for (size_t i = 0; i < NUM_ROTORS_MAX; i++) {
		// Get rotor axis
		matrix::Vector3f axis(
			geometry.rotors[i].axis_x,
			geometry.rotors[i].axis_y,
			geometry.rotors[i].axis_z
		);

		// Normalize axis
		float axis_norm = axis.norm();

		if (axis_norm > FLT_EPSILON) {
			axis /= axis_norm;

		} else {
			// Bad axis definition, ignore this rotor
			continue;
		}

		// Get rotor position
		matrix::Vector3f position(
			geometry.rotors[i].position_x,
			geometry.rotors[i].position_y,
			geometry.rotors[i].position_z
		);

		// Get coefficients
		float ct = geometry.rotors[i].thrust_coef;
		float km = geometry.rotors[i].moment_ratio;

		if (fabsf(ct) < FLT_EPSILON) {
			continue;
		}

		// Compute thrust generated by this rotor
		matrix::Vector3f thrust = ct * axis;

		// Compute moment generated by this rotor
		matrix::Vector3f moment = ct * position.cross(axis) - ct * km * axis;

		// Fill corresponding items in effectiveness matrix
		for (size_t j = 0; j < 3; j++) {
			effectiveness(j, i) = moment(j);
			effectiveness(j + 3, i) = thrust(j);
		}

		num_actuators = i + 1;
	}

	return num_actuators;
}
