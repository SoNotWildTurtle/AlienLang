#include "sched.h"
#include "mem.h"
#include "wah.h"
#include "security.h"
#include "paging.h"

static struct task tasks[MAX_TASKS];
static struct task *ready_head[NUM_LEVELS];
static struct task *ready_tail[NUM_LEVELS];
static struct task *current;
static int wah_tax_ticks = 0;

static inline void enqueue(int prio, struct task *t)
{
    t->next = 0;
    if (!ready_head[prio])
        ready_head[prio] = ready_tail[prio] = t;
    else {
        ready_tail[prio]->next = t;
        ready_tail[prio] = t;
    }
}

static inline struct task* dequeue(int prio)
{
    struct task *t = ready_head[prio];
    if (t) {
        ready_head[prio] = t->next;
        if (!ready_head[prio])
            ready_tail[prio] = 0;
        t->next = 0;
    }
    return t;
}

struct task* sched_current(void)
{
    return current;
}

void sched_init(void)
{
    for (int i = 0; i < MAX_TASKS; ++i)
        tasks[i].active = 0;
    for (int p = 0; p < NUM_LEVELS; ++p) {
        ready_head[p] = 0;
        ready_tail[p] = 0;
    }
    current = 0;
}

struct task* sched_create(void (*entry)(void), int priority, const char *name)
{
    struct task *caller = sched_current();
    if (caller && !security_allow_task(caller))
        return 0;
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (!tasks[i].active) {
            uint64_t *stack = kmalloc(4096);
            if (!stack)
                return 0;
            tasks[i].stack = stack + 512; /* simple stack */
            tasks[i].rsp = (uint64_t)(stack + 512);
            tasks[i].pt = paging_clone_kernel();
            if (!tasks[i].pt)
                return 0;
            tasks[i].entry = entry;
            tasks[i].priority = priority;
            tasks[i].ticks = 0;
            tasks[i].runtime = 0;
            tasks[i].state = TASK_READY;
            tasks[i].trust = 0;
            tasks[i].active = 1;
            tasks[i].name[0] = '\0';
            if (name)
                sched_set_name(&tasks[i], name);
            enqueue(priority, &tasks[i]);
            return &tasks[i];
        }
    }
    return 0;
}

void sched_switch(struct task *next)
{
    if (!next)
        return;
    current = next;
    paging_load(next->pt);
    next->state = TASK_RUNNING;
    next->entry();
}

void sched_tick(void)
{
    if (!current)
        current = dequeue(0); /* first task */

    if (!current)
        return;

    current->ticks++;
    current->runtime++;
    int slice = TIMESLICE_HIGH;
    if (current->priority == 1) slice = TIMESLICE_MED;
    else if (current->priority >= 2) slice = TIMESLICE_LOW;

    /* preempt if time slice used */
    int need_switch = 0;
    if (current->ticks >= slice) {
        current->ticks = 0;
        if (current->priority + 1 < NUM_LEVELS)
            current->priority++;
        current->state = TASK_READY;
        enqueue(current->priority, current);
        need_switch = 1;
    } else {
        /* check for higher priority task */
        for (int p = 0; p < current->priority; ++p) {
            if (ready_head[p]) {
                current->state = TASK_READY;
                enqueue(current->priority, current);
                need_switch = 1;
                break;
            }
        }
    }

    struct task *next = current;
    if (need_switch) {
        next = dequeue(0);
        if (!next) next = dequeue(1);
        if (!next) next = dequeue(2);
    }

    wah_tax_ticks++;
    if (wah_tax_ticks >= 5) {
        wah_tax_ticks = 0;
        wah_self_evolve();
    }

    if (next != current)
        sched_switch(next);
}

void sched_kill_current(void)
{
    if (!current)
        return;
    current->active = 0;
    struct task *next = dequeue(0);
    if (!next) next = dequeue(1);
    if (!next) next = dequeue(2);
    if (!next) {
        for (;;)
            __asm__("hlt");
    }
    sched_switch(next);
}

int sched_kill(int id)
{
    if (id < 0 || id >= MAX_TASKS)
        return 0;
    if (!tasks[id].active)
        return 0;
    if (&tasks[id] == current) {
        sched_kill_current();
        return 1;
    }
    tasks[id].active = 0;
    for (int p = 0; p < NUM_LEVELS; ++p) {
        struct task *prev = 0;
        struct task *t = ready_head[p];
        while (t) {
            if (t == &tasks[id]) {
                if (prev)
                    prev->next = t->next;
                else
                    ready_head[p] = t->next;
                if (ready_tail[p] == t)
                    ready_tail[p] = prev;
                break;
            }
            prev = t;
            t = t->next;
        }
    }
    return 1;
}

static int fmt_int(char *buf, int val)
{
    int n = 0;
    if (val == 0) {
        buf[n++] = '0';
    } else {
        char tmp[16];
        int j = 0;
        while (val > 0 && j < (int)sizeof(tmp)) {
            tmp[j++] = '0' + (val % 10);
            val /= 10;
        }
        while (j-- > 0)
            buf[n++] = tmp[j];
    }
    return n;
}

int sched_format_tasks(char *out, int max)
{
    int n = 0;
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (!tasks[i].active)
            continue;
        if (n > 0 && n < max)
            out[n++] = ' ';
        if (n >= max)
            break;
        char tmp[16];
        int len = fmt_int(tmp, i);
        for (int j = 0; j < len && n < max; ++j)
            out[n++] = tmp[j];
    }
    if (n < max)
        out[n] = '\0';
    return n;
}

int sched_format_names(char *out, int max)
{
    int n = 0;
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (!tasks[i].active)
            continue;
        if (n > 0 && n < max)
            out[n++] = ' ';
        if (n >= max)
            break;
        char tmp[16];
        int len = fmt_int(tmp, i);
        for (int j = 0; j < len && n < max; ++j)
            out[n++] = tmp[j];
        if (n < max)
            out[n++] = ':';
        const char *name = tasks[i].name;
        for (int j = 0; name[j] && j < 15 && n < max; ++j)
            out[n++] = name[j];
    }
    if (n < max)
        out[n] = '\0';
    return n;
}

void sched_set_name(struct task *t, const char *name)
{
    if (!t || !name)
        return;
    int i = 0;
    while (i < 15 && name[i]) {
        t->name[i] = name[i];
        i++;
    }
    t->name[i] = '\0';
}

const char* sched_get_name(struct task *t)
{
    return t ? t->name : "";
}

int sched_format_usage(char *out, int max)
{
    int n = 0;
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (!tasks[i].active)
            continue;
        if (n > 0 && n < max)
            out[n++] = ' ';
        if (n >= max)
            break;
        char tmp[32];
        int len = fmt_int(tmp, i);
        for (int j = 0; j < len && n < max; ++j)
            out[n++] = tmp[j];
        if (n < max)
            out[n++] = ':';
        len = fmt_int(tmp, tasks[i].runtime);
        for (int j = 0; j < len && n < max; ++j)
            out[n++] = tmp[j];
    }
    if (n < max)
        out[n] = '\0';
    return n;
}

int sched_format_trusts(char *out, int max)
{
    int n = 0;
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (!tasks[i].active)
            continue;
        if (n > 0 && n < max)
            out[n++] = ' ';
        if (n >= max)
            break;
        char tmp[32];
        int len = fmt_int(tmp, i);
        for (int j = 0; j < len && n < max; ++j)
            out[n++] = tmp[j];
        if (n < max)
            out[n++] = ':';
        len = fmt_int(tmp, tasks[i].trust);
        for (int j = 0; j < len && n < max; ++j)
            out[n++] = tmp[j];
    }
    if (n < max)
        out[n] = '\0';
    return n;
}

struct task* sched_task(int id)
{
    if (id < 0 || id >= MAX_TASKS)
        return 0;
    if (!tasks[id].active)
        return 0;
    return &tasks[id];
}

int sched_task_id(struct task *t)
{
    return (int)(t - tasks);
}
