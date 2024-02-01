cmake_minimum_required(VERSION 3.16)
project(unittets-simplessd)

enable_testing()
add_subdirectory(googletest)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/simplessd
    ${gtest_SOURCE_DIR}/include 
    ${gtest_SOURCE_DIR}
)

add_subdirectory(simplessd)

add_executable(unit_test unit_test.cc)
target_link_libraries(unit_test simplessd gtest_main)