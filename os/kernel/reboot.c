#include "io.h"

void system_reboot(void)
{
    /* Use keyboard controller to trigger CPU reset */
    /* Wait until controller is ready */
    while (inb(0x64) & 0x02)
        ;
    outb(0x64, 0xFE);
    /* If that fails, halt */
    for (;;) {
        __asm__ volatile("hlt");
    }
}
