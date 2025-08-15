#include "blockchain.h"
#include "wah.h"
#include "sched.h"
#include <string.h>

#define MAX_BLOCKS 32

static struct block chain[MAX_BLOCKS];
static int bc_len;

static uint32_t simple_hash(uint32_t v, uint32_t prev)
{
    return (v * 2654435761u) ^ prev; // simple mix
}

void bc_init(void)
{
    memset(chain, 0, sizeof(chain));
    bc_len = 0;
}

void bc_add(uint32_t value)
{
    if (bc_len >= MAX_BLOCKS)
        return;
    uint32_t prev = bc_len ? chain[bc_len-1].prev_hash : 0;
    chain[bc_len].value = value;
    chain[bc_len].prev_hash = simple_hash(value, prev);
    bc_len++;
    wah_audit("BC_ADD", sched_current());
}

int bc_verify(void)
{
    uint32_t prev = 0;
    for (int i=0;i<bc_len;i++) {
        uint32_t h = simple_hash(chain[i].value, prev);
        if (h != chain[i].prev_hash)
            return 0;
        prev = h;
    }
    wah_audit("BC_VERIFY", sched_current());
    return 1;
}
