px4_add_module(
	MODULE modules__supervisor_control
	MAIN supervisor_control
	SRCS
		SupervisorControl.cpp
		SupervisorControl.hpp
	DEPENDS
		px4_work_queue
	)
