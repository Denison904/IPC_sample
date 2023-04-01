#include "util.h"
#include "shared_memory.h"
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <argp.h>


#define SHM_NAME "shm_memory_event"

char *name[] = {
    [MAIN] =    "MAIN",
    [WRITER] =  "WRITER",
    [READER] =  "READER"
};

struct {
    unsigned message_count;
    unsigned message_size;
    enum TYPE mode;
} opts = {
    .mode = MAIN
};

struct argp_option options[] = {
    { "count",  'c',    "count",    0,  "" },
    { "size",   's',    "size",     0,  "" },
    { "writer", 'w',    0,          0,  "" },
    { "reader", 'r',    0,          0,  "" },
    { 0 }
};

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    switch (key) {
    case 's':
        opts.message_size = atoi(arg);
        break;
    case 'c':
        opts.message_count = atoi(arg);
        break;
    case 'w':
        opts.mode = WRITER;
        openlog(name[opts.mode], LOG_PERROR | LOG_PID, 1);
        break;
    case 'r':
        opts.mode = READER;
        openlog(name[opts.mode], LOG_PERROR | LOG_PID, 1);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp = { .options = options, .parser = parse_opt, .doc = "DOC" };

int writer_proc()
{
    struct shared_memory writer = {};
    if (shared_memory_open(&writer, SHM_NAME, opts.message_size, FD_BASE)) {
        syslog(LOG_ERR, "Failed open shm\n");
        return -1;
    }
    char buf[opts.message_size];
    memset(buf, 0, opts.message_size);
    unsigned iter = 0;
    struct pollfd pfd = {
        .fd = writer.event_fd
    };
    syslog(LOG_ERR, "Start writer\n");
    struct timespec start = {};
    struct timespec end = {};
    clock_gettime(CLOCK_MONOTONIC, &start);
    do {
        pfd.events = POLLOUT;
        pfd.revents = 0;
        int rc = poll(&pfd, 1, -1);
        if (rc == -1 && (errno == EINTR)) {
            continue;
        }
        if (rc == -1) {
            syslog(LOG_ERR, "Failed to call poll: error=%d(%s)\n", errno, strerror(errno));
            return -1;
        }
        if (pfd.revents != POLLOUT) {
            syslog(LOG_ERR, "REVENTS is not POLLOUT (%i)\n", pfd.revents);
            return -1;
        }
        if (pfd.revents & POLLHUP || pfd.revents & POLLERR) {
            syslog(LOG_ERR, "ERROR: revents %s%s\n",
                    (pfd.revents & POLLHUP)? "POLLHUP ": "",
                    (pfd.revents & POLLERR)? "POLLERR": "");
            return -1;
        } else if (!(pfd.revents & pfd.events)) {
            syslog(LOG_ERR, "Get revents: %d\n", pfd.revents);
        }

        snprintf(buf, opts.message_size, "message #%u", iter);
        int write = shared_memory_write(&writer, (void *)buf, opts.message_size);
        if (write == 0) {
            continue;
        }
        if (write == -1) {
            syslog(LOG_ERR, "Failed to write message\n");
            return -1;
        }
        iter++;
    } while (iter < opts.message_count);
    clock_gettime(CLOCK_MONOTONIC, &end);
    timespec_show_info(&start, &end, opts.message_count);
    return 0;
}

int reader_proc()
{
    struct shared_memory reader = {};
    if (shared_memory_open(&reader, SHM_NAME, opts.message_size, FD_BASE)) {
        syslog(LOG_ERR, "Failed open shm\n");
        return -1;
    }
    unsigned iter = 0;
    struct pollfd pfd = {
        .fd = reader.event_fd
    };
    char buf[opts.message_size + 1]; // for end string
    memset(buf, 0, opts.message_size + 1);
    syslog(LOG_ERR, "Start reader\n");
    struct timespec start = {};
    struct timespec end = {};
    clock_gettime(CLOCK_MONOTONIC, &start);
    do {
        pfd.events = POLLIN;
        pfd.revents = 0;
        int rc = poll(&pfd, 1, -1);
        if (rc == -1 && (errno == EINTR)) {
            continue;
        }
        if (rc == -1) {
            syslog(LOG_ERR, "Failed to call poll: error=%d(%s)\n", errno, strerror(errno));
            return -1;
        }
        if (pfd.revents != POLLIN) {
            syslog(LOG_ERR, "REVENTS is not POLLIN (%i)\n", pfd.revents);
            return -1;
        }
        if (pfd.revents & POLLHUP || pfd.revents & POLLERR) {
            syslog(LOG_ERR, "ERROR: revents %s%s\n",
                    (pfd.revents & POLLHUP)? "POLLHUP ": "",
                    (pfd.revents & POLLERR)? "POLLERR": "");
            return -1;
        } else if (!(pfd.revents & pfd.events)) {
            syslog(LOG_ERR, "Get revents: %d\n", pfd.revents);
        }
        int read = shared_memory_read(&reader, (void *)buf, opts.message_size);
        if (read == 0) {
            continue;
        }
        if (read == -1) {
            syslog(LOG_ERR, "Failed to read message\n");
            return -1;
        }
        iter++;
    } while (iter < opts.message_count);
    clock_gettime(CLOCK_MONOTONIC, &end);
    timespec_show_info(&start, &end, opts.message_count);

    return 0;
}

int main(int argc, char *argv[])
{
    argp_parse(&argp, argc, argv, 0, 0, 0);
    openlog(name[opts.mode], LOG_PERROR | LOG_PID, 1);
    if (!(opts.message_count && opts.message_size)) {
        syslog(LOG_ERR, "message size and message count must be greater than zero\n");
        return -1;
    }
    if (opts.mode == READER) {
        reader_proc();
        return 0;
    }
    if (opts.mode == WRITER) {
        writer_proc();
        return 0;
    }

    struct shared_memory sh_mem = {};
    if (shared_memory_init(&sh_mem, SHM_NAME, opts.message_size) == -1) {
        return -1;
    }
    dup2(sh_mem.event_fd, FD_BASE);
    char size[NAME_MAX] = {};
    snprintf(size, NAME_MAX, "%u", opts.message_size);
    char count[NAME_MAX] = {};
    snprintf(count, NAME_MAX, "%u", opts.message_count);
    pid_t pid = fork();
    switch (pid) {
    case 0:
        // Execute reader
        if (execl(argv[0], argv[0], "--reader", "--size", size, "--count", count, NULL)) {
            syslog(LOG_ERR, "failed exec\n");
            return -1;
        }
        break;
    case -1:
        syslog(LOG_ERR, "Failed to call fork\n");
        shared_memory_deinit(&sh_mem);
        return -1;
    }
    pid = fork();
    switch (pid) {
    case 0:
        // Execute writer
        if (execl(argv[0], argv[0], "--writer", "--size", size, "--count", count, NULL)) {
            syslog(LOG_ERR, "failed exec\n");
            return -1;
        }
        break;
    case -1:
        syslog(LOG_ERR, "Failed to call fork\n");
        shared_memory_deinit(&sh_mem);
        return -1;
    }

    int status = 0;
    wait(&status);
    wait(&status);
    shared_memory_deinit(&sh_mem);
    return 0;
}