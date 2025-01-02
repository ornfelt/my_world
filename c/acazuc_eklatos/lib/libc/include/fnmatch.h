#ifndef FNMATCH_H
#define FNMATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define FNM_NOMATCH 1

#define FNM_NOESCAPE (1 << 0)
#define FNM_PATHNAME (1 << 1)
#define FNM_PERIOD   (1 << 2)

int fnmatch(const char *pattern, const char *str, int flags);

#ifdef __cplusplus
}
#endif

#endif
