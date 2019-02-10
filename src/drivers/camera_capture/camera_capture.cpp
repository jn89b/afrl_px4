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
 * @file camera_capture.cpp
 *
 * Online and offline geotagging from camera feedback
 *
 * @author Mohammed Kabir <kabir@uasys.io>
 */

#include "camera_capture.hpp"

#define commandParamToInt(n) static_cast<int>(n >= 0 ? n + 0.5f : n - 0.5f)

namespace camera_capture
{
CameraCapture	*g_camera_capture;
}

CameraCapture::CameraCapture() :
	_capture_enabled(false),
	_trigger_pub(nullptr),
	_command_ack_pub(nullptr),
	_command_sub(-1),
	_capture_seq(0),
	_last_fall_time(0),
	_last_exposure_time(0),
	_capture_overflows(0)
{

	memset(&_work, 0, sizeof(_work));

	// Parameters
	_p_strobe_delay = param_find("CAM_CAP_DELAY");
	param_get(_p_strobe_delay, &_strobe_delay);

	struct camera_trigger_s trigger = {};
	_trigger_pub = orb_advertise(ORB_ID(camera_trigger), &trigger);
}

CameraCapture::~CameraCapture()
{
	camera_capture::g_camera_capture = nullptr;
}

void
CameraCapture::capture_callback(uint32_t chan_index,
				hrt_abstime edge_time, uint32_t edge_state, uint32_t overflow)
{

	if (edge_state == 0) {											// Falling edge
		// Timestamp and compensate for strobe delay
		_last_fall_time = edge_time - uint64_t(1000 * _strobe_delay);

	} else if (edge_state == 1 && _last_fall_time > 0) {			// Falling edge and got rising before
		struct camera_trigger_s	trigger {};

		trigger.timestamp = edge_time - ((edge_time - _last_fall_time) / 2);	// Get timestamp of mid-exposure
		trigger.seq = _capture_seq++;

		orb_publish(ORB_ID(camera_trigger), _trigger_pub, &trigger);

		_last_exposure_time = edge_time - _last_fall_time;
	}

	_capture_overflows = overflow;

}

void
CameraCapture::capture_trampoline(void *context, uint32_t chan_index,
				  hrt_abstime edge_time, uint32_t edge_state, uint32_t overflow)
{
	camera_capture::g_camera_capture->capture_callback(chan_index, edge_time, edge_state, overflow);
}

void
CameraCapture::cycle_trampoline(void *arg)
{

	CameraCapture *cap = reinterpret_cast<CameraCapture *>(arg);

	if (cap->_command_sub < 0) {
		cap->_command_sub = orb_subscribe(ORB_ID(vehicle_command));
	}

	bool updated = false;
	orb_check(cap->_command_sub, &updated);

	// Command handling
	if (updated) {

		vehicle_command_s cmd;
		orb_copy(ORB_ID(vehicle_command), cap->_command_sub, &cmd);

		// TODO : this should eventuallly be a capture control command
		if (cmd.command == vehicle_command_s::VEHICLE_CMD_DO_TRIGGER_CONTROL) {

			// Enable/disable signal capture
			if (commandParamToInt(cmd.param1) == 1) {
				cap->set_capture_control(true);

			} else if (commandParamToInt(cmd.param1) == 0) {
				cap->set_capture_control(false);

			}

			// Reset capture sequence
			if (commandParamToInt(cmd.param2) == 1) {
				cap->reset_statistics(true);

			}

			// Acknowledge the command
			vehicle_command_ack_s command_ack = {
				.timestamp = 0,
				.result_param2 = 0,
				.command = cmd.command,
				.result = (uint8_t)vehicle_command_s::VEHICLE_CMD_RESULT_ACCEPTED,
				.from_external = false,
				.result_param1 = 0,
				.target_system = cmd.source_system,
				.target_component = cmd.source_component
			};

			if (cap->_command_ack_pub == nullptr) {
				cap->_command_ack_pub = orb_advertise_queue(ORB_ID(vehicle_command_ack), &command_ack,
							vehicle_command_ack_s::ORB_QUEUE_LENGTH);

			} else {
				orb_publish(ORB_ID(vehicle_command_ack), cap->_command_ack_pub, &command_ack);

			}
		}

	}

	work_queue(LPWORK, &_work, (worker_t)&CameraCapture::cycle_trampoline, camera_capture::g_camera_capture,
		   USEC2TICK(100000)); // 100ms
}

void
CameraCapture::set_capture_control(bool enabled)
{
	if (enabled) {
		// register callbacks
		//up_input_capture_set(4, Both, 0, &CameraCapture::capture_trampoline, this);
		up_input_capture_set(5, Both, 0, &CameraCapture::capture_trampoline, this);
		_capture_enabled = true;

	} else {
		//up_input_capture_set(4, Disabled, 0, NULL, NULL);
		up_input_capture_set(5, Disabled, 0, NULL, NULL);
		_capture_enabled = false;
	}

	reset_statistics(false);
}

void
CameraCapture::reset_statistics(bool reset_seq)
{
	if (reset_seq) { _capture_seq = 0; }

	_last_fall_time = 0;
	_last_exposure_time = 0;
	_capture_overflows = 0;
}

void
CameraCapture::start()
{
	// start to monitor at low rates for capture control commands
	work_queue(LPWORK, &_work, (worker_t)&CameraCapture::cycle_trampoline, this, USEC2TICK(1)); // TODO : is this low rate??!
}

void
CameraCapture::stop()
{

	work_cancel(LPWORK, &_work);

	if (camera_capture::g_camera_capture != nullptr) {
		delete (camera_capture::g_camera_capture);
	}
}

void
CameraCapture::status()
{
	PX4_INFO("Capture enabled : %s", _capture_enabled ? "YES" : "NO");
	PX4_INFO("Frame sequence : %u", _capture_seq);
	PX4_INFO("Last fall timestamp : %llu", _last_fall_time);
	PX4_INFO("Last exposure time : %0.2f ms", double(_last_exposure_time) / 1000.0);
	PX4_INFO("Number of overflows : %u", _capture_overflows);
}

static int usage()
{
	PX4_INFO("usage: camera_capture {start|stop|on|off|reset}\n");
	return 1;
}

extern "C" __EXPORT int camera_capture_main(int argc, char *argv[]);

int camera_capture_main(int argc, char *argv[])
{
	if (argc < 2) {
		return usage();
	}

	if (!strcmp(argv[1], "start")) {

		if (camera_capture::g_camera_capture != nullptr) {
			PX4_WARN("already running");
			return 0;
		}

		camera_capture::g_camera_capture = new CameraCapture();

		if (camera_capture::g_camera_capture == nullptr) {
			PX4_WARN("alloc failed");
			return 1;
		}

		camera_capture::g_camera_capture->start();
		return 0;
	}

	if (camera_capture::g_camera_capture == nullptr) {
		PX4_WARN("not running");
		return 1;

	} else if (!strcmp(argv[1], "stop")) {
		camera_capture::g_camera_capture->stop();

	} else if (!strcmp(argv[1], "status")) {
		camera_capture::g_camera_capture->status();

	} else if (!strcmp(argv[1], "on")) {
		camera_capture::g_camera_capture->set_capture_control(true);

	} else if (!strcmp(argv[1], "off")) {
		camera_capture::g_camera_capture->set_capture_control(false);

	} else if (!strcmp(argv[1], "reset")) {
		camera_capture::g_camera_capture->set_capture_control(false);
		camera_capture::g_camera_capture->reset_statistics(true);

	} else {
		return usage();
	}

	return 0;
}
