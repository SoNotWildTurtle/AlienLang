#include "wah_vm.h"
#include "log.h"
#include <stddef.h>

static int vm_running;
static char vm_image[32];

void wah_vm_start(const char *image)
{
    if (vm_running)
        return; /* already running */

    vm_running = 1;
    if (image) {
        size_t i;
        for (i = 0; i < sizeof(vm_image)-1 && image[i]; i++)
            vm_image[i] = image[i];
        vm_image[i] = '\0';
        log_write("start_vm", -1);
        log_write(vm_image, -1);
    } else {
        vm_image[0] = '\0';
        log_write("start_vm", -1);
    }
}

void wah_vm_stop(void)
{
    if (!vm_running)
        return;

    vm_running = 0;
    log_write("stop_vm", -1);
}

int wah_vm_is_running(void)
{
    return vm_running;
}

const char *wah_vm_image(void)
{
    return vm_image;
}
