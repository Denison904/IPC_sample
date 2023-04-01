#include <stdlib.h>
#include <linux/limits.h>

struct shared_memory {
    int event_fd;
    void *memory;
    unsigned size;
    int shm_fd;
    char shm_name[NAME_MAX];
    struct shared_memory_info *shm_info;
};

int shared_memory_init(struct shared_memory *shm_eventfd, char *shm_name, size_t size);

int shared_memory_open(struct shared_memory *shm_eventfd, char *shm_name, size_t size, int fd);

int shared_memory_deinit(struct shared_memory *shm_eventfd);

int shared_memory_write(struct shared_memory *shm_eventfd, void *mem, size_t size);

int shared_memory_read(struct shared_memory *shm_eventfd, void *mem, size_t size);