px4_add_library(FlightTestInput
	FlightTestInput.cpp
)

target_include_directories(FlightTestInput
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

# target_link_libraries(FlightTestInput PRIVATE
# 		controllib
# 		systemlib)

