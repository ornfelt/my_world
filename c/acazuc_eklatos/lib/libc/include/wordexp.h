#ifndef WORDEXP_H
#define WORDEXP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WRDE_APPEND     (1 << 0)
#define WRDE_DOOFFS     (1 << 1)
#define WRDE_NOCMD      (1 << 2)
#define WRDE_REUSE      (1 << 3)
#define WRDE_SHOWERR    (1 << 4)
#define WRDE_UNDEF      (1 << 5)
#define WRDE_NP_NOBK    (1 << 6)
#define WRDE_NP_GET_VAR (1 << 7)
#define WRDE_NP_CMD_EXP (1 << 8)

#define WRDE_BADCHAR 1
#define WRDE_BADVAL  2
#define WRDE_CMDSUB  3
#define WRDE_NOSPACE 4
#define WRDE_SYNTAX  5

typedef struct wordexp wordexp_t;

struct wordexp
{
	size_t we_wordc;
	char **we_wordv;
	size_t we_offs;
	void *we_ptr;
	char **(*we_np_get_var)(wordexp_t *we, const char *name, size_t name_len);
	char *(*we_np_cmd_exp)(wordexp_t *we, const char *cmd, size_t cmd_len);
};

int wordexp(const char *word, wordexp_t *we, int flags);
void wordfree(wordexp_t *we);

#ifdef __cplusplus
}
#endif

#endif
