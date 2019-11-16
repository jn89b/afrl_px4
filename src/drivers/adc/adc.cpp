/****************************************************************************
 *
 *   Copyright (C) 2012-2019 PX4 Development Team. All rights reserved.
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
 * @file adc.cpp
 *
 * Driver for an ADC.
 *
 */
#include <stdint.h>
#include <drivers/drv_adc.h>
#include <drivers/drv_hrt.h>
#include <lib/cdev/CDev.hpp>
#include <lib/perf/perf_counter.h>
#include <px4_arch/adc.h>
#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>
#include <uORB/Publication.hpp>
#include <uORB/topics/adc_report.h>
#include <uORB/topics/system_power.h>


using namespace time_literals;

#ifndef ADC_CHANNELS
#error "board needs to define ADC_CHANNELS to use this driver"
#endif

#define ADC_TOTAL_CHANNELS 		32


class ADC : public cdev::CDev, public px4::ScheduledWorkItem
{
public:
	ADC(uint32_t base_address, uint32_t channels);
	~ADC();

	virtual int		init();

	virtual int		ioctl(file *filp, int cmd, unsigned long arg);
	virtual ssize_t		read(file *filp, char *buffer, size_t len);

protected:
	virtual int		open_first(struct file *filp);
	virtual int		close_last(struct file *filp);

private:
	void			Run() override;

	/**
	 * Sample a single channel and return the measured value.
	 *
	 * @param channel		The channel to sample.
	 * @return			The sampled value, or UINT32_MAX if sampling failed.
	 */
	uint32_t		sample(unsigned channel);

	void			update_adc_report(hrt_abstime now);
	void			update_system_power(hrt_abstime now);


	static const hrt_abstime	kINTERVAL{10_ms};	/**< 100Hz base rate */

	perf_counter_t			_sample_perf;

	unsigned			_channel_count{0};
	const uint32_t			_base_address;
	px4_adc_msg_t			*_samples{nullptr};	/**< sample buffer */

	uORB::Publication<adc_report_s>		_to_adc_report{ORB_ID(adc_report)};
	uORB::Publication<system_power_s>	_to_system_power{ORB_ID(system_power)};
};

ADC::ADC(uint32_t base_address, uint32_t channels) :
	CDev(ADC0_DEVICE_PATH),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::hp_default),
	_sample_perf(perf_alloc(PC_ELAPSED, "adc_samples")),
	_base_address(base_address)
{
	/* always enable the temperature sensor */
	channels |= px4_arch_adc_temp_sensor_mask();

	/* allocate the sample array */
	for (unsigned i = 0; i < ADC_TOTAL_CHANNELS; i++) {
		if (channels & (1 << i)) {
			_channel_count++;
		}
	}

	if (_channel_count > PX4_MAX_ADC_CHANNELS) {
		PX4_ERR("PX4_MAX_ADC_CHANNELS is too small:is %d needed:%d", PX4_MAX_ADC_CHANNELS, _channel_count);
	}

	_samples = new px4_adc_msg_t[_channel_count];

	/* prefill the channel numbers in the sample array */
	if (_samples != nullptr) {
		unsigned index = 0;

		for (unsigned i = 0; i < ADC_TOTAL_CHANNELS; i++) {
			if (channels & (1 << i)) {
				_samples[index].am_channel = i;
				_samples[index].am_data = 0;
				index++;
			}
		}
	}
}

ADC::~ADC()
{
	if (_samples != nullptr) {
		delete _samples;
	}

	perf_free(_sample_perf);
	px4_arch_adc_uninit(_base_address);
}

int
ADC::init()
{
	int rv = px4_arch_adc_init(_base_address);

	if (rv < 0) {
		PX4_DEBUG("sample timeout");
		return rv;
	}

	/* create the device node */
	return CDev::init();
}

int
ADC::ioctl(file *filp, int cmd, unsigned long arg)
{
	return -ENOTTY;
}

ssize_t
ADC::read(file *filp, char *buffer, size_t len)
{
	const size_t maxsize = sizeof(px4_adc_msg_t) * _channel_count;

	if (len > maxsize) {
		len = maxsize;
	}

	/* block interrupts while copying samples to avoid racing with an update */
	irqstate_t flags = px4_enter_critical_section();
	memcpy(buffer, _samples, len);
	px4_leave_critical_section(flags);

	return len;
}

int
ADC::open_first(struct file *filp)
{
	/* get fresh data */
	Run();

	/* and schedule regular updates */
	ScheduleOnInterval(kINTERVAL, kINTERVAL);

	return 0;
}

int
ADC::close_last(struct file *filp)
{
	ScheduleClear();

	return 0;
}

void
ADC::Run()
{
	hrt_abstime now = hrt_absolute_time();

	/* scan the channel set and sample each */
	for (unsigned i = 0; i < _channel_count; i++) {
		_samples[i].am_data = sample(_samples[i].am_channel);
	}

	update_adc_report(now);
	update_system_power(now);
}

void
ADC::update_adc_report(hrt_abstime now)
{
	adc_report_s adc = {};
	adc.timestamp = now;

	unsigned max_num = _channel_count;

	if (max_num > (sizeof(adc.channel_id) / sizeof(adc.channel_id[0]))) {
		max_num = (sizeof(adc.channel_id) / sizeof(adc.channel_id[0]));
	}

	for (unsigned i = 0; i < max_num; i++) {
		adc.channel_id[i] = _samples[i].am_channel;
		adc.channel_value[i] = _samples[i].am_data * 3.3f / px4_arch_adc_dn_fullcount();
		;
	}

	_to_adc_report.publish(adc);
}

void
ADC::update_system_power(hrt_abstime now)
{
#if defined (BOARD_ADC_USB_CONNECTED)
	system_power_s system_power {};
	system_power.timestamp = now;

	/* Assume HW provides only ADC_SCALED_V5_SENSE */
	int cnt = 1;
	/* HW provides both ADC_SCALED_V5_SENSE and ADC_SCALED_V3V3_SENSORS_SENSE */
#  if defined(ADC_SCALED_V5_SENSE) && defined(ADC_SCALED_V3V3_SENSORS_SENSE)
	cnt++;
#  endif

	for (unsigned i = 0; i < _channel_count; i++) {
#  if defined(ADC_SCALED_V5_SENSE)

		if (_samples[i].am_channel == ADC_SCALED_V5_SENSE) {
			// it is 2:1 scaled
			system_power.voltage5v_v = _samples[i].am_data * (ADC_V5_V_FULL_SCALE / px4_arch_adc_dn_fullcount());
			cnt--;

		} else
#  endif
#  if defined(ADC_SCALED_V3V3_SENSORS_SENSE)
		{
			if (_samples[i].am_channel == ADC_SCALED_V3V3_SENSORS_SENSE) {
				// it is 2:1 scaled
				system_power.voltage3v3_v = _samples[i].am_data * (ADC_3V3_SCALE * (3.3f / px4_arch_adc_dn_fullcount()));
				system_power.v3v3_valid = 1;
				cnt--;
			}
		}

#  endif

		if (cnt == 0) {
			break;
		}
	}

	/* Note once the board_config.h provides BOARD_ADC_USB_CONNECTED,
	 * It must provide the true logic GPIO BOARD_ADC_xxxx macros.
	 */
	// these are not ADC related, but it is convenient to
	// publish these to the same topic

	system_power.usb_connected = BOARD_ADC_USB_CONNECTED;
	/* If provided used the Valid signal from HW*/
#if defined(BOARD_ADC_USB_VALID)
	system_power.usb_valid = BOARD_ADC_USB_VALID;
#else
	/* If not provided then use connected */
	system_power.usb_valid  = system_power.usb_connected;
#endif

	/* The valid signals (HW dependent) are associated with each brick */
#if !defined(BOARD_NUMBER_DIGITAL_BRICKS)
	bool  valid_chan[BOARD_NUMBER_BRICKS] = BOARD_BRICK_VALID_LIST;
	system_power.brick_valid = 0;

	for (int b = 0; b < BOARD_NUMBER_BRICKS; b++) {
		system_power.brick_valid |=  valid_chan[b] ? 1 << b : 0;
	}

#endif

	system_power.servo_valid   = BOARD_ADC_SERVO_VALID;

#ifdef BOARD_ADC_PERIPH_5V_OC
	// OC pins are active low
	system_power.periph_5v_oc  = BOARD_ADC_PERIPH_5V_OC;
#endif

#ifdef BOARD_ADC_HIPOWER_5V_OC
	system_power.hipower_5v_oc = BOARD_ADC_HIPOWER_5V_OC;
#endif

	/* lazily publish */
	_to_system_power.publish(system_power);

#endif // BOARD_ADC_USB_CONNECTED
}

uint32_t
ADC::sample(unsigned channel)
{
	perf_begin(_sample_perf);
	uint32_t result = px4_arch_adc_sample(_base_address, channel);

	if (result == UINT32_MAX) {
		PX4_ERR("sample timeout");
	}

	perf_end(_sample_perf);
	return result;
}

/*
 * Driver 'main' command.
 */
extern "C" __EXPORT int adc_main(int argc, char *argv[]);

namespace
{
ADC	*g_adc{nullptr};

int
test(void)
{

	int fd = open(ADC0_DEVICE_PATH, O_RDONLY);

	if (fd < 0) {
		PX4_ERR("can't open ADC device %d", errno);
		return 1;
	}

	for (unsigned i = 0; i < 20; i++) {
		px4_adc_msg_t data[ADC_TOTAL_CHANNELS];
		ssize_t count = read(fd, data, sizeof(data));

		if (count < 0) {
			PX4_ERR("read error");
			return 1;
		}

		unsigned channels = count / sizeof(data[0]);

		for (unsigned j = 0; j < channels; j++) {
			printf("%d: %u  ", data[j].am_channel, data[j].am_data);
		}

		printf("\n");
		px4_usleep(500000);
	}

	return 0;
}
}

int
adc_main(int argc, char *argv[])
{
	if (g_adc == nullptr) {
		g_adc = new ADC(SYSTEM_ADC_BASE, ADC_CHANNELS);

		if (g_adc == nullptr) {
			PX4_ERR("couldn't allocate the ADC driver");
			return 1;
		}

		if (g_adc->init() != OK) {
			delete g_adc;
			PX4_ERR("ADC init failed");
			return 1;
		}
	}

	if (argc > 1) {
		if (!strcmp(argv[1], "test")) {
			return test();
		}
	}

	return 0;
}
