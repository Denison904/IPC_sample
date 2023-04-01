#include "shared_memory.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <poll.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <time.h>

struct shared_memory_info {
    pid_t server;
    pid_t client;
    size_t size;
};

#define OFFSET sizeof(struct shared_memory_info)

int shared_memory_init(struct shared_memory *shm_signal, char *shm_name, size_t size)
{
    if (strlen(shm_name) + 1 > NAME_MAX) {
        syslog(LOG_ERR, "Failed shm_name is long\n");
        return -1;
    }

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRWXU);
    if (shm_fd == -1) {
        syslog(LOG_ERR, "Failed to call shm_open: error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }

    int rc = ftruncate(shm_fd, size + OFFSET);
    if (rc == -1) {
        syslog(LOG_ERR, "Failed to call ftruncate: error=%d(%s)\n", errno, strerror(errno));
        shm_unlink(shm_name);
        return -1;
    }

    void *mem = mmap(NULL, size + OFFSET, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mem == NULL) {
        syslog(LOG_ERR, "Failed to call mmap: error=%d(%s)\n", errno, strerror(errno));
        shm_unlink(shm_signal->shm_name);
        return -1;
    }

    shm_signal->shm_info = mem;
    sigset_t mask = {};
    if (sigemptyset(&mask) == -1) {
        syslog(LOG_ERR, "Failed to call sigaddset: error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }
    if (sigaddset(&mask, SIGUSR1) == -1) {
        syslog(LOG_ERR, "Failed to call sigaddset: error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        syslog(LOG_ERR, "Failed to call sigprocmask: error=%d(%s)\n", errno, strerror(errno));
        munmap(mem, size + OFFSET);
        shm_unlink(shm_name);
        shm_signal->shm_info = NULL;
        return -1;
    }

    int sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (sfd == -1) {
        syslog(LOG_ERR, "Failed to call signalfd: error=%d(%s)\n", errno, strerror(errno));
        munmap(mem, size + OFFSET);
        shm_unlink(shm_name);
        shm_signal->shm_info = NULL;
        return -1;
    }
    shm_signal->signalfd = sfd;
    shm_signal->memory = mem + OFFSET;
    strcpy(shm_signal->shm_name, shm_name);
    shm_signal->shm_fd = shm_fd;
    shm_signal->size = size;
    shm_signal->type = SMT_CREATER;
    return 0;
}

int shared_memory_deinit(struct shared_memory *shm_signal)
{
    shm_unlink(shm_signal->shm_name);
    munmap(shm_signal->memory, shm_signal->size + OFFSET);
    close(shm_signal->signalfd);
    return 0;
}

int shared_memory_set_pid(struct shared_memory *shm, pid_t pid)
{
    switch (shm->type) {
    case SMT_WRITER:
        shm->shm_info->client = pid;
        break;
    case SMT_READER:
        shm->shm_info->server = pid;
        break;
    default:
        syslog(LOG_ERR, "Unknown type\n");
        return -1;
    }
    return 0;
}

int shared_memory_set_pid_glob(struct shared_memory *shm, pid_t pid, enum SHARED_MEMORY_TYPE type)
{
    switch (type) {
    case SMT_WRITER:
        shm->shm_info->client = pid;
        break;
    case SMT_READER:
        shm->shm_info->server = pid;
        break;
    default:
        syslog(LOG_ERR, "ERROR! Uncorrect type: %d\n", type);
        return -1;
    }
    syslog(LOG_ERR, "pid c:%d, s:%d\n", shm->shm_info->client, shm->shm_info->server);
    return 0;
}

int shared_memory_open(struct shared_memory *shm_signal, char *shm_name, size_t size, enum SHARED_MEMORY_TYPE type)
{
    if (strlen(shm_name) + 1 > NAME_MAX) {
        syslog(LOG_ERR, "Failed shm_name is long\n");
        return -1;
    }
    int shm_fd = shm_open(shm_name, O_RDWR, 0);
    if (shm_fd == -1) {
        syslog(LOG_ERR, "Failed to call shm_open: error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }
    void *mem = mmap(NULL, size + OFFSET, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mem == NULL) {
        syslog(LOG_ERR, "Failed to call mmap: error=%d(%s)\n", errno, strerror(errno));
        shm_unlink(shm_name);
        shm_signal->shm_info = NULL;
        return -1;
    }
    sigset_t mask = {};
    if (sigemptyset(&mask) == -1) {
        syslog(LOG_ERR, "Failed to call sigaddset: error=%d(%s)\n", errno, strerror(errno));
        munmap(mem, size + OFFSET);
        shm_unlink(shm_name);
        shm_signal->shm_info = NULL;
        return -1;
    }
    if (sigaddset(&mask, SIGUSR1) == -1) {
        syslog(LOG_ERR, "Failed to call sigaddset: error=%d(%s)\n", errno, strerror(errno));
        munmap(mem, size + OFFSET);
        shm_unlink(shm_name);
        shm_signal->shm_info = NULL;
        return -1;
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        syslog(LOG_ERR, "Failed to call sigprocmask: error=%d(%s)\n", errno, strerror(errno));
        munmap(mem, size + OFFSET);
        shm_unlink(shm_name);
        shm_signal->shm_info = NULL;
        return -1;
    }

    int sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (sfd == -1) {
        syslog(LOG_ERR, "Failed to call signalfd: error=%d(%s)\n", errno, strerror(errno));
        munmap(mem, size + OFFSET);
        shm_unlink(shm_name);
        shm_signal->shm_info = NULL;
        return -1;
    }
    shm_signal->signalfd = sfd;
    strcpy(shm_signal->shm_name, shm_name);
    shm_signal->shm_fd = shm_fd;
    shm_signal->type = type;
    shm_signal->memory = mem + OFFSET;
    shm_signal->shm_info = mem;
    shm_signal->size = size;

    if (type == SMT_WRITER) {
        kill(getpid(), SIGUSR1);
    }
    return 0;
}

int shared_memory_write(struct shared_memory *shm_signal, void *mem, size_t size)
{
    if (shm_signal->size < size) {
        syslog(LOG_ERR, "Failed to write msg size: %zu, buf size: %zu\n", shm_signal->size, size);
        return -1;
    }

    struct signalfd_siginfo info = {};
    int rc = read(shm_signal->signalfd, &info, sizeof(struct signalfd_siginfo));
    if (rc == -1 && errno == EAGAIN) {
        return 0;
    }
    if (rc == -1) {
        syslog(LOG_ERR, "Failed to read signal info, error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }

    shm_signal->shm_info->size = size;
    memcpy(shm_signal->memory, mem, size);
    pid_t target = shm_signal->type == SMT_WRITER? shm_signal->shm_info->server: shm_signal->shm_info->client;

    if (kill(target, SIGUSR1) == -1) {
        syslog(LOG_ERR, "Failed to send signal, error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }
    return size;
}

int shared_memory_read(struct shared_memory *shm_signal, void *mem, size_t size)
{
    size_t read_bytes = shm_signal->shm_info->size;
    if (read_bytes > size) {
        syslog(LOG_ERR, "Failed to read msg size: %zu, buf size: %zu\n", read_bytes, size);
        return -1;
    }
    struct signalfd_siginfo info = {};
    int rc = read(shm_signal->signalfd, &info, sizeof(struct signalfd_siginfo));
    if (rc == -1 && errno == EAGAIN) {
        return 0;
    }
    if (rc == -1) {
        syslog(LOG_ERR, "Failed to read signal info, error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }

    memcpy(mem, shm_signal->memory, size);
    pid_t target = shm_signal->type == SMT_WRITER? shm_signal->shm_info->server: shm_signal->shm_info->client;
    if (kill(target, SIGUSR1) == -1) {
        syslog(LOG_ERR, "Failed to send signal, error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }
    return read_bytes;
}