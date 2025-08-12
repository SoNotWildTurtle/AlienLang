#ifndef WAH_VM_H
#define WAH_VM_H

void wah_vm_start(const char *image);
void wah_vm_stop(void);
int  wah_vm_is_running(void);
const char *wah_vm_image(void);

#endif
