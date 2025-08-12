#include "isr.h"
#include "wah.h"
#include "sched.h"
#include <stdint.h>

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

extern void timer_isr_stub(void);
extern void page_fault_isr_stub(void);

void page_fault_isr(void)
{
    struct task *t = sched_current();
    wah_audit("PAGE_FAULT", t);
    if (!wah_is_superuser(t)) {
        t->active = 0;
        sched_kill_current();
    } else {
        volatile unsigned char *v = (unsigned char*)0xb8000;
        v[0] = 'P'; v[1] = 0x4f;
        v[2] = 'F'; v[3] = 0x4f;
        for (;;)
            __asm__("hlt");
    }
}

static struct idt_entry idt[256];

static void idt_set_gate(int n, void (*handler)(void))
{
    uint64_t addr = (uint64_t)handler;
    idt[n].offset_low = addr & 0xFFFF;
    idt[n].selector = 0x08;
    idt[n].ist = 0;
    idt[n].type_attr = 0x8E;
    idt[n].offset_mid = (addr >> 16) & 0xFFFF;
    idt[n].offset_high = addr >> 32;
    idt[n].zero = 0;
}

void isr_init(void)
{
    idt_set_gate(14, page_fault_isr_stub);
    idt_set_gate(32, timer_isr_stub);
    struct idt_ptr idtp = { sizeof(idt)-1, (uint64_t)idt };
    __asm__ volatile("lidt %0" : : "m"(idtp));
    __asm__ volatile("sti");
}
