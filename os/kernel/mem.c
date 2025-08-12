#include "mem.h"

static unsigned long heap_start = 0x100000;
static unsigned long heap_ptr;
static unsigned long heap_end = 0x120000; /* simplistic 128KB heap */

void mem_init(void)
{
    heap_ptr = heap_start;
}

void* kmalloc(unsigned size)
{
    if (heap_ptr + size >= heap_end)
        return 0;
    void* ret = (void*)heap_ptr;
    heap_ptr += (size + 7) & ~7; /* align to 8 bytes */
    return ret;
}

void mem_usage(unsigned long *used, unsigned long *total)
{
    if (used)
        *used = heap_ptr - heap_start;
    if (total)
        *total = heap_end - heap_start;
}
