#include "paging.h"
#include "mem.h"

extern struct page_table pml4[];

static struct page_table *kernel_pml4 = 0;

void paging_init(void)
{
    kernel_pml4 = &pml4[0];
}

struct page_table* paging_new_table(void)
{
    struct page_table *pt = kmalloc(sizeof(struct page_table));
    if (pt)
        for (int i = 0; i < 512; ++i)
            pt->entries[i] = 0;
    return pt;
}

struct page_table* paging_clone_kernel(void)
{
    struct page_table *pt = paging_new_table();
    if (!pt)
        return 0;
    for (int i = 0; i < 512; ++i)
        pt->entries[i] = kernel_pml4->entries[i];
    return pt;
}

void paging_load(struct page_table *pt)
{
    if (!pt) return;
    __asm__ volatile("mov %0, %%cr3" :: "r"(pt));
}
