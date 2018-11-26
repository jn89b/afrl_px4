
set(cpu_flags "-mcpu=cortex-a53 -mfpu=neon -mfloat-abi=hard")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cpu_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cpu_flags}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${cpu_flags} -D__ASSEMBLY__")
