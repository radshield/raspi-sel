add_library(record_system record_system.cpp)
target_link_libraries(record_system PUBLIC
	cpupower
)
