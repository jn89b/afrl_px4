include (${CMAKE_CURRENT_LIST_DIR}/uavcan_board_identity)

px4_add_board(
	PLATFORM nuttx
	VENDOR freefly
	MODEL can-rtk-gps
	LABEL canbootloader
	TOOLCHAIN arm-none-eabi
	ARCHITECTURE cortex-m7
	CONSTRAINED_MEMORY
	DRIVERS
		bootloaders
		lights/rgbled_ncp5623c
)
