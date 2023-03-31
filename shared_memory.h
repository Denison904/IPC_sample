#include <stdlib.h>
#include <linux/limits.h>

struct shared_memory_info;

enum SHARED_MEMORY_TYPE {
    SMT_WRITER,
    SMT_READER,
    SMT_CREATER
};

struct shared_memory {
    void *memory;
    size_t size;
    struct shared_memory_info *shm_info;
    enum SHARED_MEMORY_TYPE type;
    int shm_fd;
    char shm_name[NAME_MAX];
    int signalfd;
};

int shared_memory_init(struct shared_memory *shm_signal, char *shm_name, size_t size);

int shared_memory_open(struct shared_memory *shm_signal, char *shm_name, size_t size, enum SHARED_MEMORY_TYPE type);

int shared_memory_set_pid(struct shared_memory *shm, pid_t pid);

int shared_memory_set_pid_glob(struct shared_memory *shm, pid_t pid, enum SHARED_MEMORY_TYPE type);

int shared_memory_deinit(struct shared_memory *shm_signal);

int shared_memory_write(struct shared_memory *shm_signal, void *mem, size_t size);

int shared_memory_read(struct shared_memory *shm_signal, void *mem, size_t size);