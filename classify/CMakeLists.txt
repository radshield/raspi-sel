find_package(Python3 COMPONENTS Interpreter Development)

add_library(classify classify.cpp)
target_include_directories(classify PUBLIC
	${Python3_INCLUDE_DIRS}
)
target_link_libraries(classify PUBLIC
	${Python3_LIBRARIES}
)
