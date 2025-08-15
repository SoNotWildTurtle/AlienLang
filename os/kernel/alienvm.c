#include "alienvm.h"
#include "wah.h"
#include "sched.h"
#include "wah_msg.h"
#include "blockchain.h"
#include "log.h"
#include "mem.h"
#include "timer.h"
#include "random.h"
#include "hash.h"

static vm_handler_t handlers[256];

void vm_register_handler(uint8_t opcode, vm_handler_t h)
{
    handlers[opcode] = h;
}

int vm_init_context(struct VM *vm, uint8_t *code, int code_len, int stack_sz, int heap_sz)
{
    vm->bytecode = code;
    vm->code_len = code_len;
    vm->ip = 0;
    vm->trace = 0;
    vm->stack_size = stack_sz;
    vm->heap_size = heap_sz;
    vm->sp = 0;
    vm->stack = (SymbolicValue*)kmalloc(sizeof(SymbolicValue) * stack_sz);
    if (!vm->stack)
        return -1;
    if (heap_sz > 0) {
        vm->heap = (uint8_t*)kmalloc(heap_sz);
        if (!vm->heap)
            return -1;
    } else {
    vm->heap = 0;
    }
    for (int i = 0; i < 16; i++)
        vm->vars[i] = make_symbolic_constant(0);
    return 0;
}

static SymbolicValue make_sym(int v)
{
    SymbolicValue s;
    s.value = v;
    return s;
}

SymbolicValue make_symbolic_constant(int v)
{
    return make_sym(v);
}

SymbolicValue symbolic_add(SymbolicValue a, SymbolicValue b)
{
    return make_sym(a.value + b.value);
}

static SymbolicValue symbolic_sub(SymbolicValue a, SymbolicValue b)
{
    return make_sym(a.value - b.value);
}

static SymbolicValue symbolic_mul(SymbolicValue a, SymbolicValue b)
{
    return make_sym(a.value * b.value);
}

static SymbolicValue symbolic_div(SymbolicValue a, SymbolicValue b)
{
    if (b.value == 0) return make_sym(0);
    return make_sym(a.value / b.value);
}

static SymbolicValue symbolic_mod(SymbolicValue a, SymbolicValue b)
{
    if (b.value == 0) return make_sym(0);
    return make_sym(a.value % b.value);
}

static SymbolicValue symbolic_and(SymbolicValue a, SymbolicValue b)
{
    return make_sym(a.value & b.value);
}

static SymbolicValue symbolic_or(SymbolicValue a, SymbolicValue b)
{
    return make_sym(a.value | b.value);
}

static SymbolicValue symbolic_xor(SymbolicValue a, SymbolicValue b)
{
    return make_sym(a.value ^ b.value);
}

void symbolic_call_wah(SymbolicValue v)
{
    char buf[32];
    /* very simple decimal conversion */
    int val = v.value;
    int i = 0;
    if (val == 0) {
        buf[i++] = '0';
    } else {
        char tmp[16];
        int j = 0;
        int sign = val < 0;
        if (sign) val = -val;
        while (val && j < 16) {
            tmp[j++] = '0' + (val % 10);
            val /= 10;
        }
        if (sign) buf[i++] = '-';
        while (j--) buf[i++] = tmp[j];
    }
    buf[i] = '\0';
    wah_msg_send(buf, i);
    wah_audit("CALL_WAH", sched_current());
}

/* Built-in opcode handlers */
static void op_push(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[(*sp)++] = make_symbolic_constant(vm->bytecode[vm->ip++]);
}

static void op_add(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[*sp-2] = symbolic_add(stack[*sp-2], stack[*sp-1]);
    (*sp)--;
}

static void op_sub(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[*sp-2] = symbolic_sub(stack[*sp-2], stack[*sp-1]);
    (*sp)--;
}

static void op_mul(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[*sp-2] = symbolic_mul(stack[*sp-2], stack[*sp-1]);
    (*sp)--;
}

static void op_div(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[*sp-2] = symbolic_div(stack[*sp-2], stack[*sp-1]);
    (*sp)--;
}

static void op_mod(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[*sp-2] = symbolic_mod(stack[*sp-2], stack[*sp-1]);
    (*sp)--;
}

static void op_and(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[*sp-2] = symbolic_and(stack[*sp-2], stack[*sp-1]);
    (*sp)--;
}

static void op_or(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[*sp-2] = symbolic_or(stack[*sp-2], stack[*sp-1]);
    (*sp)--;
}

static void op_xor(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[*sp-2] = symbolic_xor(stack[*sp-2], stack[*sp-1]);
    (*sp)--;
}

static void op_random(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[(*sp)++] = make_symbolic_constant(rand_next());
}

static void op_time(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[(*sp)++] = make_symbolic_constant((int)timer_ms());
}

static void op_getenv(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[(*sp)++] = make_symbolic_constant(wah_get_env_mode());
}

static void op_randport(struct VM *vm, SymbolicValue *stack, int *sp)
{
    stack[(*sp)++] = make_symbolic_constant(rand_next() % 256);
}

static void op_pid(struct VM *vm, SymbolicValue *stack, int *sp)
{
    int id = sched_task_id(sched_current());
    stack[(*sp)++] = make_symbolic_constant(id);
}

static void op_sleep(struct VM *vm, SymbolicValue *stack, int *sp)
{
    msleep((unsigned)stack[--(*sp)].value);
}

static void op_print(struct VM *vm, SymbolicValue *stack, int *sp)
{
    symbolic_call_wah(stack[--(*sp)]);
}

static void op_call_wah(struct VM *vm, SymbolicValue *stack, int *sp)
{
    symbolic_call_wah(stack[--(*sp)]);
}

static void op_wahmsg(struct VM *vm, SymbolicValue *stack, int *sp)
{
    uint8_t len = vm->bytecode[vm->ip++];
    if (len > 31) len = 31; /* limit */
    char buf[32];
    for (int i = 0; i < len; i++)
        buf[i] = vm->bytecode[vm->ip++];
    wah_msg_send(buf, len);
    wah_audit("WAHMSG", sched_current());
}

static int fmt_int(char *buf, int val)
{
    int n = 0;
    if (val < 0) { buf[n++]='-'; val=-val; }
    if (val == 0) {
        buf[n++] = '0';
    } else {
        char tmp[16];
        int j = 0;
        while (val > 0 && j < (int)sizeof(tmp)) {
            tmp[j++] = '0' + (val % 10);
            val /= 10;
        }
        while (j-- > 0)
            buf[n++] = tmp[j];
    }
    return n;
}

static void op_wahkill(struct VM *vm, SymbolicValue *stack, int *sp)
{
    int id = stack[--(*sp)].value;
    char msg[16];
    int n = 0;
    msg[n++]='K';msg[n++]='I';msg[n++]='L';msg[n++]='L';msg[n++]=' ';
    n += fmt_int(msg + n, id);
    wah_msg_send(msg, n);
    wah_audit("WAHKILL", sched_current());
}

static void op_logmsg(struct VM *vm, SymbolicValue *stack, int *sp)
{
    uint8_t len = vm->bytecode[vm->ip++];
    if (len > 31) len = 31;
    char buf[32];
    for (int i = 0; i < len; i++)
        buf[i] = vm->bytecode[vm->ip++];
    log_write(buf, len);
    wah_audit("LOGMSG", sched_current());
}

static void op_bcadd(struct VM *vm, SymbolicValue *stack, int *sp)
{
    bc_add((uint32_t)stack[--(*sp)].value);
}

static void op_bcverify(struct VM *vm, SymbolicValue *stack, int *sp)
{
    int ok = bc_verify();
    stack[(*sp)++] = make_symbolic_constant(ok);
}

static void op_hash(struct VM *vm, SymbolicValue *stack, int *sp)
{
    unsigned v = (unsigned)stack[--(*sp)].value;
    unsigned h = hash_u32(v);
    stack[(*sp)++] = make_symbolic_constant((int)h);
}

static void op_store(struct VM *vm, SymbolicValue *stack, int *sp)
{
    uint8_t idx = vm->bytecode[vm->ip++];
    if (idx < 16)
        vm->vars[idx] = stack[--(*sp)];
}

static void op_load(struct VM *vm, SymbolicValue *stack, int *sp)
{
    uint8_t idx = vm->bytecode[vm->ip++];
    if (idx < 16)
        stack[(*sp)++] = vm->vars[idx];
    else
        stack[(*sp)++] = make_symbolic_constant(0);
}

static void op_jz(struct VM *vm, SymbolicValue *stack, int *sp)
{
    int8_t off = (int8_t)vm->bytecode[vm->ip++];
    SymbolicValue v = stack[--(*sp)];
    if (v.value == 0)
        vm->ip += off;
}

static void op_jmp(struct VM *vm, SymbolicValue *stack, int *sp)
{
    int8_t off = (int8_t)vm->bytecode[vm->ip++];
    vm->ip += off;
}

static void vm_init(void)
{
    for (int i = 0; i < 256; i++) handlers[i] = NULL;
    vm_register_handler(OP_PUSH, op_push);
    vm_register_handler(OP_ADD, op_add);
    vm_register_handler(OP_SUB, op_sub);
    vm_register_handler(OP_MUL, op_mul);
    vm_register_handler(OP_DIV, op_div);
    vm_register_handler(OP_MOD, op_mod);
    vm_register_handler(OP_AND, op_and);
    vm_register_handler(OP_OR, op_or);
    vm_register_handler(OP_XOR, op_xor);
    vm_register_handler(OP_RANDOM, op_random);
    vm_register_handler(OP_TIME, op_time);
    vm_register_handler(OP_GETENV, op_getenv);
    vm_register_handler(OP_RANDPORT, op_randport);
    vm_register_handler(OP_PID, op_pid);
    vm_register_handler(OP_SLEEP, op_sleep);
    vm_register_handler(OP_PRINT, op_print);
    vm_register_handler(OP_CALL_WAH, op_call_wah);
    vm_register_handler(OP_WAHMSG, op_wahmsg);
    vm_register_handler(OP_WAHKILL, op_wahkill);
    vm_register_handler(OP_LOGMSG, op_logmsg);
    vm_register_handler(OP_BCADD, op_bcadd);
    vm_register_handler(OP_BCVERIFY, op_bcverify);
    vm_register_handler(OP_HASH, op_hash);
    vm_register_handler(OP_STORE, op_store);
    vm_register_handler(OP_LOAD, op_load);
    vm_register_handler(OP_JZ, op_jz);
    vm_register_handler(OP_JMP, op_jmp);
}

void run_vm(struct VM *vm)
{
    static int init = 0;
    if (!init) { vm_init(); init = 1; }
    static int optimized = 0;
    if (!optimized) { vm_jit_optimize(vm->bytecode, vm->code_len); optimized = 1; }
    wah_audit("RUN_VM", sched_current());
    if (!vm->stack)
        return;
    SymbolicValue *stack = vm->stack;
    int *sp = &vm->sp;
    while (1) {
        uint8_t op = vm->bytecode[vm->ip++];
        vm_handler_t h = handlers[op];
        if (!h) return;
        if (vm->trace) {
            char msg[32];
            msg[0] = 'O'; msg[1] = 'P'; msg[2] = '_';
            msg[3] = '0' + (op/100)%10; /* simple decimal */
            msg[4] = '0' + (op/10)%10;  
            msg[5] = '0' + (op%10);
            msg[6] = 0;
            log_write(msg, -1);
        }
        h(vm, stack, sp);
        if (op == OP_HALT)
            return;
    }
}
