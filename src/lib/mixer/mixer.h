/****************************************************************************
 *
 *   Copyright (C) 2012 PX4 Development Team. All rights reserved.
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
 * @file mixer.h
 *
 * Generic, programmable, procedural control signal mixers.
 *
 * This library implements a generic mixer interface that can be used
 * by any driver or subsytem that wants to combine several control signals
 * into a single output.
 *
 * Terminology
 * ===========
 *
 * control value
 *	A mixer input value, typically provided by some controlling
 *	component of the system.
 *
 * control group
 * 	A collection of controls provided by a single controlling component.
 *
 * actuator
 *	The mixer output value.
 *
 *
 * Mixing basics
 * =============
 *
 * An actuator derives its value from the combination of one or more
 * control values. Each of the control values is scaled according to
 * the actuator's configuration and then combined to produce the
 * actuator value, which may then be further scaled to suit the specific
 * output type.
 *
 * Internally, all scaling is performed using floating point values.
 * Inputs and outputs are clamped to the range -1.0 to 1.0.
 *
 * control    control   control
 *    |          |         |
 *    v          v         v
 *  scale      scale     scale
 *    |          |         |
 *    |          v         |
 *    +-------> mix <------+
 *               |
 *             scale
 *               |
 *               v
 *              out
 *
 * Scaling
 * -------
 *
 * Each scaler allows the input value to be scaled independently for
 * inputs greater/less than zero. An offset can be applied to the output,
 * as well as lower and upper boundary constraints.
 * Negative scaling factors cause the output to be inverted (negative input
 * produces positive output).
 *
 * Scaler pseudocode:
 *
 * if (input < 0)
 *     output = (input * NEGATIVE_SCALE) + OFFSET
 * else
 *     output = (input * POSITIVE_SCALE) + OFFSET
 *
 * if (output < LOWER_LIMIT)
 *     output = LOWER_LIMIT
 * if (output > UPPER_LIMIT)
 *     output = UPPER_LIMIT
 *
 *
 * Mixing
 * ------
 *
 * Mixing behaviour varies based on the specific mixer class; each
 * mixer class describes its behaviour in more detail.
 *
 *
 * Controls
 * --------
 *
 * The precise assignment of controls may vary depending on the
 * application, but the following assignments should be used
 * when appropriate.  Some mixer classes have specific assumptions
 * about the assignment of controls.
 *
 * control | standard meaning
 * --------+-----------------------
 *     0   | roll
 *     1   | pitch
 *     2   | yaw
 *     3   | primary thrust
 */


#pragma once

#include <stdint.h>
#include <lib/mathlib/mathlib.h>

#include "Mixer.hpp"
#include "MixerGroup.hpp"
#include "NullMixer.hpp"
#include "SimpleMixer.hpp"
#include "MultirotorMixer.hpp"
#include "HelicopterMixer.hpp"
