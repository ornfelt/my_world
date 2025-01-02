#include <libelf64.h>

#define print_elfN print_elf64

#define PRIdN PRId64
#define PRIuN PRIu64
#define PRIxN PRIx64

#include "elfn.c"
