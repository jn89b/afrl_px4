/****************************************************************************
 *
 *   Copyright (c) 2016-2018 PX4 Development Team. All rights reserved.
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

#include <stdlib.h>
#include <stdbool.h>

#include <drivers/device/device.h>
#include <drivers/drv_hrt.h>

#include <uORB/uORB.h>
#include <uORB/topics/telemetry_status.h>

typedef enum {
	SATCOM_OK = 0,
	SATCOM_NO_MSG = -1,
	SATCOM_ERROR = -255,
} satcom_status;

typedef enum {
	SATCOM_UART_OK = 0,
	SATCOM_UART_OPEN_FAIL = -1,
} satcom_uart_status;

typedef enum {
	SATCOM_READ_OK = 0,
	SATCOM_READ_TIMEOUT = -1,
	SATCOM_READ_PARSING_FAIL = -2,
} satcom_read_status;

typedef enum {
	SATCOM_RESULT_OK,
	SATCOM_RESULT_ERROR,
	SATCOM_RESULT_SBDRING,
	SATCOM_RESULT_READY,
	SATCOM_RESULT_HWFAIL,
	SATCOM_RESULT_NA,
} satcom_result_code;

//typedef struct
//{
//	uint8_t	info;
//	uint8_t	result_code;
//} satcom_at_msg;

typedef enum {
	SATCOM_STATE_STANDBY,
	SATCOM_STATE_CSQ,
	SATCOM_STATE_SBDSESSION,
	SATCOM_STATE_TEST,
} satcom_state;

extern "C" __EXPORT int iridiumsbd_main(int argc, char *argv[]);

#define SATCOM_TX_BUF_LEN			340		// TX buffer size - maximum for a SBD MO message is 340, but billed per 50
#define SATCOM_MAX_MESSAGE_LENGTH		50		// Maximum length of the expected messages sent over this link
#define SATCOM_RX_MSG_BUF_LEN			270		// RX buffer size for MT messages
#define SATCOM_RX_COMMAND_BUF_LEN		50		// RX buffer size for other commands
#define SATCOM_SIGNAL_REFRESH_DELAY		20000000 // update signal quality every 20s
#define MAVLINK_PACKAGE_START		254 // The value of the first byte of the mavlink header


/**
 * The driver for the Rockblock 9602 and 9603 RockBlock module for satellite communication over the Iridium satellite system
 *
 * TODO:
 * 	- Improve TX buffer handling:
 * 		- Do not reset the full TX buffer but delete the oldest HIGH_LATENCY2 message if one is in the buffer or delete the oldest message in general
 * 	- Keep CDev active even if the driver is stopped to avoid a hard fault caused by MavLink.
 */
class IridiumSBD : public device::CDev
{
public:
	/*
	 * Constructor
	 */
	IridiumSBD();

	/*
	 * Start the driver
	 */
	static int start(int argc, char *argv[]);

	/*
	 * Stop the driver
	 */
	static int stop();

	/*
	 * Display driver status
	 */
	static void status();

	/*
	 * Run a driver test based on the input
	 *  - `s`: Send a test string
	 *  - `read`: Start a sbd read session
	 *  - else: Is assumed to be a valid AT command and written to the modem
	 */
	static void test(int argc, char *argv[]);

	/*
	 * Passes everything to CDev
	 */
	int ioctl(struct file *filp, int cmd, unsigned long arg);

private:
	/*
	 * Entry point of the task, has to be a static function
	 */
	static void main_loop_helper(int argc, char *argv[]);

	/*
	 * Main driver loop
	 */
	void main_loop(int argc, char *argv[]);

	/*
	 * Loop executed while in SATCOM_STATE_STANDBY
	 *
	 * Changes to SATCOM_STATE_TEST, SATCOM_STATE_SBDSESSION if required.
	 * Periodically changes to SATCOM_STATE_CSQ for a signal quality check.
	 */
	void standby_loop(void);

	/*
	 * Loop executed while in SATCOM_STATE_CSQ
	 *
	 * Changes to SATCOM_STATE_STANDBY after finished signal quality check.
	 */
	void csq_loop(void);

	/*
	 * Loop executed while in SATCOM_STATE_SBDSESSION
	 *
	 * Changes to SATCOM_STATE_STANDBY after finished sbd session.
	 */
	void sbdsession_loop(void);

	/*
	 * Loop executed while in SATCOM_STATE_TEST
	 *
	 * Changes to SATCOM_STATE_STANDBY after finished test.
	 */
	void test_loop(void);

	/*
	 * Get the network signal strength
	 */
	void start_csq(void);

	/*
	 * Start a sbd session
	 */
	void start_sbd_session(void);

	/*
	 * Check if the test command is valid. If that is the case
	 * change to SATCOM_STATE_TEST
	 */
	void start_test(void);

	/*
	 * Use to send mavlink messages directly
	 */
	ssize_t write(struct file *filp, const char *buffer, size_t buflen);

	/*
	 * Use to read received mavlink messages directly
	 */
	ssize_t read(struct file *filp, char *buffer, size_t buflen);

	/*
	 * Write the tx buffer to the modem
	 */
	void write_tx_buf();

	/*
	 * Read binary data from the modem
	 */
	void read_rx_buf();

	/*
	 * Send a AT command to the modem
	 */
	void write_at(const char *command);

	/*
	 * Read return from modem and store it in rx_command_buf
	 */
	satcom_result_code read_at_command(int16_t timeout = 100);

	/*
	 * Read return from modem and store it in rx_msg_buf
	 */
	satcom_result_code read_at_msg(int16_t timeout = 100);

	/*
	 * Read the return from the modem
	 */
	satcom_result_code read_at(uint8_t *rx_buf, int *rx_len, int16_t timeout = 100);

	/*
	 * Schedule a test (set test_pending to true)
	 */
	void schedule_test(void);

	/*
	 * Clear the MO message buffer
	 */
	bool clear_mo_buffer();

	/*
	 * Open and configure the given UART port
	 */
	satcom_uart_status open_uart(char *uart_name);

	/*
	 * Checks if the modem responds to the "AT" command
	 */
	bool is_modem_ready(void);

	/*
	 * Get the poll state
	 */
	pollevent_t poll_state(struct file *filp);

	/*
	 * Publish the up to date telemetry status
	 */
	void publish_telemetry_status(void);

	static IridiumSBD *instance;
	static int task_handle;
	bool task_should_exit = false;
	int uart_fd = -1;

	int32_t param_read_interval_s = -1;
	int32_t param_session_timeout_s = -1;
	int32_t param_stacking_time_ms = -1;

	hrt_abstime last_signal_check = 0;
	uint8_t signal_quality = 0;

	orb_advert_t telemetry_status_pub = nullptr;

	bool test_pending = false;
	char test_command[32];
	hrt_abstime test_timer = 0;

	uint8_t rx_command_buf[SATCOM_RX_COMMAND_BUF_LEN] = {0};
	int rx_command_len = 0;

	uint8_t rx_msg_buf[SATCOM_RX_MSG_BUF_LEN] = {0};
	int rx_msg_end_idx = 0;
	int rx_msg_read_idx = 0;

	uint8_t tx_buf[SATCOM_TX_BUF_LEN] = {0};
	int tx_buf_write_idx = 0;

	bool tx_buf_write_pending = false;
	bool ring_pending = false;
	bool rx_session_pending = false;
	bool rx_read_pending = false;
	bool tx_session_pending = false;

	hrt_abstime last_write_time = 0;
	hrt_abstime last_read_time = 0;
	hrt_abstime last_heartbeat = 0;
	hrt_abstime session_start_time = 0;

	satcom_state state = SATCOM_STATE_STANDBY;
	satcom_state new_state = SATCOM_STATE_STANDBY;

	pthread_mutex_t tx_buf_mutex = pthread_mutex_t();
	bool verbose = false;

	orb_advert_t _mavlink_log_pub{nullptr};
};
