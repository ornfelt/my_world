#include <elf32.h>

typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Phdr Elf_Phdr;
typedef Elf32_Dyn  Elf_Dyn;
typedef Elf32_Sym  Elf_Sym;
typedef Elf32_Rel  Elf_Rel;
typedef Elf32_Rela Elf_Rela;

#define ELFCLASS ELFCLASS32
#define ELFEM    EM_ARM

#define ELF_ST_BIND ELF32_ST_BIND
#define ELF_ST_TYPE ELF32_ST_TYPE

#define ELF_R_TYPE ELF32_R_TYPE
#define ELF_R_SYM  ELF32_R_SYM

#define R_NONE       R_ARM_NONE
#define R_RELATIVE32 R_ARM_RELATIVE
#define R_JMP_SLOT32 R_ARM_JUMP_SLOT
#define R_GLOB_DAT32 R_ARM_GLOB_DAT
#define R_ABS32      R_ARM_ABS32