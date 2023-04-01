cmake_minimum_required(VERSION 3.16.3)

project(BENCH_MARK)

include_directories(util)

add_subdirectory(util)
add_subdirectory(socket)
add_subdirectory(signal)
add_subdirectory(event)