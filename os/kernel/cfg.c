#include "cfg.h"

#define CFG_MAX 16
#define KEYLEN 16
#define VALUELEN 32

struct cfg_entry {
    char key[KEYLEN];
    char value[VALUELEN];
};

static struct cfg_entry cfg_store[CFG_MAX];

static int cfg_streq(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == 0 && *b == 0;
}

static void cfg_copy(char *dst, const char *src, int max)
{
    int i = 0;
    while (src[i] && i < max - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

void cfg_set(const char *key, const char *value)
{
    for (int i = 0; i < CFG_MAX; i++) {
        if (cfg_store[i].key[0] == '\0' || cfg_streq(cfg_store[i].key, key)) {
            cfg_copy(cfg_store[i].key, key, KEYLEN);
            cfg_copy(cfg_store[i].value, value, VALUELEN);
            return;
        }
    }
}

const char *cfg_get(const char *key)
{
    for (int i = 0; i < CFG_MAX; i++) {
        if (cfg_streq(cfg_store[i].key, key))
            return cfg_store[i].value;
    }
    return "";
}
