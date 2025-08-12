#include "log.h"
#include <stddef.h>

static char log_buf[1024];
static int log_pos;

void log_init(void)
{
    log_pos = 0;
    log_buf[0] = '\0';
}

void log_clear(void)
{
    log_pos = 0;
    log_buf[0] = '\0';
}

void log_write(const char *msg, int len)
{
    if (len < 0) {
        len = 0;
        while (msg[len]) len++;
    }
    if (log_pos + len + 1 >= (int)sizeof(log_buf))
        return;
    for (int i = 0; i < len; i++)
        log_buf[log_pos + i] = msg[i];
    log_pos += len;
    log_buf[log_pos++] = '\n';
    log_buf[log_pos] = '\0';
}

const char *log_buffer(void)
{
    return log_buf;
}

int log_length(void)
{
    return log_pos;
}
