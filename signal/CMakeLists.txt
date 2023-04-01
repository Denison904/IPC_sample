cmake_minimum_required(VERSION 3.16.3)

project(SIGNAL)

add_executable(shm_signal shared_memory.c main.c)
target_link_libraries(shm_signal util)
target_link_libraries(shm_signal rt)