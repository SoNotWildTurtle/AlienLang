#include "wah_msg.h"
#include "net.h"
#include "timer.h"

void wah_msg_init(void)
{
    net_init();
}

int wah_msg_send(const char *msg, size_t len)
{
    char buf[128];
    uint64_t ms = timer_ms();
    int n = 0;
    char tmp[32];
    int j = 0;
    if (ms == 0) {
        tmp[j++] = '0';
    } else {
        while (ms > 0 && j < (int)sizeof(tmp)) {
            tmp[j++] = '0' + (ms % 10);
            ms /= 10;
        }
    }
    while (j-- > 0 && n < (int)sizeof(buf) - 1)
        buf[n++] = tmp[j];
    if (n < (int)sizeof(buf) - 1)
        buf[n++] = ' ';
    size_t copy = len;
    if (copy > sizeof(buf) - n)
        copy = sizeof(buf) - n;
    for (size_t i = 0; i < copy; i++)
        buf[n++] = msg[i];
    return net_send(buf, n);
}

int wah_msg_recv(char *buf, size_t len)
{
    return net_recv(buf, len);
}

int wah_msg_broadcast(const char *msg, size_t len)
{
    char buf[128];
    uint64_t ms = timer_ms();
    int n = 0;
    char tmp[32];
    int j = 0;
    if (ms == 0) {
        tmp[j++] = '0';
    } else {
        while (ms > 0 && j < (int)sizeof(tmp)) {
            tmp[j++] = '0' + (ms % 10);
            ms /= 10;
        }
    }
    while (j-- > 0 && n < (int)sizeof(buf) - 1)
        buf[n++] = tmp[j];
    if (n < (int)sizeof(buf) - 1)
        buf[n++] = ' ';
    size_t copy = len;
    if (copy > sizeof(buf) - n)
        copy = sizeof(buf) - n;
    for (size_t i = 0; i < copy; i++)
        buf[n++] = msg[i];
    return net_broadcast(buf, n);
}
