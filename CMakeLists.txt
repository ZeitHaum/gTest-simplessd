cmake_minimum_required(VERSION 3.16)
project(unittets-simplessd)

enable_testing()
add_subdirectory(googletest)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/simplessd
    ${gtest_SOURCE_DIR}/include 
    ${gtest_SOURCE_DIR}
)

# enable debug
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_subdirectory(simplessd)

add_library(random_generator 
    ${CMAKE_CURRENT_LIST_DIR}/random_generator.cc
)

add_executable(unit_test unit_test.cc)
target_link_libraries(unit_test simplessd gtest_main random_generator)