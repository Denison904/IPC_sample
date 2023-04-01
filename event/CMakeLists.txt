cmake_minimum_required(VERSION 3.16.3)

project(EVENT)

add_executable(shm_event ${option} shared_memory.c main.c)
target_link_libraries(shm_event util)
target_link_libraries(shm_event rt)