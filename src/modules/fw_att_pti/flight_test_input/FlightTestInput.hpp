#pragma once

//standard libs and lockguard
#include <containers/LockGuard.hpp>
#include <mathlib/mathlib.h>
#include <stdint.h>


//helper stuff
#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/defines.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/module_params.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/tasks.h>
#include <px4_platform_common/px4_work_queue/WorkItem.hpp>
#include <uORB/topics/parameter_update.h>
#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionCallback.hpp>
// #include <controllib/block/BlockParam.hpp>
#include <systemlib/mavlink_log.h>

// #include <uORB/topics/flight_test_input.h>
#include <uORB/topics/commander_state.h>
#include <uORB/topics/vehicle_status.h>


class FlightTestInput : public ModuleParams
{
	public:
		FlightTestInput();
		~FlightTestInput();

		// Update test input computation
		void update(float dt);

		// Inject current test input
		float inject(const uint8_t injection_point, const float inject_input);

	private:
		enum FlightTestInputState {
			TEST_INPUT_OFF = 0,
			TEST_INPUT_WAIT,
			TEST_INPUT_INIT,
			TEST_INPUT_RUNNING,
			TEST_INPUT_COMPLETE
		} _state;


		void computeSweep(float dt);
		// void computeDoublet(float dt);
		orb_advert_t	_mavlink_log_pub;

		float _time_running;
		float _raw_output;

		// frequency sweep sine sum
		float _sweep_sine_input;

		// define parameters for system, update this
		// parameters *_params;

		// Subscriptions
		//uORB::SubscriptionInterval _parameter_update_sub{ORB_ID(parameter_update), 1_s};

		DEFINE_PARAMETERS(
			(ParamInt<px4::params::FTI_MODE>) _param_fti_mode,
			(ParamInt<px4::params::FTI_ENABLE>) _param_fti_enable,
			(ParamInt<px4::params::FTI_INJXN_POINT>) _param_fti_injxn_point,
			(ParamFloat<px4::params::FTI_FS_DURATION>) _param_fti_fs_duration,
			(ParamFloat<px4::params::FTI_FS_FRQ_BEGIN>) _param_fti_fs_frq_begin,
			(ParamFloat<px4::params::FTI_FS_FRQ_END>) _param_fti_fs_frq_end,
			(ParamFloat<px4::params::FTI_FS_AMP_BEGIN>) _param_fti_fs_amp_begin,
			(ParamFloat<px4::params::FTI_FS_AMP_END>) _param_fti_fs_amp_end,
			(ParamFloat<px4::params::FTI_FS_FRQ_RAMP>) _param_fti_fs_ramp

		)

		// system main_state and nav_state captured during test input init
		uint8_t _main_state;
		uint8_t _nav_state;

		// check for vehicle status updates.
		int _vehicle_status_sub;
		int _commander_state_sub;

		struct vehicle_status_s _vehicle_status;
		struct commander_state_s _commander_state;

		void vehicle_status_poll();
		void commander_state_poll();

};

// #endif
