#ifndef FS_H
#define FS_H

void fs_init(void);
int fs_create(const char *name, const char *data, unsigned size);
const char *fs_read(int id, unsigned *size);

#endif
