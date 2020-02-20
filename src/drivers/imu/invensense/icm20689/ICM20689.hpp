/****************************************************************************
 *
 *   Copyright (c) 2020 PX4 Development Team. All rights reserved.
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
 * @file ICM20689.hpp
 *
 * Driver for the Invensense ICM20689 connected via SPI.
 *
 */

#pragma once

#include "InvenSense_ICM20689_registers.hpp"

#include <drivers/drv_hrt.h>
#include <lib/drivers/accelerometer/PX4Accelerometer.hpp>
#include <lib/drivers/device/spi.h>
#include <lib/drivers/gyroscope/PX4Gyroscope.hpp>
#include <lib/ecl/geo/geo.h>
#include <lib/perf/perf_counter.h>
#include <px4_platform_common/atomic.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>

using namespace InvenSense_ICM20689;

class ICM20689 : public device::SPI, public px4::ScheduledWorkItem
{
public:
	ICM20689(int bus, uint32_t device, enum Rotation rotation = ROTATION_NONE);
	~ICM20689() override;

	bool Init();
	void Start();
	void Stop();
	bool Reset();
	void PrintInfo();

private:

	struct register_config_t {
		Register reg;
		uint8_t set_bits{0};
		uint8_t clear_bits{0};
	};

	int probe() override;

	static int DataReadyInterruptCallback(int irq, void *context, void *arg);
	void DataReady();

	void Run() override;

	bool CheckRegister(const register_config_t &reg_cfg, bool notify = true);
	bool Configure(bool notify = true);
	void ConfigureAccel();
	void ConfigureGyro();
	void ConfigureSampleRate(int sample_rate);

	uint8_t RegisterRead(Register reg);
	void RegisterClearBits(Register reg, uint8_t clearbits);
	void RegisterSetAndClearBits(Register reg, uint8_t setbits, uint8_t clearbits);
	void RegisterSetBits(Register reg, uint8_t setbits);
	void RegisterWrite(Register reg, uint8_t value);

	void ResetFIFO();

	uint8_t *_dma_data_buffer{nullptr};

	PX4Accelerometer _px4_accel;
	PX4Gyroscope _px4_gyro;

	perf_counter_t _transfer_perf{perf_alloc(PC_ELAPSED, MODULE_NAME": transfer")};
	perf_counter_t _bad_register_perf{perf_alloc(PC_COUNT, MODULE_NAME": bad register")};
	perf_counter_t _bad_transfer_perf{perf_alloc(PC_COUNT, MODULE_NAME": bad transfer")};
	perf_counter_t _fifo_empty_perf{perf_alloc(PC_COUNT, MODULE_NAME": fifo empty")};
	perf_counter_t _fifo_overflow_perf{perf_alloc(PC_COUNT, MODULE_NAME": fifo overflow")};
	perf_counter_t _fifo_reset_perf{perf_alloc(PC_COUNT, MODULE_NAME": fifo reset")};
	perf_counter_t _drdy_interval_perf{perf_alloc(PC_INTERVAL, MODULE_NAME": drdy interval")};

	hrt_abstime _last_config_check{0};
	hrt_abstime _time_last_temperature_update{0};

	px4::atomic<int> _data_ready_count{0};

	uint8_t _checked_register{0};

	bool _using_data_ready_interrupt_enabled{false};

	// Sensor Configuration
	static constexpr uint32_t GYRO_RATE{8000};  // 8 kHz gyro
	static constexpr uint32_t ACCEL_RATE{4000}; // 4 kHz accel
	static constexpr uint32_t FIFO_MAX_SAMPLES{ math::min(FIFO::SIZE / sizeof(FIFO::DATA) + 1, sizeof(PX4Gyroscope::FIFOSample::x) / sizeof(PX4Gyroscope::FIFOSample::x[0]))};

	uint16_t _fifo_empty_interval_us{1000}; // 1000 us / 1000 Hz transfer interval
	uint8_t _fifo_gyro_samples{static_cast<uint8_t>(_fifo_empty_interval_us / (1000000 / GYRO_RATE))};
	uint8_t _fifo_accel_samples{static_cast<uint8_t>(_fifo_empty_interval_us / (1000000 / ACCEL_RATE))};

	static constexpr uint8_t size_register_cfg{11};
	register_config_t _register_cfg[size_register_cfg] {
		// Register               | Set bits, Clear bits
		{ Register::PWR_MGMT_1,    PWR_MGMT_1_BIT::CLKSEL_0, PWR_MGMT_1_BIT::DEVICE_RESET | PWR_MGMT_1_BIT::SLEEP },
		{ Register::ACCEL_CONFIG,  ACCEL_CONFIG_BIT::ACCEL_FS_SEL_16G, 0 },
		{ Register::ACCEL_CONFIG2, ACCEL_CONFIG2_BIT::ACCEL_FCHOICE_B, ACCEL_CONFIG2_BIT::FIFO_SIZE },
		{ Register::GYRO_CONFIG,   GYRO_CONFIG_BIT::FS_SEL_2000_DPS, GYRO_CONFIG_BIT::FCHOICE_B_8KHZ_BYPASS_DLPF },
		{ Register::CONFIG,        CONFIG_BIT::DLPF_CFG_BYPASS_DLPF_8KHZ, Bit7 | CONFIG_BIT::FIFO_MODE },
		{ Register::USER_CTRL,     USER_CTRL_BIT::FIFO_EN | USER_CTRL_BIT::I2C_IF_DIS, 0 },
		{ Register::FIFO_EN,       FIFO_EN_BIT::XG_FIFO_EN | FIFO_EN_BIT::YG_FIFO_EN | FIFO_EN_BIT::ZG_FIFO_EN | FIFO_EN_BIT::ACCEL_FIFO_EN, FIFO_EN_BIT::TEMP_FIFO_EN },
		{ Register::INT_ENABLE,    INT_ENABLE_BIT::FIFO_OFLOW_EN | INT_ENABLE_BIT::DATA_RDY_INT_EN }
	};
};
