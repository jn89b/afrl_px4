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
 * @file FlightTaskOrbit.hpp
 *
 * Flight task for orbiting in a circle around a target position
 *
 * @author Matthias Grob <maetugr@gmail.com>
 */

#pragma once

#include "FlightTaskManualPosition.hpp"

class FlightTaskSport : public FlightTaskManualPosition
{
public:
	FlightTaskSport(control::SuperBlock *parent, const char *name) :
		FlightTaskManualPosition(parent, name),
		_velocity_hor_max(parent, "MPC_XY_VEL_MAX", false)
	{ }

	virtual ~FlightTaskSport() = default;

protected:
	void _scaleVelocity(matrix::Vector3f &velocity)
	{
		const matrix::Vector3f velocity_scale(_velocity_hor_max.get(),
						      _velocity_hor_max.get(),
						      (velocity(2) > 0.0f) ? _vel_max_down.get() : _vel_max_up.get());
		velocity = velocity.emult(velocity_scale);
	}

private:
	control::BlockParamFloat _velocity_hor_max; /**< maximal allowed horizontal speed, in sport mode full stick input*/

};
