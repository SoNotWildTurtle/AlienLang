#include "mem.h"
#include "sched.h"
#include "wah.h"
#include "alienvm.h"
#include "paging.h"
#include "fs.h"
#include "net.h"
#include "timer.h"
#include "isr.h"
#include "security.h"
#include "blockchain.h"
#include "log.h"
#include "pkg.h"
#include "device.h"
#include "io.h"

static volatile unsigned char *video = (unsigned char*)0xb8000;
volatile uint32_t hotpatch_flag = 0;

static void print(const char *msg)
{
    log_write(msg, -1);
    for (unsigned i = 0; msg[i]; i++) {
        outb(0xE9, msg[i]);
        video[i*2] = msg[i];
        video[i*2 + 1] = 0x07;
    }
}

static void idle_task(void)
{
    for (;;)
        __asm__("hlt");
}

static void wah_task(void)
{
    for (;;)
        __asm__("hlt");
}

static uint8_t demo_code[] = {
    OP_PUSH, 42,
    OP_BCADD,
    OP_BCVERIFY,
    OP_WAHMSG, 5, 'H','e','l','l','o',
    OP_LOGMSG, 4, 'T','e','s','t',
    OP_CALL_WAH,
    OP_HALT
};

static void alien_task(void)
{
    struct VM vm;
    if (vm_init_context(&vm, demo_code, sizeof(demo_code), 128, 0) == 0) {
        run_vm(&vm);
    }
    for (;;)
        __asm__("hlt");
}

void kernel_main(void)
{
    paging_init();
    log_init();
    mem_init();
    bc_init();
    net_init();
    security_init();
    fs_init();
    pkg_init();
    device_register("console", 0);
    device_init_all();
    static const char demo_pkg[] = "Hello from pkg";
    pkg_install("welcome.txt", demo_pkg, sizeof(demo_pkg)-1);
    sched_init();
    wah_init(wah_task);
    timer_init();
    const char *msg = "AlienOS booted";
    char *buf = kmalloc(16);
    if (buf) {
        for (int i = 0; msg[i] && i < 15; i++)
            buf[i] = msg[i];
        buf[15] = '\0';
        print(buf);
    } else {
        print("memfail");
    }
    if (hotpatch_flag)
        print("Hotpatch active");
    sched_create(idle_task, 2, "idle");
    sched_create(alien_task, 1, "alien");
    for (;;) 
        __asm__("hlt");
}
