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
#include <time.h>
#include <sys/socket.h>

struct shared_memory_info {
    size_t size;
};

// #define TIME
#define OFFSET sizeof(struct shared_memory_info)

int shared_memory_create(struct shared_memory *shm_eventfd, char *shm_name, size_t size)
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
        return -1;
    }

    void *mem = mmap(NULL, size + OFFSET, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mem == NULL) {
        syslog(LOG_ERR, "Failed to call mmap: error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }
    shm_eventfd->shm_info = mem;
    shm_eventfd->shm_fd = shm_fd;
    shm_eventfd->memory = mem + OFFSET;
    strcpy(shm_eventfd->shm_name, shm_name);
    shm_eventfd->size = size;
    return 0;
}

int shared_memory_init(struct shared_memory *shm_eventfd, char *shm_name, size_t size)
{
    if (shared_memory_create(shm_eventfd, shm_name, size)) {
        return 0;
    }
    int fd = eventfd(0, EFD_NONBLOCK);
    if (fd == -1) {
        shared_memory_deinit(shm_eventfd);
        syslog(LOG_ERR, "Failed to call eventfd: error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }

    shm_eventfd->event_fd = fd;
    return 0;
}

int shared_memory_deinit(struct shared_memory *shm_eventfd)
{
    shm_unlink(shm_eventfd->shm_name);
    munmap(shm_eventfd->memory, shm_eventfd->size + OFFSET);
    close(shm_eventfd->event_fd);
    return 0;
}

int shared_memory_open(struct shared_memory *shm_eventfd, char *shm_name, size_t size, int fd)
{
    if (shared_memory_create(shm_eventfd, shm_name, size)) {
        return 0;
    }

    shm_eventfd->event_fd = fd;
    return 0;
}

int shared_memory_write(struct shared_memory *shm_eventfd, void *mem, size_t size)
{
    if (size > shm_eventfd->size) {
        syslog(LOG_ERR, "So much data to write\n");
        return -1;
    }

    shm_eventfd->shm_info->size = size;
    memcpy(shm_eventfd->memory, mem, size);

    static eventfd_t d = 0xfffffffffffffffe;
    if (write(shm_eventfd->event_fd, &d, sizeof(d)) == -1) {
        syslog(LOG_ERR, "Failed to write eventfd: error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }
    return size;
}

int shared_memory_read(struct shared_memory *shm_eventfd, void *mem, size_t size)
{
    size_t read_bytes = shm_eventfd->shm_info->size;
    if (read_bytes > size) {
        syslog(LOG_ERR, "Failed to read msg size: %zu, buf size: %zu\n", read_bytes, size);
        return -1;
    }

    memcpy(mem, shm_eventfd->memory, read_bytes);
    static uint64_t d = 0;

    if (read(shm_eventfd->event_fd, &d, sizeof(d)) == -1) {
        syslog(LOG_ERR, "Failed to read eventfd: error=%d(%s)\n", errno, strerror(errno));
        return -1;
    }
    return read_bytes;
}