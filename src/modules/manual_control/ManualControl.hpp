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

#pragma once

#include <px4_platform_common/defines.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/module_params.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>
#include <lib/hysteresis/hysteresis.h>
#include <lib/perf/perf_counter.h>
#include <uORB/topics/manual_control_input.h>
#include <uORB/topics/manual_control_switches.h>
#include <uORB/topics/manual_control_setpoint.h>
#include <uORB/topics/parameter_update.h>
#include <uORB/Publication.hpp>
#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionCallback.hpp>
#include "ManualControlSelector.hpp"

using namespace time_literals;

namespace manual_control
{

class ManualControl : public ModuleBase<ManualControl>, public ModuleParams, public px4::ScheduledWorkItem
{
public:
	ManualControl();
	~ManualControl() override;

	/** @see ModuleBase */
	static int task_spawn(int argc, char *argv[]);

	/** @see ModuleBase */
	static int custom_command(int argc, char *argv[]);

	/** @see ModuleBase */
	static int print_usage(const char *reason = nullptr);

	bool init();

	int print_status() override;

private:
	static constexpr int MAX_MANUAL_INPUT_COUNT = 3;

	enum class ArmingOrigin {
		GESTURE = 1,
		SWITCH = 2,
		BUTTON = 3,
	};

	void Run() override;

	void evaluate_mode_slot(uint8_t mode_slot);
	void send_mode_command(int32_t commander_main_state);
	void send_arm_command(bool should_arm, ArmingOrigin origin);
	void send_rtl_command();
	void send_loiter_command();
	void send_offboard_command();
	void send_termination_command(bool should_terminate);
	void publish_landing_gear(int8_t action);
	void send_vtol_transition_command(uint8_t action);

	uORB::Publication<manual_control_setpoint_s> _manual_control_setpoint_pub{ORB_ID(manual_control_setpoint)};

	uORB::SubscriptionInterval _parameter_update_sub{ORB_ID(parameter_update), 1_s};
	uORB::SubscriptionCallbackWorkItem _manual_control_input_subs[MAX_MANUAL_INPUT_COUNT] {
		{this, ORB_ID(manual_control_input), 0},
		{this, ORB_ID(manual_control_input), 1},
		{this, ORB_ID(manual_control_input), 2},
	};
	uORB::SubscriptionCallbackWorkItem _manual_control_switches_sub{this, ORB_ID(manual_control_switches)};

	systemlib::Hysteresis _stick_arm_hysteresis{false};
	systemlib::Hysteresis _stick_disarm_hysteresis{false};

	ManualControlSelector _selector;
	bool _published_invalid_once{false};
	int _last_selected_input{-1};

	bool _previous_arm_gesture{false};
	bool _previous_disarm_gesture{false};

	float _previous_x{NAN};
	float _previous_y{NAN};
	float _previous_z{NAN};
	float _previous_r{NAN};

	manual_control_switches_s _previous_switches{};
	bool _previous_switches_initialized{false};
	int32_t _last_mode_slot_flt{-1};

	perf_counter_t	_loop_perf{perf_alloc(PC_ELAPSED, MODULE_NAME": cycle")};
	perf_counter_t	_loop_interval_perf{perf_alloc(PC_INTERVAL, MODULE_NAME": interval")};

	DEFINE_PARAMETERS(
		(ParamInt<px4::params::COM_RC_IN_MODE>) _param_com_rc_in_mode,
		(ParamFloat<px4::params::COM_RC_LOSS_T>) _param_com_rc_loss_t,
		(ParamFloat<px4::params::COM_RC_STICK_OV>) _param_com_rc_stick_ov,
		(ParamInt<px4::params::COM_RC_ARM_HYST>) _param_rc_arm_hyst,
		(ParamInt<px4::params::COM_FLTMODE1>) _param_fltmode_1,
		(ParamInt<px4::params::COM_FLTMODE2>) _param_fltmode_2,
		(ParamInt<px4::params::COM_FLTMODE3>) _param_fltmode_3,
		(ParamInt<px4::params::COM_FLTMODE4>) _param_fltmode_4,
		(ParamInt<px4::params::COM_FLTMODE5>) _param_fltmode_5,
		(ParamInt<px4::params::COM_FLTMODE6>) _param_fltmode_6
	)
};
} // namespace manual_control
