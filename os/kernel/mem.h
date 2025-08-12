#ifndef MEM_H
#define MEM_H

void mem_init(void);
void* kmalloc(unsigned size);
void mem_usage(unsigned long *used, unsigned long *total);

#endif
