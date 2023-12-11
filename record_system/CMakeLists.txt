add_library(record_system record.cpp)
target_link_libraries(record_system PUBLIC
	cpupower
)
