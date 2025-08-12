#ifndef ALIENVM_H
#define ALIENVM_H

#include <stdint.h>

typedef struct {
    int value;
} SymbolicValue;

struct VM {
    uint8_t *bytecode;
    int code_len;
    uint64_t ip;
    int trace;
    /* simple stack/heap management */
    SymbolicValue *stack;
    int sp;
    int stack_size;
    uint8_t *heap;
    int heap_size;
    SymbolicValue vars[16];
};

enum OpCode {
    OP_HALT = 0,
    OP_PUSH,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_RANDOM,
    OP_TIME,
    OP_GETENV,
    OP_RANDPORT,
    OP_SLEEP,
    OP_PRINT,
    OP_CALL_WAH,
    OP_WAHMSG,
    OP_LOGMSG,
    OP_BCADD,
    OP_BCVERIFY,
    OP_HASH,
    OP_STORE,
    OP_LOAD,
    OP_JZ,
    OP_JMP,
    OP_WAHKILL,
    OP_PID
};

typedef void (*vm_handler_t)(struct VM *, SymbolicValue *, int *);

void vm_register_handler(uint8_t opcode, vm_handler_t handler);
int vm_init_context(struct VM *vm, uint8_t *code, int code_len, int stack_sz, int heap_sz);

/* Optional bytecode optimizer */
void vm_jit_optimize(uint8_t *code, int len);

SymbolicValue make_symbolic_constant(int v);
SymbolicValue symbolic_add(SymbolicValue a, SymbolicValue b);
void symbolic_call_wah(SymbolicValue v);

void run_vm(struct VM *vm);

#endif
