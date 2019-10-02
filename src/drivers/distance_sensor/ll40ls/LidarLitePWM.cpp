/****************************************************************************
 *
 *   Copyright (c) 2014-2019 PX4 Development Team. All rights reserved.
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
 * @file LidarLitePWM.h
 * @author Johan Jansen <jnsn.johan@gmail.com>
 * @author Ban Siesta <bansiesta@gmail.com>
 *
 * Driver for the PulsedLight Lidar-Lite range finders connected via PWM.
 *
 * This driver accesses the pwm_input published by the pwm_input driver.
 */

#include "LidarLitePWM.h"

LidarLitePWM::LidarLitePWM(const uint8_t rotation) :
	LidarLite(rotation),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::hp_default)
{
}

LidarLitePWM::~LidarLitePWM()
{
	stop();
}

int
LidarLitePWM::init()
{
	start();

	return PX4_OK;
}

void
LidarLitePWM::start()
{
	ScheduleOnInterval(get_measure_interval());
}

void
LidarLitePWM::stop()
{
	ScheduleClear();
}

void
LidarLitePWM::Run()
{
	measure();
}

int
LidarLitePWM::measure()
{
	perf_begin(_sample_perf);

	const hrt_abstime timestamp_sample = hrt_absolute_time();

	if (PX4_OK != collect()) {
		PX4_DEBUG("collection error");
		perf_count(_comms_errors);
		perf_end(_sample_perf);
		return PX4_ERROR;
	}

	const float current_distance = float(_pwm.pulse_width) * 1e-3f;   /* 10 usec = 1 cm distance for LIDAR-Lite */

	/* Due to a bug in older versions of the LidarLite firmware, we have to reset sensor on (distance == 0) */
	if (current_distance <= 0.0f) {
		perf_count(_sensor_zero_resets);
		perf_end(_sample_perf);
		return reset_sensor();
	}

	_px4_rangefinder.update(timestamp_sample, current_distance);

	perf_end(_sample_perf);
	return PX4_OK;
}

int
LidarLitePWM::collect()
{
	int fd = ::open(PWMIN0_DEVICE_PATH, O_RDONLY);

	if (fd == -1) {
		return PX4_ERROR;
	}

	if (::read(fd, &_pwm, sizeof(_pwm)) == sizeof(_pwm)) {
		::close(fd);
		return PX4_OK;
	}

	::close(fd);
	return EAGAIN;
}
