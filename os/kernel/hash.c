#include "hash.h"

unsigned hash_u32(unsigned v)
{
    unsigned h = 2166136261u;
    for (int i = 0; i < 4; i++) {
        h ^= (v >> (i * 8)) & 0xFF;
        h *= 16777619u;
    }
    return h;
}
