#ifndef RANDOM_H
#define RANDOM_H
#include <stdint.h>
void rand_seed(uint32_t s);
uint32_t rand_next(void);
#endif
