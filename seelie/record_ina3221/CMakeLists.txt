add_library(record_ina3221 ina3221.cpp)
target_link_libraries(record_ina3221 PUBLIC
	i2c
)
