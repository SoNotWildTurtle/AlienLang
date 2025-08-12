#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
void timer_init(void);
void timer_isr(void);
uint64_t timer_ticks(void);
uint64_t timer_ms(void);
void msleep(unsigned ms);
#endif
