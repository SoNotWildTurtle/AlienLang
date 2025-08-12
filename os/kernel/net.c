#include "net.h"
#include "mem.h"

#define NET_QUEUE_SIZE 8
#define BUF_SIZE 64
#define NET_ENDPOINTS 256

static char queue[NET_ENDPOINTS][NET_QUEUE_SIZE][BUF_SIZE];
static int q_head[NET_ENDPOINTS];
static int q_tail[NET_ENDPOINTS];
static int wah_verified = 0;
static unsigned handshake_token = 0;

static int net_send_ep(int ep, const char *buf, size_t len);

static unsigned simple_hash_buf(const char *buf, size_t len, unsigned key)
{
    unsigned h = key;
    for (size_t i = 0; i < len; i++)
        h = (h * 2654435761u) ^ (unsigned char)buf[i];
    return h;
}

void net_init(void)
{
    for (int i = 0; i < NET_ENDPOINTS; i++) {
        q_head[i] = q_tail[i] = 0;
    }
    wah_verified = 0;
    handshake_token = 0;
}

void net_begin_handshake(unsigned token)
{
    wah_verified = 0;
    handshake_token = token;
}

static char hex_digit(unsigned v)
{
    return (v < 10) ? '0' + v : 'A' + (v - 10);
}

static void u32_to_hex(unsigned val, char *out)
{
    for (int i = 0; i < 8; i++) {
        out[7 - i] = hex_digit(val & 0xF);
        val >>= 4;
    }
}

int net_send_handshake(unsigned token)
{
    char msg[13]; /* HELLO + 8 hex */
    msg[0] = 'H'; msg[1] = 'E'; msg[2] = 'L'; msg[3] = 'L'; msg[4] = 'O';
    u32_to_hex(token, msg + 5);
    return net_send_ep(0, msg, 13);
}

static int net_send_ep(int ep, const char *buf, size_t len)
{
    if (((q_tail[ep] + 1) % NET_QUEUE_SIZE) == q_head[ep])
        return -1; /* full */
    if (len > BUF_SIZE)
        len = BUF_SIZE;
    for (size_t i = 0; i < len; i++)
        queue[ep][q_tail[ep]][i] = buf[i];
    queue[ep][q_tail[ep]][len] = '\0';
    q_tail[ep] = (q_tail[ep] + 1) % NET_QUEUE_SIZE;
    return (int)len;
}

int net_send(const char *buf, size_t len)
{
    return net_send_ep(0, buf, len);
}

int net_send_port(int port, const char *buf, size_t len)
{
    if (port < 0 || port >= NET_ENDPOINTS)
        return -1;
    return net_send_ep(port, buf, len);
}

int net_send_secure(const char *buf, size_t len, unsigned key)
{
    char tmp[BUF_SIZE];
    if (len > BUF_SIZE - 4)
        len = BUF_SIZE - 4;
    for (size_t i = 0; i < len; i++)
        tmp[i] = buf[i];
    unsigned h = simple_hash_buf(buf, len, key);
    tmp[len]     = (h >> 24) & 0xFF;
    tmp[len + 1] = (h >> 16) & 0xFF;
    tmp[len + 2] = (h >> 8) & 0xFF;
    tmp[len + 3] = h & 0xFF;
    return net_send_ep(0, tmp, len + 4);
}

static int net_recv_ep(int ep, char *buf, size_t len)
{
    if (q_head[ep] == q_tail[ep])
        return -1; /* empty */
    const char *src = queue[ep][q_head[ep]];
    if (!wah_verified) {
        if (src[0] == 'A' && src[1] == 'C' && src[2] == 'K') {
            unsigned val = 0;
            for (int i = 3; i < 11; i++) {
                char c = src[i];
                unsigned d;
                if ('0' <= c && c <= '9') d = c - '0';
                else if ('A' <= c && c <= 'F') d = c - 'A' + 10;
                else if ('a' <= c && c <= 'f') d = c - 'a' + 10;
                else { val = 0xFFFFFFFF; break; }
                val = (val << 4) | d;
            }
            if (val == handshake_token) {
                wah_verified = 1;
                q_head[ep] = (q_head[ep] + 1) % NET_QUEUE_SIZE;
                return 0;
            }
        }
        return -1;
    }
    size_t i;
    for (i = 0; src[i] && i < len; i++)
        buf[i] = src[i];
    if (i < len)
        buf[i] = '\0';
    q_head[ep] = (q_head[ep] + 1) % NET_QUEUE_SIZE;
    return (int)i;
}

int net_recv(char *buf, size_t len)
{
    return net_recv_ep(0, buf, len);
}

int net_recv_port(int port, char *buf, size_t len)
{
    if (port < 0 || port >= NET_ENDPOINTS)
        return -1;
    return net_recv_ep(port, buf, len);
}

int net_recv_secure(char *buf, size_t len, unsigned key)
{
    char tmp[BUF_SIZE];
    int r = net_recv_ep(0, tmp, BUF_SIZE);
    if (r < 4)
        return -1;
    unsigned h = (unsigned)((tmp[r-4] << 24) | (tmp[r-3] << 16) |
                             (tmp[r-2] << 8) | tmp[r-1]);
    int msg_len = r - 4;
    unsigned calc = simple_hash_buf(tmp, msg_len, key);
    if (h != calc)
        return -1;
    if ((size_t)msg_len > len)
        msg_len = len;
    for (int i = 0; i < msg_len; i++)
        buf[i] = tmp[i];
    if ((size_t)msg_len < len)
        buf[msg_len] = '\0';
    return msg_len;
}

int net_broadcast(const char *buf, size_t len)
{
    int ret = 0;
    for (int ep = 0; ep < NET_ENDPOINTS; ep++) {
        int r = net_send_ep(ep, buf, len);
        if (r > ret)
            ret = r;
    }
    return ret;
}

void net_mark_wah_verified(void)
{
    wah_verified = 1;
}

int net_is_wah_verified(void)
{
    return wah_verified;
}
