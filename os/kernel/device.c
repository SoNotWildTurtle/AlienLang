#include "device.h"

static device_t devices[MAX_DEVICES];
static int num_devices;

void device_register(const char *name, device_init_fn init) {
    if (num_devices >= MAX_DEVICES)
        return;
    devices[num_devices].name = name;
    devices[num_devices].init = init;
    num_devices++;
}

void device_init_all(void) {
    for (int i = 0; i < num_devices; i++) {
        if (devices[i].init)
            devices[i].init();
    }
}

int device_count(void) {
    return num_devices;
}

int device_format_list(char *out, int max) {
    int n = 0;
    for (int i = 0; i < num_devices && n < max; i++) {
        const char *name = devices[i].name ? devices[i].name : "";
        for (int j = 0; name[j] && n < max - 1; j++)
            out[n++] = name[j];
        if (i < num_devices - 1 && n < max - 1)
            out[n++] = ' ';
    }
    if (n < max)
        out[n] = '\0';
    return n;
}
