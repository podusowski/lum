cmake_minimum_required(VERSION 3.15)
project(lum CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/Catch2)

add_executable(tests tests/test.cpp)
