#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Very basic allocator. This is needed as we need to allocate in the shared memory.
   This one is too simple... and wrong. It does not free(!).
*/

#define SHARED_AREA_ADDR (void *)0x200000000
#define SHM_FILE "shared_mem_file"
#define SHM_SIZE 1024 * 1024 * 50

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

    server_fd = fd;
    server_memptr = memptr;
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

    // attach shared memory
    area_free_start = SHARED_AREA_ADDR;
    area_top = area_free_start + SHM_SIZE;
}

void allocator_init_test()
{
    // just something local, for tests
    area_free_start = malloc(32);
    *area_free_start = malloc(1024 * 1024);
    area_top = area_free_start + 1024 * 1024;
}

void *s_malloc(int size)
{
    // printf("malloc: %p->%p\n", area_free_start, *area_free_start);
    assert(area_free_start);                    //not initialized
    assert(*area_free_start + size < area_top); //OOM

    void *p = *area_free_start;
    *area_free_start += size;
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
