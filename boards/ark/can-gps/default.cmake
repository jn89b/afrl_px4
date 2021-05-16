include (${CMAKE_CURRENT_LIST_DIR}/uavcan_board_identity)

px4_add_board(
	PLATFORM nuttx
	VENDOR ark
	MODEL can-gps
	LABEL default
	TOOLCHAIN arm-none-eabi
	ARCHITECTURE cortex-m4
	#CONSTRAINED_FLASH
	CONSTRAINED_MEMORY
	ROMFSROOT cannode
	UAVCAN_INTERFACES 1
	DRIVERS
		barometer/bmp388
		bootloaders
		gps
		imu/invensense/icm42688p
		magnetometer/bosch/bmm150
		safety_button
		tone_alarm
		uavcannode
	MODULES
		#ekf2
		#load_mon
		#sensors
	SYSTEMCMDS
		#mft
		#mtd
		param
		#perf
		#reboot
		#system_time
		#top
		#topic_listener
		#ver
		#work_queue
)
