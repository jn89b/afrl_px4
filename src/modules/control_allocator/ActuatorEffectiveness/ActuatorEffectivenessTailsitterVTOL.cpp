/****************************************************************************
 *
 *   Copyright (c) 2021 PX4 Development Team. All rights reserved.
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
 * @file ActuatorEffectivenessTailsitterVTOL.hpp
 *
 * Actuator effectiveness for tailsitter VTOL
 */

#include "ActuatorEffectivenessTailsitterVTOL.hpp"

using namespace matrix;

ActuatorEffectivenessTailsitterVTOL::ActuatorEffectivenessTailsitterVTOL(ModuleParams *parent)
	: ModuleParams(parent), _mc_rotors(this), _control_surfaces(this)
{
	setFlightPhase(FlightPhase::HOVER_FLIGHT);
}
bool
ActuatorEffectivenessTailsitterVTOL::getEffectivenessMatrix(Configuration &configuration, bool force)
{
	if (!force) {
		return false;
	}

	// MC motors
	configuration.selected_matrix = 0;
	_mc_rotors.enableYawControl(_mc_rotors.geometry().num_rotors > 3); // enable MC yaw control if more than 3 rotors

	_mc_rotors.getEffectivenessMatrix(configuration, true);

	// Control Surfaces
	configuration.selected_matrix = 1;
	_first_control_surface_idx = configuration.num_actuators_matrix[configuration.selected_matrix];
	_control_surfaces.getEffectivenessMatrix(configuration, true);

	return true;
}

void ActuatorEffectivenessTailsitterVTOL::setFlightPhase(const FlightPhase &flight_phase)
{
	if (_flight_phase == flight_phase) {
		return;
	}

	ActuatorEffectiveness::setFlightPhase(flight_phase);

	// update stopped motors //TODO: add option to switch off certain motors in FW
	switch (flight_phase) {
	case FlightPhase::FORWARD_FLIGHT:
		_stopped_motors = 0;
		break;

	case FlightPhase::HOVER_FLIGHT:
	case FlightPhase::TRANSITION_FF_TO_HF:
	case FlightPhase::TRANSITION_HF_TO_FF:
		_stopped_motors = 0;
		break;
	}
}
