cmake_minimum_required(VERSION 3.15)
project(lum CXX)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};external/Catch")

find_package(Catch2)
add_executable(tests tests/test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
