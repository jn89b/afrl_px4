
px4_add_board(
	VENDOR parrot
	MODEL bebop
	PLATFORM posix
	ARCHITECTURE cortex-a53
	TOOLCHAIN arm-linux-gnueabihf

	DRIVERS
		barometer/ms5611
		gps
		linux_sbus
		pwm_out_sim

	DF_DRIVERS # NOTE: DriverFramework is migrating to intree PX4 drivers
		mpu6050
		ak8963
		bebop_bus
		bebop_rangefinder
		mt9v117

	MODULES
		attitude_estimator_q
		#camera_feedback
		commander
		dataman
		ekf2
		events
		#fw_att_control
		#fw_pos_control_l1
		#rover_pos_control
		land_detector
		landing_target_estimator
		load_mon
		local_position_estimator
		logger
		mavlink
		mc_att_control
		mc_rate_control
		mc_pos_control
		navigator
		rc_update
		sensors
		sih
		#vtol_att_control
		airspeed_selector

	SYSTEMCMDS
		#config
		esc_calib
		led_control
		mixer
		motor_ramp
		param
		perf
		pwm
		reboot
		sd_bench
		#tests # tests and test runner
		top
		topic_listener
		tune_control
		ver
		work_queue

	)
