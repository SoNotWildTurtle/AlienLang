#include "timer.h"
#include "io.h"
#include "isr.h"
#include "sched.h"

static uint64_t tick_count = 0;

void timer_isr(void)
{
    tick_count++;
    sched_tick();
}

extern void timer_isr_stub(void);

void timer_init(void)
{
    uint16_t divisor = 1193180 / 100; /* 100Hz */
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
    isr_init();
}

uint64_t timer_ticks(void)
{
    return tick_count;
}

uint64_t timer_ms(void)
{
    return timer_ticks() * 10;
}

void msleep(unsigned ms)
{
    uint64_t start = timer_ticks();
    uint64_t target = start + ms / 10; /* 100Hz => 10ms per tick */
    while (timer_ticks() < target)
        __asm__("hlt");
}
