/****************************************************************************
 *
 *   Copyright (c) 2012-2016 PX4 Development Team. All rights reserved.
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
 * @file main.cpp
 *
 * Driver for the Invensense mpu9250 connected via I2C or SPI.
 *
 * @authors Andrew Tridgell
 *          Robert Dickenson
 *
 * based on the mpu6000 driver
 */

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/getopt.h>

#include "mpu9250.h"

/** driver 'main' command */
extern "C" { __EXPORT int mpu9250_main(int argc, char *argv[]); }

enum MPU9250_BUS {
	MPU9250_BUS_ALL = 0,
	MPU9250_BUS_I2C_INTERNAL,
	MPU9250_BUS_I2C_EXTERNAL,
	MPU9250_BUS_SPI_INTERNAL,
	MPU9250_BUS_SPI_INTERNAL2,
	MPU9250_BUS_SPI_EXTERNAL
};

/**
 * Local functions in support of the shell command.
 */
namespace mpu9250
{

// list of supported bus configurations
struct mpu9250_bus_option {
	enum MPU9250_BUS busid;
	MPU9250_constructor interface_constructor;
	bool magpassthrough;
	uint8_t busnum;
	uint32_t address;
	MPU9250	*dev;
} bus_options[] = {
#if defined(USE_I2C)
#  if defined(PX4_I2C_BUS_ONBOARD) && defined(PX4_I2C_OBDEV_MPU9250)
	{ MPU9250_BUS_I2C_INTERNAL, &MPU9250_I2C_interface, false, PX4_I2C_BUS_ONBOARD, PX4_I2C_OBDEV_MPU9250, nullptr },
#  endif
#  if defined(PX4_I2C_BUS_EXPANSION) && defined(PX4_I2C_OBDEV_MPU9250)
	{ MPU9250_BUS_I2C_EXTERNAL, &MPU9250_I2C_interface, false, PX4_I2C_BUS_EXPANSION, PX4_I2C_OBDEV_MPU9250, nullptr },
#  endif
#  if defined(PX4_I2C_BUS_EXPANSION1) && defined(PX4_I2C_OBDEV_MPU9250)
	{ MPU9250_BUS_I2C_EXTERNAL, &MPU9250_I2C_interface, false, PX4_I2C_BUS_EXPANSION1, PX4_I2C_OBDEV_MPU9250, nullptr },
#  endif
#  if defined(PX4_I2C_BUS_EXPANSION2) && defined(PX4_I2C_OBDEV_MPU9250)
	{ MPU9250_BUS_I2C_EXTERNAL, &MPU9250_I2C_interface, false, PX4_I2C_BUS_EXPANSION2, PX4_I2C_OBDEV_MPU9250, nullptr },
#  endif
#endif
#if defined(PX4_SPI_BUS_SENSORS) && defined(PX4_SPIDEV_MPU)
	{ MPU9250_BUS_SPI_INTERNAL, &MPU9250_SPI_interface, true, PX4_SPI_BUS_SENSORS, PX4_SPIDEV_MPU, nullptr },
#endif
#if defined(PX4_SPI_BUS_SENSORS) && defined(PX4_SPIDEV_MPU2)
	{ MPU9250_BUS_SPI_INTERNAL2, &MPU9250_SPI_interface, true, PX4_SPI_BUS_SENSORS, PX4_SPIDEV_MPU2, nullptr },
#endif
#if defined(PX4_SPI_BUS_EXT) && defined(PX4_SPIDEV_EXT_MPU)
	{ MPU9250_BUS_SPI_EXTERNAL, &MPU9250_SPI_interface, true, PX4_SPI_BUS_EXT, PX4_SPIDEV_EXT_MPU, nullptr },
#endif
};

#define NUM_BUS_OPTIONS (sizeof(bus_options)/sizeof(bus_options[0]))


bool	start_bus(struct mpu9250_bus_option &bus, enum Rotation rotation);
mpu9250_bus_option *find_bus(enum MPU9250_BUS busid);

int	start(enum MPU9250_BUS busid, enum Rotation rotation);
int	stop(enum MPU9250_BUS busid);
int	info(enum MPU9250_BUS busid);
int	usage();

/**
 * find a bus structure for a busid
 */
struct mpu9250_bus_option *find_bus(enum MPU9250_BUS busid)
{
	for (uint8_t i = 0; i < NUM_BUS_OPTIONS; i++) {
		if ((busid == MPU9250_BUS_ALL ||
		     busid == bus_options[i].busid) && bus_options[i].dev != nullptr) {
			return &bus_options[i];
		}
	}

	PX4_ERR("bus %u not started", (unsigned)busid);
	return nullptr;
}

/**
 * start driver for a specific bus option
 */
bool
start_bus(mpu9250_bus_option &bus, enum Rotation rotation)
{
	PX4_INFO("Bus probed: %d", bus.busid);

	if (bus.dev != nullptr) {
		PX4_ERR("SPI %d not available", bus.busid);
		return false;
	}

	device::Device *interface = bus.interface_constructor(bus.busnum, bus.address);

	if (interface == nullptr) {
		PX4_WARN("no device on bus %u", (unsigned)bus.busid);
		return false;
	}

	if (interface->init() != OK) {
		delete interface;
		PX4_WARN("no device on bus %u", (unsigned)bus.busid);
		return false;
	}

	device::Device *mag_interface = nullptr;

#ifdef USE_I2C
	/* For i2c interfaces, connect to the magnetomer directly */
	const bool is_i2c = bus.busid == MPU9250_BUS_I2C_INTERNAL || bus.busid == MPU9250_BUS_I2C_EXTERNAL;

	if (is_i2c) {
		mag_interface = AK8963_I2C_interface(bus.busnum);
	}

#endif

	bus.dev = new MPU9250(interface, mag_interface, rotation);

	if (bus.dev == nullptr) {
		delete interface;

		if (mag_interface != nullptr) {
			delete mag_interface;
		}

		return false;
	}

	if (OK != bus.dev->init()) {
		goto fail;
	}

	return true;

fail:

	if (bus.dev != nullptr) {
		delete (bus.dev);
		bus.dev = nullptr;
	}

	PX4_ERR("driver start failed");

	return false;
}

/**
 * Start the driver.
 *
 * This function only returns if the driver is up and running
 * or failed to detect the sensor.
 */
int
start(enum MPU9250_BUS busid, enum Rotation rotation)
{
	bool started = false;

	for (unsigned i = 0; i < NUM_BUS_OPTIONS; i++) {
		if (bus_options[i].dev != nullptr) {
			// this device is already started
			continue;
		}

		if (busid != MPU9250_BUS_ALL && bus_options[i].busid != busid) {
			// not the one that is asked for
			continue;
		}

		started |= start_bus(bus_options[i], rotation);

		if (started) { break; }
	}

	return PX4_OK;
}

int
stop(enum MPU9250_BUS busid)
{
	mpu9250_bus_option *bus = find_bus(busid);

	if (bus != nullptr && bus->dev != nullptr) {
		delete bus->dev;
		bus->dev = nullptr;

	} else {
		/* warn, but not an error */
		PX4_WARN("already stopped.");
	}

	return PX4_OK;
}

/**
 * Print a little info about the driver.
 */
int
info(enum MPU9250_BUS busid)
{
	mpu9250_bus_option *bus = find_bus(busid);

	if (bus != nullptr && bus->dev != nullptr) {
		PX4_WARN("driver not running");
		return PX4_ERROR;
	}

	bus->dev->print_info();

	return PX4_OK;
}

int
usage()
{
	PX4_INFO("missing command: try 'start', 'stop', 'info'");
	PX4_INFO("options:");
	PX4_INFO("    -X    (i2c external bus)");
	PX4_INFO("    -I    (i2c internal bus)");
	PX4_INFO("    -s    (spi internal bus)");
	PX4_INFO("    -S    (spi external bus)");
	PX4_INFO("    -t    (spi internal bus, 2nd instance)");
	PX4_INFO("    -R rotation");

	return PX4_OK;
}

} // namespace

int
mpu9250_main(int argc, char *argv[])
{
	int myoptind = 1;
	int ch;
	const char *myoptarg = nullptr;

	enum MPU9250_BUS busid = MPU9250_BUS_ALL;
	enum Rotation rotation = ROTATION_NONE;

	while ((ch = px4_getopt(argc, argv, "XISstMR:", &myoptind, &myoptarg)) != EOF) {
		switch (ch) {
		case 'X':
			busid = MPU9250_BUS_I2C_EXTERNAL;
			break;

		case 'I':
			busid = MPU9250_BUS_I2C_INTERNAL;
			break;

		case 'S':
			busid = MPU9250_BUS_SPI_EXTERNAL;
			break;

		case 's':
			busid = MPU9250_BUS_SPI_INTERNAL;
			break;

		case 't':
			busid = MPU9250_BUS_SPI_INTERNAL2;
			break;

		case 'R':
			rotation = (enum Rotation)atoi(myoptarg);
			break;

		default:
			return mpu9250::usage();
		}
	}

	if (myoptind >= argc) {
		return mpu9250::usage();
	}

	const char *verb = argv[myoptind];

	/*
	 * Start/load the driver.
	 */
	if (!strcmp(verb, "start")) {
		return mpu9250::start(busid, rotation);
	}

	if (!strcmp(verb, "stop")) {
		return mpu9250::stop(busid);
	}

	/*
	 * Print driver information.
	 */
	if (!strcmp(verb, "info")) {
		return mpu9250::info(busid);
	}

	return mpu9250::usage();
}
