#include "random.h"

static uint32_t rnd_state = 1;

void rand_seed(uint32_t s)
{
    if (s) rnd_state = s;
}

uint32_t rand_next(void)
{
    /* xorshift32 algorithm for better randomness */
    uint32_t x = rnd_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rnd_state = x;
    return rnd_state;
}
