
px4_nuttx_configure(HWCLASS m4 CONFIG nsh ROMFS y ROMFSROOT px4fmu_test)

set(config_module_list
	#
	# Board support modules
	#
	drivers/device
	drivers/stm32
	drivers/stm32/adc
	drivers/stm32/tone_alarm
	drivers/led
	drivers/px4fmu
	drivers/px4io
	drivers/boards
	drivers/rgbled
	drivers/imu/mpu6000
#TO FIT	drivers/imu/mpu9250
	drivers/imu/lsm303d
	drivers/imu/l3gd20
	drivers/hmc5883
	#drivers/mb12xx
	#drivers/srf02
	#drivers/sf0x
	#drivers/ll40ls
	#drivers/teraranger
	drivers/gps
	#drivers/pwm_out_sim
	#drivers/hott
	drivers/blinkm
	drivers/airspeed
	drivers/barometer
	drivers/differential_pressure
	modules/sensors
	#drivers/mkblctrl
	drivers/px4flow
	#drivers/oreoled
	#drivers/vmount
	drivers/pwm_input
	drivers/camera_trigger
	#drivers/bst
	#drivers/lis3mdl
	drivers/tfmini

	#
	# System commands
	#
	systemcmds/bl_update
	systemcmds/config
	systemcmds/dumpfile
	#systemcmds/esc_calib
#TO FIT	systemcmds/hardfault_log
	systemcmds/mixer
	systemcmds/motor_ramp
	systemcmds/mtd
	systemcmds/nshterm
	systemcmds/param
	systemcmds/perf
	systemcmds/pwm
	systemcmds/reboot
	#systemcmds/sd_bench
	systemcmds/top
	#systemcmds/topic_listener
	systemcmds/ver

	#
	# Testing
	#
	drivers/distance_sensor/sf0x/sf0x_tests
	drivers/test_ppm
	#lib/rc/rc_tests
	modules/commander/commander_tests
	modules/mc_pos_control/mc_pos_control_tests
	lib/controllib/controllib_test
	modules/mavlink/mavlink_tests
	modules/uORB/uORB_tests
	systemcmds/tests

	#
	# General system control
	#
	modules/commander
	modules/load_mon
	modules/navigator
	modules/mavlink
	#modules/gpio_led
	#modules/uavcan
	modules/land_detector

	#
	# Estimation modules
	#
	#modules/attitude_estimator_q
	#modules/position_estimator_inav
	#modules/local_position_estimator
	modules/ekf2

	#
	# Vehicle Control
	#
	modules/fw_pos_control_l1
	modules/fw_att_control
	modules/mc_att_control
	modules/mc_pos_control
	modules/vtol_att_control

	#
	# Logging
	#
	#modules/logger
	#modules/sdlog2

	#
	# Library modules
	#
	modules/systemlib/param
	modules/systemlib
	modules/uORB
	modules/dataman

	#
	# Libraries
	#
	lib/controllib
	lib/conversion
	lib/DriverFramework/framework
	lib/ecl
	lib/led
	lib/mathlib
	lib/mixer
	lib/terrain_estimation
	lib/tunes
	lib/version
	lib/FlightTasks

	#
	# OBC challenge
	#
	#examples/bottle_drop

	#
	# Rover apps
	#
	#examples/rover_steering_control

	#
	# Demo apps
	#
	#examples/math_demo
	# Tutorial code from
	# https://px4.io/dev/px4_simple_app
	#examples/px4_simple_app

	# Tutorial code from
	# https://px4.io/dev/debug_values
	#examples/px4_mavlink_debug

	# Tutorial code from
	# https://px4.io/dev/example_fixedwing_control
	#examples/fixedwing_control

	# Hardware test
	#examples/hwtest
)
