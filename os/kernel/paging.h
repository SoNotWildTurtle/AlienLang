#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

struct page_table {
    uint64_t entries[512];
};

void paging_init(void);
struct page_table* paging_new_table(void);
struct page_table* paging_clone_kernel(void);
void paging_load(struct page_table *pt);

#endif
