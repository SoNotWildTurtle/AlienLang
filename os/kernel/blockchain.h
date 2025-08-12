#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include <stdint.h>

struct block {
    uint32_t value;
    uint32_t prev_hash;
};

void bc_init(void);
void bc_add(uint32_t value);
int bc_verify(void);

#endif
