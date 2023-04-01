#include "util.h"
#include <syslog.h>

float message_per_second(unsigned count, time_t sec, time_t nanosec)
{
    float fcount = (float)count;
    float fsec = (float)sec;
    float fnanosec = nanosec * 1e-9;
    return count / (fsec + fnanosec);
}

void timespec_diff(struct timespec *start, struct timespec *end, struct timespec *diff)
{
    if (start->tv_nsec > end->tv_nsec) {
        end->tv_sec -= 1;
        diff->tv_nsec = 1000000000 - start->tv_nsec + end->tv_nsec;
    } else {
        diff->tv_nsec = end->tv_nsec - start->tv_nsec;
    }
    diff->tv_sec = end->tv_sec - start->tv_sec;
}

void timespec_show_info(struct timespec *start, struct timespec *end, unsigned message_count)
{
    struct timespec diff = {};
    timespec_diff(start, end, &diff);
    syslog(LOG_ERR, "START_TIME: s: %lu,  n:%lu\n", start->tv_sec, start->tv_nsec);
    syslog(LOG_ERR, "END_TIME: s: %lu,  n:%lu\n", end->tv_sec, end->tv_nsec);
    syslog(LOG_ERR, "DURATION: sec: %lu, nanosec: %lu\n", diff.tv_sec, diff.tv_nsec);
    syslog(LOG_ERR, "Message per seconds: %f\n",
           message_per_second(message_count, diff.tv_sec, diff.tv_nsec));
}

