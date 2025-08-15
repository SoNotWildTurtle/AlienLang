#include "compiler.h"
#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s 'source'\n", argv[0]);
        return 1;
    }
    uint8_t code[64];
    int len = compile_source(argv[1], code, sizeof(code));
    if (len < 0) {
        fprintf(stderr, "compile error\n");
        return 1;
    }
    for (int i = 0; i < len; i++) {
        printf("%u ", code[i]);
    }
    printf("\n");
    return 0;
}
