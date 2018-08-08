/****************************************************************************
 *
 *   Copyright (C) 2015-2018 PX4 Development Team. All rights reserved.
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
 * This is the main() of PX4 for POSIX.
 *
 * The application is designed as a daemon/server app with multiple clients.
 * Both, the server and the client is started using this main() function.
 *
 * If the executable is called with its usual name 'px4', it will start the
 * server. However, if it is started with an executable name (symlink) starting
 * with 'px4-' such as 'px4-navigator', it will start as a client and try to
 * connect to the server.
 *
 * The symlinks for all modules are created using the build system.
 *
 * @author Mark Charlebois <charlebm@gmail.com>
 * @author Roman Bapst <bapstroman@gmail.com>
 * @author Julian Oes <julian@oes.ch>
 * @author Beat Küng <beat-kueng@gmx.net>
 */

#include <string>
#include <fstream>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <px4_log.h>
#include <px4_tasks.h>
#include <px4_posix.h>
#include <px4_log.h>

#include "apps.h"
#include "px4_middleware.h"
#include "DriverFramework.hpp"
#include "px4_middleware.h"
#include "px4_daemon/client.h"
#include "px4_daemon/server.h"
#include "px4_daemon/pxh.h"


static const char *LOCK_FILE_PATH = "/tmp/px4_lock";

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif


static volatile bool _exit_requested = false;


namespace px4
{
void init_once(void);
}

static void sig_int_handler(int sig_num);
static void sig_fpe_handler(int sig_num);
static void sig_segv_handler(int sig_num);

static void register_sig_handler();
static void set_cpu_scaling();
static int create_symlinks_if_needed(std::string &data_path);
static int create_dirs();
static int run_startup_bash_script(const char *commands_file);
static void wait_to_exit();
static bool is_already_running();
static void print_usage();
static bool dir_exists(const std::string &path);
static bool file_exists(const std::string &name);
static std::string pwd();


#ifdef __PX4_SITL_MAIN_OVERRIDE
int SITL_MAIN(int argc, char **argv);

int SITL_MAIN(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	bool is_client = false;
	bool pxh_off = false;

	/* Symlinks point to all commands that can be used as a client with a prefix. */
	const char prefix[] = PX4_BASH_PREFIX;

	if (strstr(argv[0], prefix)) {
		is_client = true;
	}

	if (is_client) {

		if (!is_already_running()) {
			PX4_ERR("PX4 daemon not running yet");
			return -1;
		}

		/* Remove the prefix. */
		argv[0] += strlen(prefix);

		px4_daemon::Client client;
		client.generate_uuid();
		client.register_sig_handler();
		return client.process_args(argc, (const char **)argv);

	} else {
		if (is_already_running()) {
			PX4_ERR("PX4 daemon already running");
			return -1;
		}

		/* Server/daemon apps need to parse the command line arguments. */

		int positional_arg_count = 0;
		std::string data_path = "";
		std::string commands_file = "";
		std::string node_name = "";

		// parse arguments
		for (int i = 1; i < argc; ++i) {
			if (argv[i][0] == '-') {

				if (strcmp(argv[i], "-h") == 0) {
					print_usage();
					return 0;

				} else if (strcmp(argv[i], "-d") == 0) {
					pxh_off = true;

				} else {
					printf("Unknown/unhandled parameter: %s\n", argv[i]);
					print_usage();
					return 1;
				}

			} else if (!strncmp(argv[i], "__", 2)) {

				// FIXME: what is this?
				// ros arguments
				if (!strncmp(argv[i], "__name:=", 8)) {
					std::string name_arg = argv[i];
					node_name = name_arg.substr(8);
					PX4_INFO("node name: %s", node_name.c_str());
				}

			} else {
				//printf("positional argument\n");

				positional_arg_count += 1;

				if (positional_arg_count == 1) {
					commands_file = argv[i];

				} else if (positional_arg_count == 2) {
					data_path = commands_file;
					commands_file = argv[i];
				}
			}

			if (positional_arg_count != 2 && positional_arg_count != 1) {
				PX4_ERR("Error expected 1 or 2 position arguments, got %d", positional_arg_count);
				print_usage();
				return -1;
			}
		}

		if (!file_exists(commands_file)) {
			PX4_ERR("Error opening startup file, does not exist: %s", commands_file.c_str());
			return -1;
		}

		PX4_INFO("startup file: %s", commands_file.c_str());

		register_sig_handler();
		set_cpu_scaling();

		px4_daemon::Server server;
		server.start();

		int ret = create_symlinks_if_needed(data_path);

		if (ret != PX4_OK) {
			return ret;
		}

		ret = create_dirs();

		if (ret != PX4_OK) {
			return ret;
		}

		DriverFramework::Framework::initialize();

		px4::init_once();
		px4::init(argc, argv, "px4");

		ret = run_startup_bash_script(commands_file.c_str());

		// We now block here until we need to exit.
		if (pxh_off) {
			wait_to_exit();

		} else {
			px4_daemon::Pxh pxh;
			pxh.run_pxh();
		}

		// When we exit, we need to stop muorb on Snapdragon.

#ifdef __PX4_POSIX_EAGLE
		// Sending muorb stop is needed if it is running to exit cleanly.
		// TODO: we should check with px4_task_is_running("muorb") before stopping it.
		std::string muorb_stop_cmd("muorb stop");
		px4_daemon::Pxh::process_line(muorb_stop_cmd, true);
#endif

		std::string cmd("shutdown");
		px4_daemon::Pxh::process_line(cmd, true);

	}

	return PX4_OK;
}

int create_symlinks_if_needed(std::string &data_path)
{
	std::string current_path = pwd();

	if (data_path.size() == 0) {
		// No data path given, we'll just try to use the current working dir.
		data_path = current_path;
		PX4_INFO("assuming working directory is rootfs, no symlinks needed.");
		return PX4_OK;
	}

	if (data_path.compare(current_path) == 0) {
		// We are already running in the data path, so no need to symlink
		PX4_INFO("working directory seems to be rootfs, no symlinks needed");
		return PX4_OK;
	}

	std::vector<std::string> path_sym_links;
	path_sym_links.push_back("etc");
	path_sym_links.push_back("test_data");

	for (const auto &it = path_sym_links.begin(); it != path_sym_links.end(); ++it) {

		PX4_DEBUG("path sym link: %s", it->c_str());;

		std::string src_path = data_path + "/" + *it;
		std::string dest_path = current_path + "/" + *it;

		if (dir_exists(dest_path.c_str())) {
			continue;
		}

		PX4_INFO("Creating symlink %s -> %s", src_path.c_str(), dest_path.c_str());

		// create sym-links
		int ret = symlink(src_path.c_str(), dest_path.c_str());

		if (ret != 0) {
			PX4_ERR("Error creating symlink %s -> %s", src_path.c_str(), dest_path.c_str());
			return ret;

		} else {
			PX4_DEBUG("Successfully created symlink %s -> %s", src_path.c_str(), dest_path.c_str());
		}
	}

	return PX4_OK;
}

int create_dirs()
{
	std::string current_path = pwd();

	std::vector<std::string> dirs;
	dirs.push_back("log");

	for (int i = 0; i < dirs.size(); i++) {
		std::string dir = dirs[i];
		PX4_DEBUG("mkdir: %s", dir.c_str());;
		std::string dir_path = current_path + "/" + dir;

		if (dir_exists(dir_path)) {
			continue;
		}

		// create dirs
		int ret = mkdir(dir_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

		if (ret != OK) {
			PX4_WARN("failed creating new dir: %s", dir_path.c_str());
			return ret;

		} else {
			PX4_DEBUG("Successfully created dir %s", dir_path.c_str());
		}
	}

	return PX4_OK;
}


void register_sig_handler()
{
	// SIGINT
	struct sigaction sig_int {};
	sig_int.sa_handler = sig_int_handler;
	sig_int.sa_flags = 0;// not SA_RESTART!;

	// SIGFPE
	struct sigaction sig_fpe {};
	sig_int.sa_handler = sig_fpe_handler;
	sig_int.sa_flags = 0;// not SA_RESTART!;

	// SIGPIPE
	// We want to ignore if a PIPE has been closed.
	struct sigaction sig_pipe {};
	sig_pipe.sa_handler = SIG_IGN;

	// SIGSEGV
	struct sigaction sig_segv {};
	sig_segv.sa_handler = sig_segv_handler;
	sig_segv.sa_flags = SA_RESTART | SA_SIGINFO;

	sigaction(SIGINT, &sig_int, NULL);
	//sigaction(SIGTERM, &sig_int, NULL);
	sigaction(SIGFPE, &sig_fpe, NULL);
	sigaction(SIGPIPE, &sig_pipe, NULL);
	sigaction(SIGSEGV, &sig_segv, nullptr);
}

void sig_int_handler(int sig_num)
{
	fflush(stdout);
	printf("\nExiting...\n");
	fflush(stdout);
	px4_daemon::Pxh::stop();
	_exit_requested = true;
}

void sig_fpe_handler(int sig_num)
{
	fflush(stdout);
	printf("\nfloating point exception\n");
	PX4_BACKTRACE();
	fflush(stdout);
	px4_daemon::Pxh::stop();
	_exit_requested = true;
}

void sig_segv_handler(int sig_num)
{
	fflush(stdout);
	printf("\nSegmentation Fault\n");
	PX4_BACKTRACE();
	fflush(stdout);
}

void set_cpu_scaling()
{
#ifdef __PX4_POSIX_EAGLE
	// On Snapdragon we miss updates in sdlog2 unless all 4 CPUs are run
	// at the maximum frequency all the time.
	// Interestingely, cpu0 and cpu3 set the scaling for all 4 CPUs on Snapdragon.
	system("echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
	system("echo performance > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor");

	// Alternatively we could also raise the minimum frequency to save some power,
	// unfortunately this still lead to some drops.
	//system("echo 1190400 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
#endif
}

int run_startup_bash_script(const char *commands_file)
{
	std::string bash_command("bash ");

	bash_command += commands_file;

	PX4_INFO("Calling bash script: %s", bash_command.c_str());

	int ret = 0;

	if (!bash_command.empty()) {
		ret = system(bash_command.c_str());

		if (ret == 0) {
			PX4_INFO("Startup script returned successfully");

		} else {
			PX4_WARN("Startup script returned with return value: %d", ret);
		}

	} else {
		PX4_INFO("Startup script empty");
	}

	return ret;
}

void wait_to_exit()
{
	while (!_exit_requested) {
		usleep(100000);
	}
}

void print_usage()
{
	printf("Usage for Server/daemon process: \n");
	printf("\n");
	printf("    px4 [-h|-d] [rootfs_directory] startup_file\n");
	printf("\n");
	printf("    <rootfs_directory> directory where startup files and mixers are located,\n");
	printf("                       (if not given, CWD is used)\n");
	printf("    <startup_file>     bash start script to be used as startup\n");
	printf("        -h             help/usage information\n");
	printf("        -d             daemon mode, don't start pxh shell\n");
	printf("\n");
	printf("Usage for client: \n");
	printf("\n");
	printf("    px4-MODULE command using symlink.\n");
	printf("        e.g.: px4-commander status\n");
}

bool is_already_running()
{
	struct flock fl;
	int fd = open(LOCK_FILE_PATH, O_RDWR | O_CREAT, 0666);

	if (fd < 0) {
		return false;
	}

	fl.l_type   = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start  = 0;
	fl.l_len    = 0;
	fl.l_pid    = getpid();

	if (fcntl(fd, F_SETLK, &fl) == -1) {
		// We failed to create a file lock, must be already locked.

		if (errno == EACCES || errno == EAGAIN) {
			return true;
		}
	}

	return false;
}

bool px4_exit_requested(void)
{
	return _exit_requested;
}

bool file_exists(const std::string &name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

bool dir_exists(const std::string &path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0) {
		return false;

	} else if (info.st_mode & S_IFDIR) {
		return true;

	} else {
		return false;
	}
}

std::string pwd()
{
	char temp[PATH_MAX];
	return (getcwd(temp, PATH_MAX) ? std::string(temp) : std::string(""));
}
