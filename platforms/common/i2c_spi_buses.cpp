/****************************************************************************
 *
 * Copyright (C) 2020 PX4 Development Team. All rights reserved.
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

#include <board_config.h>
#ifndef BOARD_DISABLE_I2C_SPI

#ifndef MODULE_NAME
#define MODULE_NAME "SPI_I2C"
#endif

#include <lib/drivers/device/Device.hpp>
#include <px4_platform_common/i2c_spi_buses.h>
#include <px4_platform_common/log.h>

BusInstanceIterator::BusInstanceIterator(I2CSPIInstance **instances, int max_num_instances,
		const BusCLIArguments &cli_arguments, uint16_t devid_driver_index)
	: _instances(instances), _max_num_instances(max_num_instances),
	  _bus_option(cli_arguments.bus_option),
	  _spi_bus_iterator(spiFilter(cli_arguments.bus_option),
			    cli_arguments.bus_option == I2CSPIBusOption::SPIExternal ? cli_arguments.chipselect_index : devid_driver_index,
			    cli_arguments.requested_bus),
	  _i2c_bus_iterator(i2cFilter(cli_arguments.bus_option), cli_arguments.requested_bus)
{
}

bool BusInstanceIterator::next()
{
	int bus = -1;

	if (busType() == BOARD_INVALID_BUS) {
		while (++_current_instance < _max_num_instances && _instances[_current_instance] == nullptr) {}

		return _current_instance < _max_num_instances;

	} else if (busType() == BOARD_SPI_BUS) {
		if (_spi_bus_iterator.next()) {
			bus = _spi_bus_iterator.bus().bus;
		}

	} else {
		if (_i2c_bus_iterator.next()) {
			bus = _i2c_bus_iterator.bus().bus;
		}
	}

	if (bus != -1) {
		// find matching runtime instance
		_current_instance = -1;

		for (int i = 0; i < _max_num_instances; ++i) {
			if (!_instances[i]) {
				continue;
			}

			if (_bus_option == _instances[i]->_bus_option && bus == _instances[i]->_bus) {
				_current_instance = i;
			}
		}

		return true;
	}

	return false;
}

int BusInstanceIterator::nextFreeInstance() const
{
	for (int i = 0; i < _max_num_instances; ++i) {
		if (_instances[i] == nullptr) {
			return i;
		}
	}

	return -1;
}

I2CSPIInstance *BusInstanceIterator::instance() const
{
	if (_current_instance < 0 || _current_instance >= _max_num_instances) {
		return nullptr;
	}

	return _instances[_current_instance];
}

void BusInstanceIterator::resetInstance()
{
	if (_current_instance >= 0 && _current_instance < _max_num_instances) {
		_instances[_current_instance] = nullptr;
	}
}

board_bus_types BusInstanceIterator::busType() const
{
	switch (_bus_option) {
	case I2CSPIBusOption::All:
		return BOARD_INVALID_BUS;

	case I2CSPIBusOption::I2CInternal:
	case I2CSPIBusOption::I2CExternal:
		return BOARD_I2C_BUS;

	case I2CSPIBusOption::SPIInternal:
	case I2CSPIBusOption::SPIExternal:
		return BOARD_SPI_BUS;
	}

	return BOARD_INVALID_BUS;
}

int BusInstanceIterator::bus() const
{
	if (busType() == BOARD_INVALID_BUS) {
		return -1;

	} else if (busType() == BOARD_SPI_BUS) {
		return _spi_bus_iterator.bus().bus;

	} else {
		return _i2c_bus_iterator.bus().bus;
	}
}

uint32_t BusInstanceIterator::devid() const
{
	if (busType() == BOARD_INVALID_BUS) {
		return 0;

	} else if (busType() == BOARD_SPI_BUS) {
		return _spi_bus_iterator.devid();

	} else {
		return 0;
	}
}

uint32_t BusInstanceIterator::DRDYGPIO() const
{
	if (busType() == BOARD_INVALID_BUS) {
		return 0;

	} else if (busType() == BOARD_SPI_BUS) {
		return _spi_bus_iterator.DRDYGPIO();

	} else {
		return 0;
	}
}

bool BusInstanceIterator::external() const
{
	if (busType() == BOARD_INVALID_BUS) {
		return false;

	} else if (busType() == BOARD_SPI_BUS) {
		return _spi_bus_iterator.external();

	} else {
		return _i2c_bus_iterator.external();
	}
}

I2CBusIterator::FilterType BusInstanceIterator::i2cFilter(I2CSPIBusOption bus_option)
{
	switch (bus_option) {
	case I2CSPIBusOption::All: return I2CBusIterator::FilterType::All;

	case I2CSPIBusOption::I2CExternal: return I2CBusIterator::FilterType::ExternalBus;

	case I2CSPIBusOption::I2CInternal: return I2CBusIterator::FilterType::InternalBus;

	default: break;
	}

	return I2CBusIterator::FilterType::All;
}

SPIBusIterator::FilterType BusInstanceIterator::spiFilter(I2CSPIBusOption bus_option)
{
	switch (bus_option) {
	case I2CSPIBusOption::SPIExternal: return SPIBusIterator::FilterType::ExternalBus;

	case I2CSPIBusOption::SPIInternal: return SPIBusIterator::FilterType::InternalBus;

	default: break;
	}

	return SPIBusIterator::FilterType::InternalBus;
}


int I2CSPIDriverBase::module_start(const BusCLIArguments &cli, BusInstanceIterator &iterator,
				   void(*print_usage)(),
				   instantiate_method instantiate, I2CSPIInstance **instances)
{
	if (iterator.configuredBusOption() == I2CSPIBusOption::All) {
		PX4_ERR("need to specify a bus type");
		print_usage();
		return -1;
	}

	bool started = false;

	while (iterator.next()) {
		if (iterator.instance()) {
			continue; // already running
		}

		const int free_index = iterator.nextFreeInstance();

		if (free_index < 0) {
			PX4_ERR("Not enough instances");
			return -1;
		}

		device::Device::DeviceId device_id{};
		device_id.devid_s.bus = iterator.bus();

		switch (iterator.busType()) {
		case BOARD_I2C_BUS: device_id.devid_s.bus_type = device::Device::DeviceBusType_I2C; break;

		case BOARD_SPI_BUS: device_id.devid_s.bus_type = device::Device::DeviceBusType_SPI; break;

		case BOARD_INVALID_BUS: device_id.devid_s.bus_type = device::Device::DeviceBusType_UNKNOWN; break;
		}

		// initialize the object and bus on the work queue thread - this will also probe for the device
		I2CSPIDriverInitializer initializer(px4::device_bus_to_wq(device_id.devid), cli, iterator, instantiate, free_index);
		initializer.ScheduleNow();
		initializer.wait();
		I2CSPIDriverBase *instance = initializer.instance();

		if (!instance) {
			PX4_DEBUG("instantiate failed (no device on bus %i (devid 0x%x)?)", iterator.bus(), iterator.devid());
			continue;
		}

		instances[free_index] = instance;
		started = true;

		// print some info that we are running
		switch (iterator.busType()) {
		case BOARD_I2C_BUS:
			PX4_INFO_RAW("%s #%i on I2C bus %d%s\n",
				     instance->ItemName(), free_index, iterator.bus(), iterator.external() ? " (external)" : "");
			break;

		case BOARD_SPI_BUS:
			PX4_INFO_RAW("%s #%i on SPI bus %d (devid=0x%x)%s\n",
				     instance->ItemName(), free_index, iterator.bus(), PX4_SPI_DEV_ID(iterator.devid()),
				     iterator.external() ? " (external)" : "");
			break;

		case BOARD_INVALID_BUS:
			break;
		}
	}

	return started ? 0 : -1;
}


int I2CSPIDriverBase::module_stop(BusInstanceIterator &iterator)
{
	bool is_running = false;

	while (iterator.next()) {
		if (iterator.instance()) {
			I2CSPIDriverBase *instance = (I2CSPIDriverBase *)iterator.instance();
			instance->request_stop_and_wait();
			delete iterator.instance();
			iterator.resetInstance();
			is_running = true;
		}
	}

	if (!is_running) {
		PX4_ERR("Not running");
		return -1;
	}

	return 0;
}

int I2CSPIDriverBase::module_status(BusInstanceIterator &iterator)
{
	bool is_running = false;

	while (iterator.next()) {
		if (iterator.instance()) {
			I2CSPIDriverBase *instance = (I2CSPIDriverBase *)iterator.instance();
			instance->print_status();
			is_running = true;
		}
	}

	if (!is_running) {
		PX4_INFO("Not running");
		return -1;
	}

	return 0;
}

int I2CSPIDriverBase::module_custom_method(const BusCLIArguments &cli, BusInstanceIterator &iterator)
{
	while (iterator.next()) {
		if (iterator.instance()) {
			I2CSPIDriverBase *instance = (I2CSPIDriverBase *)iterator.instance();
			instance->custom_method(cli);
		}
	}

	return 0;
}

void I2CSPIDriverBase::print_status()
{
	bool is_i2c_bus = _bus_option == I2CSPIBusOption::I2CExternal || _bus_option == I2CSPIBusOption::I2CInternal;
	PX4_INFO("Running on %s Bus %i", is_i2c_bus ? "I2C" : "SPI", _bus);
}

void I2CSPIDriverBase::request_stop_and_wait()
{
	_task_should_exit.store(true);
	ScheduleNow(); // wake up the task (in case it is not scheduled anymore or just to be faster)
	unsigned int i = 0;

	do {
		px4_usleep(20000); // 20 ms
		// wait at most 2 sec
	} while (++i < 100 && !_task_exited.load());

	if (i >= 100) {
		PX4_ERR("Module did not respond to stop request");
	}
}

I2CSPIDriverInitializer::I2CSPIDriverInitializer(const px4::wq_config_t &config, const BusCLIArguments &cli,
		const BusInstanceIterator &iterator, instantiate_method instantiate, int runtime_instance)
	: px4::WorkItem("<driver_init>", config),
	  _cli(cli), _iterator(iterator), _runtime_instance(runtime_instance), _instantiate(instantiate)
{
	px4_sem_init(&_sem, 0, 0);
}

I2CSPIDriverInitializer::~I2CSPIDriverInitializer()
{
	px4_sem_destroy(&_sem);
}

void I2CSPIDriverInitializer::wait()
{
	while (px4_sem_wait(&_sem) != 0) {}
}

void I2CSPIDriverInitializer::Run()
{
	_instance = _instantiate(_cli, _iterator, _runtime_instance);
	px4_sem_post(&_sem);
}

#endif /* BOARD_DISABLE_I2C_SPI */
