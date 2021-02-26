#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/* Very basic allocator. This is needed as we need to allocate in the shared memory.
   This one is too simple... and wrong. It does not free(!).
*/

void *area_free_start;
void *area_top;

void allocator_init()
{
    area_free_start = malloc(1024 * 1024);
    area_top = area_free_start + 1024 * 1024;
}

void *s_malloc(int size)
{
    assert(area_free_start);                   //not initialized
    assert(area_free_start + size < area_top); //OOM

    void *p = area_free_start;
    area_free_start += size;
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