#include <FlightTestInput.hpp>

#include <systemlib/err.h>
#include <fcntl.h>
#include <unistd.h>

#include <controllib/block/BlockParam.hpp>
#include <systemlib/mavlink_log.h>
#include <uORB/topics/flight_test_input.h>
#include <uORB/topics/commander_state.h>
#include <uORB/topics/vehicle_status.h>
// _doublet_pulse_length(this, "PULSE_LEN"),
// _doublet_pulse_amplitude(this, "PULSE_AMP"),

FlightTestInput::FlightTestInput() :
	SuperBlock(NULL, "FTI"),
	_state(TEST_INPUT_OFF),
	_mavlink_log_pub(nullptr),
	_time_running(0),
	_raw_output(0),
	_sweep_sine_input(0),
	_mode(this, "MODE"),
	_enable(this, "ENABLE"),
	_injection_point(this, "INJXN_POINT"),
	_sweep_duration(this, "FS_DURATION"),
	_sweep_freq_begin(this, "FS_FRQ_BEGIN"),
	_sweep_freq_end(this, "FS_FRQ_END"),
	_sweep_freq_ramp(this, "FS_FRQ_RAMP"),
	_sweep_amplitude_begin(this, "FS_AMP_BEGIN"),
	_sweep_amplitude_end(this, "FS_AMP_END"),
	_loop_gain(this, "LOOP_GAIN"),
	_main_state(0),
	_nav_state(0),
	_vehicle_status_sub(-1),
	_commander_state_sub(-1)
{
	_flight_test_input = {};
	_vehicle_status = {};
	_commander_state = {};

	_enable.set(0);
	_enable.commit();
	_loop_gain.set(_loop_gain_default);
	_loop_gain.commit();
}

FlightTestInput::~FlightTestInput()
{
}

void
FlightTestInput::update(float dt)
{
	/* check vehicle status for changes to publication state */
	vehicle_status_poll();
	commander_state_poll();

	_enable.update();
	_loop_gain.update();

	switch (_state) {
	case TEST_INPUT_OFF:

		_state = TEST_INPUT_WAIT;

		break;

	case TEST_INPUT_WAIT:

		if (_enable.get() == 1) {
			_state = TEST_INPUT_INIT;
			_fti_set_loop_gain = _loop_gain.get();
		}

		return;

	case TEST_INPUT_INIT:
		// Initialize sweep variables and store current autopilot mode
		mavlink_log_info(&_mavlink_log_pub, "#Flight test input injection intializing");

		// only use TI (test input) param values as they were set during init
		updateParams();

		// abort sweep if any system mode (main_state or nav_state) change
		_main_state = _commander_state.main_state;
		_nav_state = _vehicle_status.nav_state;

		_time_running = 0;
		_raw_output = 0;

		// sine sum for frequency sweep calculation
		_sweep_sine_input = 0;

		_state = TEST_INPUT_RUNNING;
		break;

	case TEST_INPUT_RUNNING:
		// only run if main state and nav state are unchanged and sweep mode is valid
		if ((_main_state == _commander_state.main_state)
		    && (_nav_state == _vehicle_status.nav_state)
		    && (_enable.get() == 1)
		    && (_mode.get() == 0 || _mode.get() == 1)
		   ) {

			if (_mode.get() == 0) {
				// Frequency sweep mode
				computeSweep(dt);
			}
			// } else if (_mode.get() == 1) {
			// 	// Doublet mode
			// 	computeDoublet(dt);
			// }

			// increment time
			_time_running += dt;

		} else {
			mavlink_log_info(&_mavlink_log_pub, "#Flight test input aborted");
			_state = TEST_INPUT_COMPLETE;
		}

		break;

	case TEST_INPUT_COMPLETE:

		_raw_output = 0;

		_flight_test_input.sweep_time_segment_pct = 0;
		_flight_test_input.sweep_frequency = 0;
		_flight_test_input.sweep_amplitude = 0;

		_flight_test_input.doublet_amplitude = 0;

		_flight_test_input.injection_input = 0;
		_flight_test_input.injection_output = 0;

		// only return to off state once param is reset to 0
		if (_enable.get() == 0) {
			mavlink_log_info(&_mavlink_log_pub, "#Flight test input resetting");
			_state = TEST_INPUT_OFF;
			_loop_gain.set(_loop_gain_default);
			_loop_gain.commit();
		}

		break;
	}

	_flight_test_input.timestamp = hrt_absolute_time();
	_flight_test_input.state = _state;
	_flight_test_input.mode = _mode.get();
	_flight_test_input.injection_point = _injection_point.get();
	_flight_test_input.raw_output = _raw_output;

	if (_flight_test_input_pub != nullptr) {
		// PX4_INFO("publishing");
		orb_publish(ORB_ID(flight_test_input), _flight_test_input_pub, &_flight_test_input);

	} else {
		// PX4_INFO("advertizing");
		_flight_test_input_pub = orb_advertise(ORB_ID(flight_test_input), &_flight_test_input);
	}
}

void
FlightTestInput::computeSweep(float dt)
{
	if (_time_running <= _sweep_duration.get()) {
		// Compute Current Sweep
		float time_segment_pct = (_time_running) / math::constrain(_sweep_duration.get(), 0.01f, 1000.0f);
		float amplitude = (_sweep_amplitude_end.get() - _sweep_amplitude_begin.get()) * time_segment_pct +
				  _sweep_amplitude_begin.get();
		float frequency = (_sweep_freq_end.get() - _sweep_freq_begin.get()) * powf(time_segment_pct,
				  _sweep_freq_ramp.get()) + _sweep_freq_begin.get();

		// Compute Frequency Sweep Segment
		_sweep_sine_input += 2 * M_PI_F * frequency * dt;

		if (_sweep_sine_input > (2 * M_PI_F)) {
			_sweep_sine_input -= 2 * M_PI_F;
		}

		// double time_seg = time_segment_pct;
		// double amp = amplitude;
		// double freq = frequency;
		// PX4_INFO("time, amp, freq, :\t%8.4f\t%8.4f\t%8.4f",
		// 		time_seg,
		// 		amp,
		// 		freq);


		_raw_output = amplitude * sinf(_sweep_sine_input);

		_flight_test_input.sweep_time_segment_pct = time_segment_pct;
		_flight_test_input.sweep_amplitude = amplitude;
		_flight_test_input.sweep_frequency = frequency;

	} else {
		mavlink_log_info(&_mavlink_log_pub, "#Frequency Sweep complete");
		_state = TEST_INPUT_COMPLETE;
		_loop_gain.set(_loop_gain_default);
		_loop_gain.commit();
	}
}

// void
// FlightTestInput::computeDoublet(float dt)
// {
// 	//
// 	//          FTI_PULSE_LEN
// 	//           |~~~~~~~~~|
// 	//           |         |
// 	//           |         |
// 	//     1s    |         |             1s
// 	//  ~~~~~~~~~|         |         |~~~~~~~~~~
// 	//                     |         |
// 	//                     |         |
// 	//                     |         |
// 	//                     |~~~~~~~~~|
// 	//                     FTI_PULSE_LEN
// 	//

// 	const float lead_in_time = 1;
// 	const float lead_out_time = 1;

// 	if (_time_running < lead_in_time) {
// 		// lead in
// 		_raw_output = 0;

// 	} else if (_time_running < (lead_in_time + _doublet_pulse_length.get())) {
// 		_raw_output = _doublet_pulse_amplitude.get();

// 	} else if (_time_running < (lead_in_time + 2 * _doublet_pulse_length.get())) {
// 		_raw_output = -_doublet_pulse_amplitude.get();

// 	} else if (_time_running < (lead_in_time + 2 * _doublet_pulse_length.get() + lead_out_time)) {
// 		// lead out
// 		_raw_output = 0;

// 	} else {
// 		mavlink_log_info(&_mavlink_log_pub, "#Doublet complete");
// 		_state = TEST_INPUT_COMPLETE;
// 	}

// 	// _flight_test_input.doublet_amplitude = _raw_output;
// }

float
FlightTestInput::inject(const uint8_t injection_point, const float inject_input)
{
	float inject_output = inject_input;

	if ((_state == TEST_INPUT_RUNNING) && (injection_point == _injection_point.get())) {
		// update logging
		_flight_test_input.injection_input = inject_input;
		_flight_test_input.injection_output = (inject_input*_fti_set_loop_gain) + _raw_output;

		inject_output = (inject_input*_fti_set_loop_gain) + _raw_output;
		// double stupid_val = inject_output;
		// PX4_INFO("Old:%d\t,\t%8.4f", injection_point, double(stupid_val));
	}

	return inject_output;
}

void
FlightTestInput::vehicle_status_poll()
{
	if (_vehicle_status_sub <= 0) {
		_vehicle_status_sub = orb_subscribe(ORB_ID(vehicle_status));
	}

	bool vehicle_status_updated;
	orb_check(_vehicle_status_sub, &vehicle_status_updated);

	if (vehicle_status_updated) {
		orb_copy(ORB_ID(vehicle_status), _vehicle_status_sub, &_vehicle_status);
	}
}

void
FlightTestInput::commander_state_poll()
{
	if (_commander_state_sub <= 0) {
		_commander_state_sub = orb_subscribe(ORB_ID(commander_state));
	}

	bool commander_state_updated;
	orb_check(_commander_state_sub, &commander_state_updated);

	if (commander_state_updated) {
		orb_copy(ORB_ID(commander_state), _commander_state_sub, &_commander_state);
	}
}
