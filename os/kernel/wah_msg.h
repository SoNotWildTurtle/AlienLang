#ifndef WAH_MSG_H
#define WAH_MSG_H

#include <stddef.h>

void wah_msg_init(void);
int wah_msg_send(const char *msg, size_t len);
int wah_msg_recv(char *buf, size_t len);
int wah_msg_broadcast(const char *msg, size_t len);

#endif
