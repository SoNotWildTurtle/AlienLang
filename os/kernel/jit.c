#include "alienvm.h"

void vm_jit_optimize(uint8_t *code, int len)
{
    int dst = 0;
    for (int i = 0; i < len;) {
        if (i + 4 < len && code[i] == OP_PUSH && code[i+2] == OP_PUSH &&
            code[i+4] == OP_ADD) {
            int sum = code[i+1] + code[i+3];
            code[dst++] = OP_PUSH;
            code[dst++] = sum & 0xFF;
            i += 5;
        } else {
            code[dst++] = code[i++];
        }
    }
    while (dst < len)
        code[dst++] = OP_HALT;
}
