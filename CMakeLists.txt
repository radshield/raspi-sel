cmake_minimum_required(VERSION 3.16)
project(raspi-sel)

set (CMAKE_CXX_STANDARD 17)

add_subdirectory(classify)
add_subdirectory(cpu-stat)
add_subdirectory(record_system)
add_subdirectory(record_ina3221)

add_executable(seelie main.cpp)
target_include_directories(seelie PUBLIC
	record_system
	record_ina3221
	classify
	cpu-stat
)
target_link_libraries(seelie PUBLIC
	record_system
	record_ina3221
	classify
	CPUSnapshot
)
target_link_options(seelie PUBLIC
    -static
)
