find_package(Boost)

add_library(classify classify.cpp)
target_include_directories(classify PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/../record_system
	${Boost_INCLUDE_DIR}
)
target_link_libraries(classify PUBLIC
	${Boost_LIBRARIES}
)
