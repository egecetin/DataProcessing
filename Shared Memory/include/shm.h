#pragma once

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

/**
 * @brief
 *
 */
struct sharedMemInfo_t
{
    pthread_mutex_t lock;
    unsigned long read_index;
    unsigned long write_index;
    char data[1];
};
typedef struct sharedMemInfo_t sharedMemInfo;

/**
 * @brief
 *
 */
struct sharedMem_t
{
    unsigned long max_count;
    unsigned int element_size;
    unsigned long max_size;
    char *name;
    int shmem_fd;
    unsigned long mmap_size;
    sharedMemInfo *mem;
};
typedef struct sharedMem_t sharedMem;

sharedMem *shmemq_new(char const *name, unsigned long max_count, unsigned int element_size);
bool shmemq_try_enqueue(sharedMem *self, void *element, int len);
bool shmemq_try_dequeue(sharedMem *self, void *element, int len);
void shmemq_destroy(sharedMem *self, int unlink);