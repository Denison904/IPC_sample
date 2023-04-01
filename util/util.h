#pragma once
#include <time.h>
#include <stdlib.h>

float message_per_second(unsigned count, time_t sec, time_t nanosec);

void timespec_show_info(struct timespec *start, struct timespec *end, unsigned message_count);

#define FD_BASE 100

enum TYPE {
    MAIN,
    WRITER,
    READER
};
    