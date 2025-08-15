#ifndef ALIEN_COMPILER_H
#define ALIEN_COMPILER_H
#include <stddef.h>
#include <stdint.h>
int compile_source(const char *src, uint8_t *out, size_t max);
#endif
