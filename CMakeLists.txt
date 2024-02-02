cmake_minimum_required(VERSION 3.16)
project(unittets-simplessd)

enable_testing()
add_subdirectory(googletest)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/simplessd
    ${gtest_SOURCE_DIR}/include 
    ${gtest_SOURCE_DIR}
    ${DRAMPOWER_SOURCE_DIR}
)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# enable debug
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_subdirectory(simplessd)

#embedded DRAMPower
set(DRAMPOWER_SOURCE_DIR "." CACHE PATH "Source directory to DRAMPower library")
if (NOT EXISTS "${DRAMPOWER_SOURCE_DIR}/libdrampower/LibDRAMPower.h")
  message(FATAL_ERROR "Failed to find DRAMPower library. \
Check you specified DRAMPower path by -DDRAMPOWER_SOURCE_DIR.")
endif ()

add_library(random_generator 
    ${CMAKE_CURRENT_LIST_DIR}/random_generator.cc
)

add_executable(unit_test unit_test.cc)
target_compile_options(unit_test PRIVATE -fno-access-control)
target_link_libraries(unit_test simplessd gtest_main random_generator drampower)