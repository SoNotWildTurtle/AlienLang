#ifndef DEVICE_H
#define DEVICE_H

#define MAX_DEVICES 16

typedef void (*device_init_fn)(void);

typedef struct {
    const char *name;
    device_init_fn init;
} device_t;

void device_register(const char *name, device_init_fn init);
void device_init_all(void);
int device_count(void);
int device_format_list(char *out, int max);

#endif
