#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include "dict.h"

/* Very basic allocator. This is needed as we need to allocate in the shared memory.
   This one is too simple... and wrong. It does not free(!).
*/

#define SHARED_AREA_ADDR (void *)0x200000000
#define SHM_FILE "shared_mem_file"
#define SHM_SIZE 1024 * 1024 * 50
#define SEM_ALLOC "/SemAlloc"
#define SEM_TRIE "/SemTrie"

void **area_free_start;
void *area_top;

/* memory map:
   ptr - allocator
   ptr to root
   and then it's user data
*/

void perr(const char *msg)
{
    perror(msg);
    exit(-1);
}

int server_fd;
void *server_memptr;

sem_t *sem_alloc;
sem_t *sem_trie;

void allocator_init_server()
{
    // new shared memory
    int fd = shm_open(SHM_FILE, O_RDWR | O_CREAT, 0644);
    if (fd < 0)
        perr("Fail: shm_open (server)");

    ftruncate(fd, SHM_SIZE);

    void *memptr = mmap(SHARED_AREA_ADDR,
                        SHM_SIZE,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_FIXED,
                        fd,
                        0);
    // printf("Shared address memory: %p\n", memptr);
    if ((void *)-1 == memptr)
        perr("Fail: mmap (server)");

    assert(memptr == SHARED_AREA_ADDR);

    area_free_start = SHARED_AREA_ADDR;
    *area_free_start = SHARED_AREA_ADDR + sizeof(void *) * 2;
    area_top = SHARED_AREA_ADDR + SHM_SIZE;

    sem_alloc = sem_open(SEM_ALLOC, O_CREAT, 0644, 1);
    if (sem_alloc == (void *)-1)
        perr("sem_open - alloc");
    sem_trie = sem_open(SEM_TRIE, O_CREAT, 0644, 1);
    if (sem_trie == (void *)-1)
        perr("sem_open - trie");

    server_fd = fd;
    server_memptr = memptr;

    if (sem_post(sem_alloc) < 0)
        perr("sem_post");
    if (sem_post(sem_trie) < 0)
        perr("sem_post");
}

void **trie_root_ptr()
{
    return SHARED_AREA_ADDR + sizeof(void *);
}

void allocator_deinit_server()
{
    munmap(server_memptr, SHM_SIZE);
    close(server_fd);
    shm_unlink(SHARED_AREA_ADDR);
    sem_close(sem_alloc);
    sem_close(sem_trie);
}

void allocator_init_client()
{
    // attach shared memory
    int fd = shm_open(SHM_FILE, O_RDWR, 0644);
    if (fd < 0)
        perr("Fail: shm_open (client)");

    void *memptr = mmap(SHARED_AREA_ADDR,
                        SHM_SIZE,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_FIXED,
                        fd,
                        0);
    if ((void *)-1 == memptr)
        perr("Fail: mmap (client)");

    sem_alloc = sem_open(SEM_ALLOC, 0);
    if (sem_alloc == (void *)-1)
        perr("sem_open - alloc");
    sem_trie = sem_open(SEM_TRIE, 0);
    if (sem_trie == (void *)-1)
        perr("sem_open - trie");

    area_free_start = SHARED_AREA_ADDR;
    area_top = area_free_start + SHM_SIZE;
}

void allocator_lock()
{
    sem_wait(sem_alloc);
};
void allocator_unlock()
{
    sem_post(sem_alloc);
};

void trie_lock()
{
    sem_wait(sem_trie);
};
void trie_unlock()
{
    sem_post(sem_trie);
};

void allocator_init_test()
{
    // just something local, for tests
    area_free_start = malloc(32);
    *area_free_start = malloc(1024 * 1024);
    area_top = area_free_start + 1024 * 1024;
}

void *s_malloc(int size)
{
    allocator_lock();
    assert(area_free_start);                    //not initialized
    assert(*area_free_start + size < area_top); //OOM

    void *p = *area_free_start;
    *area_free_start += size;
    allocator_unlock();
    return p;
}

void s_free() {}

void *s_calloc(int size)
{
    void *p = s_malloc(size);
    memset(p, 0, size);
    return p;
}

char *s_strdup(char *str)
{

    char *c = s_malloc(strlen(str) + 1);
    strcpy(c, str);

    return c;
}
