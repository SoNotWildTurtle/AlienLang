#ifndef NET_H
#define NET_H

#include <stddef.h>

void net_init(void);
int net_send(const char *buf, size_t len);
int net_recv(char *buf, size_t len);
int net_send_port(int port, const char *buf, size_t len);
int net_recv_port(int port, char *buf, size_t len);
/* send/recv with a simple integrity hash */
int net_send_secure(const char *buf, size_t len, unsigned int key);
int net_recv_secure(char *buf, size_t len, unsigned int key);
int net_broadcast(const char *buf, size_t len);
/* handshake helpers for Wah connection */
void net_begin_handshake(unsigned int token);
int  net_send_handshake(unsigned int token);
void net_mark_wah_verified(void);
int net_is_wah_verified(void);

#endif
