#include "elf_64.h"

#define ELFEM EM_MIPS_RS3_LE

#define R_NONE         R_MIPS_NONE
#define R_JMP_SLOT64   R_MIPS_JUMP_SLOT
#define R_GLOB_DAT64   R_MIPS_GLOB_DAT
#define R_ABS64        R_MIPS_64
#define R_TLS_DTPMOD64 R_MIPS_TLS_DTPMOD64
#define R_TLS_DTPOFF64 R_MIPS_TLS_DTPREL64
#define R_TLS_TPOFF64  R_MIPS_TLS_TPREL64
