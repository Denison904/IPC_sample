cmake_minimum_required(VERSION 3.16.3)

project(SOCKET)

add_executable(socket ${option} main.c)

target_link_libraries(socket util)