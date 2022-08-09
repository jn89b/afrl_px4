/*
 *  UMKC FASTLAB
 *  - provides attitude controller frequency injection for the purpose of system identification
 */

#ifndef FLIGHTTESTINPUT_H
#define FLIGHTTESTINPUT_H

#include <mathlib/mathlib.h>
#include <stdint.h>

#include <controllib/block/BlockParam.hpp>
#include <systemlib/mavlink_log.h>
#include <uORB/topics/flight_test_input.h>
#include <uORB/topics/commander_state.h>
#include <uORB/topics/vehicle_status.h>

class __EXPORT FlightTestInput : public control::SuperBlock
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
	// _doublet_pulse_length(this, "PULSE_LEN"),
	// _doublet_pulse_amplitude(this, "PULSE_AMP"),
	float _time_running;
	float _raw_output;

	//fti_loop_gain 
	float _fti_set_loop_gain;
	float _loop_gain_default = 1;

	// frequency sweep sine sum
	float _sweep_sine_input;

	
	/** parameters **/
	control::BlockParamInt _mode;
	control::BlockParamInt _enable;
	control::BlockParamInt _injection_point;

	/** frequency sweep parameters **/
	control::BlockParamFloat _sweep_duration;
	control::BlockParamFloat _sweep_freq_begin;
	control::BlockParamFloat _sweep_freq_end;
	control::BlockParamFloat _sweep_freq_ramp;
	control::BlockParamFloat _sweep_amplitude_begin;
	control::BlockParamFloat _sweep_amplitude_end;
	control::BlockParamFloat _loop_gain;


	// /** doublet parameters **/
	// control::BlockParamFloat _doublet_pulse_length;
	// control::BlockParamFloat _doublet_pulse_amplitude;

	orb_advert_t _flight_test_input_pub;
	struct flight_test_input_s _flight_test_input;

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

#endif // FLIGHTTESTINPUT_H
