#include "security.h"

#include "wah.h"

void security_init(void)
{
    /* placeholder for future hardening setup */
}

int security_allow_task(struct task *caller)
{
    /* only Wah may create new tasks */
    if (!wah_is_superuser(caller))
        return 0;
    return 1;
}
