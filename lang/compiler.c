#include "compiler.h"
#include "../os/kernel/alienvm.h"
#include <string.h>
#include <stdio.h>

int compile_source(const char *src, uint8_t *out, size_t max)
{
    int a, b;
    if (sscanf(src, "add %d %d", &a, &b) == 2) {
        if (max < 7) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_PUSH; out[3] = b;
        out[4] = OP_ADD;
        out[5] = OP_PRINT;
        out[6] = OP_HALT;
        return 7;
    }
    if (sscanf(src, "sub %d %d", &a, &b) == 2) {
        if (max < 7) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_PUSH; out[3] = b;
        out[4] = OP_SUB;
        out[5] = OP_PRINT;
        out[6] = OP_HALT;
        return 7;
    }
    if (sscanf(src, "mul %d %d", &a, &b) == 2) {
        if (max < 7) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_PUSH; out[3] = b;
        out[4] = OP_MUL;
        out[5] = OP_PRINT;
        out[6] = OP_HALT;
        return 7;
    }
    if (sscanf(src, "div %d %d", &a, &b) == 2) {
        if (max < 7) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_PUSH; out[3] = b;
        out[4] = OP_DIV;
        out[5] = OP_PRINT;
        out[6] = OP_HALT;
        return 7;
    }
    if (sscanf(src, "and %d %d", &a, &b) == 2) {
        if (max < 7) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_PUSH; out[3] = b;
        out[4] = OP_AND;
        out[5] = OP_PRINT;
        out[6] = OP_HALT;
        return 7;
    }
    if (sscanf(src, "or %d %d", &a, &b) == 2) {
        if (max < 7) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_PUSH; out[3] = b;
        out[4] = OP_OR;
        out[5] = OP_PRINT;
        out[6] = OP_HALT;
        return 7;
    }
    if (sscanf(src, "xor %d %d", &a, &b) == 2) {
        if (max < 7) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_PUSH; out[3] = b;
        out[4] = OP_XOR;
        out[5] = OP_PRINT;
        out[6] = OP_HALT;
        return 7;
    }
    if (sscanf(src, "mod %d %d", &a, &b) == 2) {
        if (max < 7) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_PUSH; out[3] = b;
        out[4] = OP_MOD;
        out[5] = OP_PRINT;
        out[6] = OP_HALT;
        return 7;
    }
    if (strcmp(src, "env") == 0) {
        if (max < 3) return -1;
        out[0] = OP_GETENV;
        out[1] = OP_PRINT;
        out[2] = OP_HALT;
        return 3;
    }
    if (strcmp(src, "time") == 0) {
        if (max < 3) return -1;
        out[0] = OP_TIME;
        out[1] = OP_PRINT;
        out[2] = OP_HALT;
        return 3;
    }
    if (strcmp(src, "rand") == 0) {
        if (max < 3) return -1;
        out[0] = OP_RANDOM;
        out[1] = OP_PRINT;
        out[2] = OP_HALT;
        return 3;
    }
    if (strcmp(src, "pid") == 0) {
        if (max < 3) return -1;
        out[0] = OP_PID;
        out[1] = OP_PRINT;
        out[2] = OP_HALT;
        return 3;
    }
    if (strcmp(src, "randport") == 0) {
        if (max < 3) return -1;
        out[0] = OP_RANDPORT;
        out[1] = OP_PRINT;
        out[2] = OP_HALT;
        return 3;
    }
    if (sscanf(src, "sleep %d", &a) == 1) {
        if (max < 4) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_SLEEP;
        out[3] = OP_HALT;
        return 4;
    }
    if (sscanf(src, "bcadd %d", &a) == 1) {
        if (max < 3) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_BCADD;
        out[3] = OP_HALT;
        return 4;
    }
    if (strcmp(src, "bcverify") == 0) {
        if (max < 2) return -1;
        out[0] = OP_BCVERIFY;
        out[1] = OP_HALT;
        return 2;
    }
    if (sscanf(src, "hash %d", &a) == 1) {
        if (max < 3) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_HASH;
        out[3] = OP_HALT;
        return 4;
    }
    if (sscanf(src, "loop %d", &a) == 1) {
        if (max < 21) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_STORE; out[3] = 0;
        out[4] = OP_LOAD; out[5] = 0;
        out[6] = OP_PRINT;
        out[7] = OP_LOAD; out[8] = 0;
        out[9] = OP_PUSH; out[10] = 1;
        out[11] = OP_SUB;
        out[12] = OP_STORE; out[13] = 0;
        out[14] = OP_LOAD; out[15] = 0;
        out[16] = OP_JZ; out[17] = 2;
        out[18] = OP_JMP; out[19] = (uint8_t)(-16);
        out[20] = OP_HALT;
        return 21;
    }
    if (sscanf(src, "set %d %d", &a, &b) == 2) {
        if (max < 5) return -1;
        out[0] = OP_PUSH; out[1] = b;
        out[2] = OP_STORE; out[3] = a;
        out[4] = OP_HALT;
        return 5;
    }
    if (sscanf(src, "get %d", &a) == 1) {
        if (max < 4) return -1;
        out[0] = OP_LOAD; out[1] = a;
        out[2] = OP_PRINT;
        out[3] = OP_HALT;
        return 4;
    }
    if (strncmp(src, "wahmsg ", 7) == 0) {
        const char *msg = src + 7;
        size_t len = strlen(msg);
        if (len > 31 || max < len + 3) return -1;
        out[0] = OP_WAHMSG;
        out[1] = (uint8_t)len;
        memcpy(&out[2], msg, len);
        out[2 + len] = OP_HALT;
        return len + 3;
    }
    if (strncmp(src, "cfgset ", 7) == 0) {
        char buf[32];
        int n = snprintf(buf, sizeof(buf), "CFGSET %s", src + 7);
        if (n < 0 || n > 31 || max < n + 3) return -1;
        out[0] = OP_WAHMSG;
        out[1] = (uint8_t)n;
        memcpy(&out[2], buf, n);
        out[2 + n] = OP_HALT;
        return n + 3;
    }
    if (strncmp(src, "cfgget ", 7) == 0) {
        char buf[32];
        int n = snprintf(buf, sizeof(buf), "CFGGET %s", src + 7);
        if (n < 0 || n > 31 || max < n + 3) return -1;
        out[0] = OP_WAHMSG;
        out[1] = (uint8_t)n;
        memcpy(&out[2], buf, n);
        out[2 + n] = OP_HALT;
        return n + 3;
    }
    if (strncmp(src, "logmsg ", 7) == 0) {
        const char *msg = src + 7;
        size_t len = strlen(msg);
        if (len > 31 || max < len + 3) return -1;
        out[0] = OP_LOGMSG;
        out[1] = (uint8_t)len;
        memcpy(&out[2], msg, len);
        out[2 + len] = OP_HALT;
        return len + 3;
    }
    if (sscanf(src, "print %d", &a) == 1) {
        if (max < 4) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_PRINT;
        out[3] = OP_HALT;
        return 4;
    }
    if (sscanf(src, "kill %d", &a) == 1) {
        if (max < 4) return -1;
        out[0] = OP_PUSH; out[1] = a;
        out[2] = OP_WAHKILL;
        out[3] = OP_HALT;
        return 4;
    }
    return -1; /* unknown syntax */
}
