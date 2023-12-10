cmake_minimum_required(VERSION 3.16)
project(raspi-sel)

find_package(Python COMPONENTS Interpreter)

add_executable(record record.c)
target_include_directories(record PUBLIC
	${Python_INCLUDE_DIRS}
)
target_link_libraries(record PUBLIC
	i2c
	cpupower
	${Python_LIBRARIES}
)
