#include "elf_32.h"

#define ELFEM EM_RISCV

#define R_NONE         R_RISCV_NONE
#define R_RELATIVE32   R_RISCV_RELATIVE
#define R_ABS32        R_RISCV_32
#define R_TLS_DTPMOD32 R_RISCV_TLS_DTPMOD32
#define R_TLS_DTPOFF32 R_RISCV_TLS_DTPREL32
#define R_TLS_TPOFF32  R_RISCV_TLS_TPREL32
