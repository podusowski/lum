cmake_minimum_required(VERSION 3.15)
project(lum CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)
add_subdirectory(src)

add_executable(tests tests/test.cpp)
target_link_libraries(tests Threads::Threads lum)

add_executable(two_mutexes tests/two_mutexes.cpp)
target_link_libraries(two_mutexes Threads::Threads lum)

add_executable(nosync cases/nosync.cpp)
target_link_libraries(nosync Threads::Threads)

add_executable(sync cases/sync.cpp)
target_link_libraries(sync Threads::Threads)
