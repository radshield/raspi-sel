add_library(record record.cpp)
target_link_libraries(record PUBLIC
	i2c
	cpupower
)
