#include "fs.h"
#include "mem.h"

struct file {
    const char *name;
    const char *data;
    unsigned size;
};

static struct file files[10];
static int file_count;

void fs_init(void)
{
    file_count = 0;
}

int fs_create(const char *name, const char *data, unsigned size)
{
    if (file_count >= 10)
        return -1;
    files[file_count].name = name;
    files[file_count].data = data;
    files[file_count].size = size;
    return file_count++;
}

const char *fs_read(int id, unsigned *size)
{
    if (id < 0 || id >= file_count)
        return 0;
    if (size)
        *size = files[id].size;
    return files[id].data;
}
