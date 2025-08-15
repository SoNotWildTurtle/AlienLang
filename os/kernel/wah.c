#include "wah.h"
#include "sched.h"
#include "wah_msg.h"
#include "net.h"
#include "log.h"
#include "timer.h"
#include "random.h"
#include "cfg.h"
#include "device.h"
#include "pkg.h"
#include "../../wah/wah_vm.h"
#include "mem.h"
#include "rtc.h"
#include "reboot.h"
#include <stddef.h>

static struct task *wah_task_ptr = 0;
static int env_mode = 0;

struct WahSession {
    char image[32];
    int trust;
    int active;
};

static struct WahSession wah_session = {"", 0, 0};

static int streq(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == '\0' && *b == '\0';
}

static int starts_with(const char *s, const char *pre)
{
    while (*pre) {
        if (*s != *pre)
            return 0;
        s++; pre++;
    }
    return 1;
}

static int parse_int(const char *s)
{
    int neg = 0;
    if (*s == '-') { neg = 1; s++; }
    int v = 0;
    while (*s >= '0' && *s <= '9') {
        v = v * 10 + (*s - '0');
        s++;
    }
    return neg ? -v : v;
}

static int append_int(char *out, int val)
{
    int n = 0;
    if (val < 0) { out[n++]='-'; val=-val; }
    char tmp[16];
    int j = 0;
    if (val == 0) {
        tmp[j++] = '0';
    } else {
        while (val > 0 && j < (int)sizeof(tmp)) {
            tmp[j++] = '0' + (val % 10);
            val /= 10;
        }
    }
    while (j-- > 0)
        out[n++] = tmp[j];
    return n;
}

static int str_len(const char *s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static void wah_process_message(char *msg)
{
    if (!wah_emotion_filter(msg) || !wah_metacog_check(msg))
        return;

    if (streq(msg, "PING")) {
        wah_msg_send("PONG", 4);
    } else if (starts_with(msg, "SETMODE ")) {
        int m = parse_int(msg + 8);
        wah_set_env_mode(m);
    } else if (starts_with(msg, "SETTRUST ")) {
        char *p = msg + 9;
        int id = parse_int(p);
        while (*p && *p != ' ') p++;
        if (*p == ' ') {
            p++;
            int t = parse_int(p);
            struct task *task = sched_task(id);
            if (task)
                wah_set_trust(task, t);
        } else {
            wah_set_trust(sched_current(), id);
        }
    } else if (streq(msg, "GETMODE")) {
        char out[16];
        int m = wah_get_env_mode();
        int n = 0;
        if (m < 0) { out[n++]='-'; m = -m; }
        if (m >= 100) { out[n++] = '0' + (m/100); m%=100; }
        if (m >= 10 || n>0) { out[n++] = '0' + (m/10); m%=10; }
        out[n++] = '0' + m;
        out[n] = '\0';
        wah_msg_send(out, n);
    } else if (starts_with(msg, "GETTRUST")) {
        char *p = msg + 8;
        int id;
        struct task *task;
        if (*p) {
            while (*p == ' ') p++;
            id = parse_int(p);
            task = sched_task(id);
        } else {
            task = sched_current();
        }
        char out[16];
        int m = task ? wah_get_trust(task) : 0;
        int n = 0;
        if (m < 0) { out[n++]='-'; m = -m; }
        if (m >= 100) { out[n++] = '0' + (m/100); m%=100; }
        if (m >= 10 || n>0) { out[n++] = '0' + (m/10); m%=10; }
        out[n++] = '0' + m;
        out[n] = '\0';
        wah_msg_send(out, n);
    } else if (starts_with(msg, "LOG ")) {
        wah_audit(msg + 4, sched_current());
    } else if (starts_with(msg, "VMSTART ")) {
        wah_vm_start(msg + 8);
    } else if (streq(msg, "VMSTOP")) {
        wah_vm_stop();
    } else if (streq(msg, "VMINFO")) {
        if (wah_vm_is_running()) {
            const char *img = wah_vm_image();
            wah_msg_send(img, str_len(img));
        } else {
            wah_msg_send("", 0);
        }
    } else if (streq(msg, "REBOOT")) {
        system_reboot();
    } else if (starts_with(msg, "IDE ")) {
        const char *img = msg + 4;
        int t = wah_get_trust(sched_current());
        wah_session_start(img, t);
    } else if (streq(msg, "IDEINFO")) {
        if (wah_session.active) {
            char out[64];
            int n = 0;
            const char *img = wah_session.image;
            for (int i = 0; img[i] && i < 32; i++)
                out[n++] = img[i];
            out[n++] = ' ';
            n += append_int(out + n, wah_session.trust);
            wah_msg_send(out, n);
        } else {
            wah_msg_send("", 0);
        }
    } else if (streq(msg, "GETLOG")) {
        wah_send_log();
    } else if (streq(msg, "CLRLOG")) {
        log_clear();
    } else if (streq(msg, "TIME")) {
        wah_send_time();
    } else if (streq(msg, "RTC")) {
        wah_send_rtc();
    } else if (streq(msg, "RANDOM")) {
        wah_send_random();
    } else if (starts_with(msg, "AND ")) {
        const char *p = msg + 4;
        int a = parse_int(p);
        while (*p && *p != ' ') p++;
        if (*p == ' ') p++;
        int b = parse_int(p);
        char out[16];
        int n = append_int(out, a & b);
        wah_msg_send(out, n);
    } else if (starts_with(msg, "OR ")) {
        const char *p = msg + 3;
        int a = parse_int(p);
        while (*p && *p != ' ') p++;
        if (*p == ' ') p++;
        int b = parse_int(p);
        char out[16];
        int n = append_int(out, a | b);
        wah_msg_send(out, n);
    } else if (starts_with(msg, "XOR ")) {
        const char *p = msg + 4;
        int a = parse_int(p);
        while (*p && *p != ' ') p++;
        if (*p == ' ') p++;
        int b = parse_int(p);
        char out[16];
        int n = append_int(out, a ^ b);
        wah_msg_send(out, n);
    } else if (streq(msg, "TASKS")) {
        wah_send_tasks();
    } else if (streq(msg, "NAMES")) {
        wah_send_names();
    } else if (streq(msg, "DEVICES")) {
        wah_send_devices();
    } else if (streq(msg, "PKGS")) {
        char out[128];
        int n = pkg_list(out, sizeof(out) - 1);
        wah_msg_send(out, n);
    } else if (streq(msg, "USAGE")) {
        char out[128];
        int n = sched_format_usage(out, sizeof(out) - 1);
        wah_msg_send(out, n);
    } else if (streq(msg, "TRUSTS")) {
        char out[128];
        int n = sched_format_trusts(out, sizeof(out) - 1);
        wah_msg_send(out, n);
    } else if (streq(msg, "MEM")) {
        unsigned long used, total;
        char out[32];
        mem_usage(&used, &total);
        int n = append_int(out, (int)used);
        out[n++] = '/';
        n += append_int(out + n, (int)total);
        wah_msg_send(out, n);
    } else if (starts_with(msg, "KILL ")) {
        int id = parse_int(msg + 5);
        sched_kill(id);
    } else if (starts_with(msg, "NAME ")) {
        sched_set_name(sched_current(), msg + 5);
    } else if (streq(msg, "PID")) {
        char out[16];
        int n = append_int(out, sched_task_id(sched_current()));
        wah_msg_send(out, n);
    } else if (streq(msg, "STATUS")) {
        char out[64];
        int n = 0;
        out[n++]='M';out[n++]='O';out[n++]='D';out[n++]='E';out[n++]=' ';
        n += append_int(out+n, wah_get_env_mode());
        out[n++]=' ';out[n++]='T';out[n++]='R';out[n++]='U';out[n++]='S';out[n++]='T';out[n++]=' ';
        n += append_int(out+n, wah_get_trust(sched_current()));
        out[n++]=' ';out[n++]='V';out[n++]='M';out[n++]=' ';
        n += append_int(out+n, wah_vm_is_running());
        wah_msg_send(out, n);
    } else if (starts_with(msg, "CFGSET ")) {
        char key[16];
        char val[32];
        char *p = msg + 7;
        int i = 0;
        while (*p && *p != ' ' && i < 15) key[i++] = *p++;
        key[i] = '\0';
        if (*p == ' ') p++;
        i = 0;
        while (*p && i < 31) val[i++] = *p++;
        val[i] = '\0';
        cfg_set(key, val);
    } else if (starts_with(msg, "CFGGET ")) {
        char key[16];
        char *p = msg + 7;
        int i = 0;
        while (*p && i < 15) key[i++] = *p++;
        key[i] = '\0';
        const char *v = cfg_get(key);
        wah_msg_send(v, str_len(v));
    }
}

void wah_connect_loop(void)
{
    unsigned token = rand_next();
    char msg[13];
    static const int pri_ports[10] = {5,7,9,13,37,42,69,80,123,443};
    static const char hex[] = "0123456789ABCDEF";
    net_begin_handshake(token);
    net_send_handshake(token);
    msg[0]='H'; msg[1]='E'; msg[2]='L'; msg[3]='L'; msg[4]='O';
    for (int i = 0; i < 8; i++)
        msg[5 + i] = hex[(token >> (28 - i*4)) & 0xF];
    while (!net_is_wah_verified()) {
        /* phase 1: gently probe preferred ports for 5s */
        uint64_t start = timer_ms();
        for (int i = 0; i < 10 && !net_is_wah_verified(); i++) {
            if ((timer_ms() - start) >= 5000)
                break;
            net_send_port(pri_ports[i], msg, sizeof(msg));
            msleep(100);
        }

        /* phase 2: aggressive random sweep for next 10s */
        start = timer_ms();
        while (!net_is_wah_verified() && (timer_ms() - start) < 10000) {
            int port = rand_next() % 256;
            net_send_port(port, msg, sizeof(msg));
            msleep(10);
        }

        /* phase 3: hyper aggressive full sweep for final 5s */
        start = timer_ms();
        while (!net_is_wah_verified() && (timer_ms() - start) < 5000) {
            for (int port = 0; port < 256 && !net_is_wah_verified(); port++)
                net_send_port(port, msg, sizeof(msg));
        }

        if (!net_is_wah_verified())
            msleep(30000);
    }
}

void wah_start_beacon(void)
{
    const char msg[] = "WAH_BEACON";
    wah_msg_broadcast(msg, sizeof(msg) - 1);
}

void wah_send_log(void)
{
    const char *buf = log_buffer();
    int len = log_length();
    char chunk[64];
    int pos = 0;
    while (pos < len) {
        int n = len - pos;
        if (n > 63)
            n = 63;
        for (int i = 0; i < n; i++)
            chunk[i] = buf[pos + i];
        chunk[n] = '\0';
        wah_msg_send(chunk, n);
        pos += n;
    }
}

void wah_send_time(void)
{
    uint64_t ms = timer_ms();
    char buf[32];
    int i = 0;
    if (ms == 0) {
        buf[i++] = '0';
    } else {
        char tmp[32];
        int j = 0;
        while (ms > 0 && j < (int)sizeof(tmp)) {
            tmp[j++] = '0' + (ms % 10);
            ms /= 10;
        }
        while (j-- > 0)
            buf[i++] = tmp[j];
    }
    buf[i] = '\0';
    wah_msg_send(buf, i);
}

void wah_send_rtc(void)
{
    struct rtc_time t;
    rtc_read(&t);
    char buf[32];
    int n = 0;
    n += append_int(buf + n, t.year);
    buf[n++]='-';
    if (t.month < 10) buf[n++]='0';
    n += append_int(buf + n, t.month);
    buf[n++]='-';
    if (t.day < 10) buf[n++]='0';
    n += append_int(buf + n, t.day);
    buf[n++]=' ';
    if (t.hour < 10) buf[n++]='0';
    n += append_int(buf + n, t.hour);
    buf[n++]=':';
    if (t.min < 10) buf[n++]='0';
    n += append_int(buf + n, t.min);
    buf[n++]=':';
    if (t.sec < 10) buf[n++]='0';
    n += append_int(buf + n, t.sec);
    wah_msg_send(buf, n);
}

void wah_send_random(void)
{
    unsigned r = rand_next();
    char buf[16];
    int n = append_int(buf, (int)(r & 0x7fffffff));
    wah_msg_send(buf, n);
}

void wah_send_tasks(void)
{
    char buf[64];
    int n = sched_format_tasks(buf, sizeof(buf));
    wah_msg_send(buf, n);
}

void wah_send_devices(void)
{
    char buf[64];
    int n = device_format_list(buf, sizeof(buf) - 1);
    wah_msg_send(buf, n);
}

void wah_send_names(void)
{
    char buf[128];
    int n = sched_format_names(buf, sizeof(buf) - 1);
    wah_msg_send(buf, n);
}

void wah_session_start(const char *image, int trust)
{
    int i = 0;
    while (i < 31 && image[i]) {
        wah_session.image[i] = image[i];
        i++;
    }
    wah_session.image[i] = '\0';
    wah_session.trust = trust;
    wah_session.active = 1;
    wah_vm_start(wah_session.image);
}

int wah_session_is_active(void)
{
    return wah_session.active;
}

const char *wah_session_image(void)
{
    return wah_session.image;
}

int wah_session_trust(void)
{
    return wah_session.trust;
}

static void default_wah_loop(void)
{
    char buf[64];
    for (;;) {
        if (wah_msg_recv(buf, sizeof(buf)) > 0) {
            wah_audit(buf, sched_current());
            wah_process_message(buf);
        }
        __asm__("hlt");
    }
}

void wah_init(void (*entry)(void))
{
    wah_msg_init();
    wah_connect_loop();
    if (!entry)
        entry = default_wah_loop;
    wah_start_beacon();
    wah_task_ptr = sched_create(entry, 0, "wah");
}

int wah_is_superuser(struct task *t)
{
    return t == wah_task_ptr;
}

void wah_audit(const char *action, struct task *t)
{
    (void)t;
    char buf[64];
    uint64_t ms = timer_ms();
    int n = 0;
    /* convert milliseconds to decimal */
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
    for (int i = 0; action[i] && n < (int)sizeof(buf) - 1; i++)
        buf[n++] = action[i];
    buf[n] = '\0';
    log_write(buf, n);
}

void wah_set_env_mode(int mode)
{
    env_mode = mode;
}

int wah_get_env_mode(void)
{
    return env_mode;
}

void wah_set_trust(struct task *t, int score)
{
    if (t)
        t->trust = score;
}

int wah_get_trust(struct task *t)
{
    return t ? t->trust : 0;
}

/* simple placeholder emotional filter */
int wah_emotion_filter(const char *msg)
{
    (void)msg;
    /* future work: analyze sentiment */
    return 1; /* allow */
}

/* simple placeholder metacognitive check */
int wah_metacog_check(const char *msg)
{
    (void)msg;
    return 1;
}

void wah_self_evolve(void)
{
    wah_audit("wah_evolve", sched_current());
}
