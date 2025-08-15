#ifndef SECURITY_H
#define SECURITY_H

#include "sched.h"

void security_init(void);
int security_allow_task(struct task *t);

#endif
