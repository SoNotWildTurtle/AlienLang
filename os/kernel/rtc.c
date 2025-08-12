#include "rtc.h"
#include "io.h"

static unsigned char cmos_read(unsigned char reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static int bcd_to_bin(unsigned char val) {
    return (val & 0x0F) + ((val / 16) * 10);
}

void rtc_read(struct rtc_time *t) {
    unsigned char sec, min, hour, day, mon, year, regB;
    // ensure values are consistent
    while (1) {
        sec = cmos_read(0x00);
        min = cmos_read(0x02);
        hour = cmos_read(0x04);
        day = cmos_read(0x07);
        mon = cmos_read(0x08);
        year = cmos_read(0x09);
        unsigned char sec2 = cmos_read(0x00);
        if (sec == sec2) break;
    }
    regB = cmos_read(0x0B);
    if (!(regB & 0x04)) { // BCD mode
        sec = bcd_to_bin(sec);
        min = bcd_to_bin(min);
        hour = bcd_to_bin(hour);
        day = bcd_to_bin(day);
        mon = bcd_to_bin(mon);
        year = bcd_to_bin(year);
    }
    t->sec = sec;
    t->min = min;
    t->hour = hour;
    t->day = day;
    t->month = mon;
    t->year = 2000 + year; // RTC year since 2000
}
