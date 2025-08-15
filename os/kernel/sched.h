#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "paging.h"

#define MAX_TASKS 16
#define NUM_LEVELS 3
#define TIMESLICE_HIGH 5
#define TIMESLICE_MED 10
#define TIMESLICE_LOW 20

enum task_state {
    TASK_READY,
    TASK_RUNNING
};

struct task {
    uint64_t *stack;
    uint64_t rsp;
    int active;
    int priority;
    int ticks;
    uint64_t runtime;
    enum task_state state;
    void (*entry)(void);
    struct task *next;
    struct page_table *pt;
    int trust;
    char name[16];
};

void sched_init(void);
struct task*  sched_create(void (*entry)(void), int priority, const char *name);
struct task*  sched_current(void);
void sched_tick(void);
void sched_kill_current(void);
int  sched_kill(int id);
int  sched_format_tasks(char *out, int max);
int  sched_format_usage(char *out, int max);
int  sched_format_trusts(char *out, int max);
int  sched_format_names(char *out, int max);
void sched_set_name(struct task *t, const char *name);
const char* sched_get_name(struct task *t);
struct task* sched_task(int id);
int  sched_task_id(struct task *t);

#endif
