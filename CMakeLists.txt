cmake_minimum_required(VERSION 3.16)
project(raspi-sel)

add_subdirectory(record_system)
add_subdirectory(record_ina3221)
add_subdirectory(classify)

add_executable(seelie main.cpp)
target_include_directories(seelie PUBLIC
	record_system
	record_ina3221
	classify
)
target_link_libraries(seelie PUBLIC
	record_system
	record_ina3221
	classify
)
