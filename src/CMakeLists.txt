add_library(pigpio STATIC
	3rdparty/pigpio/command.c
	3rdparty/pigpio/pigpio.c
)
target_include_directories(pigpio PUBLIC
	3rdparty/pigpio
)

add_executable(record record.c)
target_link_libraries(record PUBLIC pigpio)
