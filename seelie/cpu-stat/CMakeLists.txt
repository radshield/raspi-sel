add_library(CPUData CPUData.cpp)

add_library(CPUSnapshot CPUSnapshot.cpp)
target_link_libraries(CPUSnapshot PUBLIC
  CPUData
)
