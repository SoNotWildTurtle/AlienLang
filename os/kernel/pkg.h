#ifndef PKG_H
#define PKG_H

void pkg_init(void);
int pkg_install(const char *name, const char *data, unsigned size);
const char *pkg_get(const char *name, unsigned *size);
int pkg_list(char *out, unsigned max);

#endif
