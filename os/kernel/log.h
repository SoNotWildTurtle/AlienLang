#ifndef LOG_H
#define LOG_H

void log_init(void);
void log_write(const char *msg, int len);
const char *log_buffer(void);
int log_length(void);
void log_clear(void);

#endif
