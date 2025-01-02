#include <libelf64.h>

#define print_elfN print_elf64

#define PRIdN PRId32
#define PRIuN PRIu32
#define PRIxN PRIx32

#include "elfn.c"
