#ifndef DLFCN_H
#define DLFCN_H

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_LOCAL  (0)
#define RTLD_LAZY   (1 << 0)
#define RTLD_NOW    (1 << 1)
#define RTLD_GLOBAL (1 << 2)

#define RTLD_DEFAULT ((void*)1)

void *dlopen(const char *filename, int flags);
int dlclose(void *handle);
char *dlerror(void);
void *dlsym(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

#endif
