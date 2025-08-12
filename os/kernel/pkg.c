#include "pkg.h"
#include "mem.h"
#include "fs.h"

struct pkg_entry {
    const char *name;
    int file_id;
};

static struct pkg_entry pkgs[10];
static int pkg_count;

void pkg_init(void)
{
    pkg_count = 0;
}

int pkg_install(const char *name, const char *data, unsigned size)
{
    if (pkg_count >= 10)
        return -1;
    int id = fs_create(name, data, size);
    if (id < 0)
        return -1;
    pkgs[pkg_count].name = name;
    pkgs[pkg_count].file_id = id;
    return pkg_count++;
}

const char *pkg_get(const char *name, unsigned *size)
{
    for (int i = 0; i < pkg_count; i++) {
        /* naive string compare */
        const char *n = pkgs[i].name;
        int j;
        for (j = 0; name[j] && n[j]; j++)
            if (name[j] != n[j])
                break;
        if (!name[j] && !n[j])
            return fs_read(pkgs[i].file_id, size);
    }
    return 0;
}

int pkg_list(char *out, unsigned max)
{
    unsigned n = 0;
    for (int i = 0; i < pkg_count; i++) {
        const char *name = pkgs[i].name;
        for (int j = 0; name[j] && n + 1 < max; j++)
            out[n++] = name[j];
        if (i + 1 < pkg_count && n + 1 < max)
            out[n++] = ' ';
    }
    if (n < max)
        out[n] = '\0';
    return n;
}
