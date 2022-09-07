cmake_minimum_required(VERSION 3.15)
project(lum CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

add_subdirectory(external/Catch2)
add_subdirectory(src)

add_executable(tests tests/test.cpp)
target_link_libraries(tests Threads::Threads lum)
