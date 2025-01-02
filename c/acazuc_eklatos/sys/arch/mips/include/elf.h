#if defined(__mips64)

#include <elf64.h>

typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Phdr Elf_Phdr;
typedef Elf64_Dyn  Elf_Dyn;
typedef Elf64_Sym  Elf_Sym;
typedef Elf64_Rel  Elf_Rel;
typedef Elf64_Rela Elf_Rela;

#define ELFCLASS ELFCLASS64
#define ELFEM    EM_MIPS_RS3_LE

#define ELF_ST_BIND ELF64_ST_BIND
#define ELF_ST_TYPE ELF64_ST_TYPE

#define ELF_R_TYPE ELF64_R_TYPE
#define ELF_R_SYM  ELF64_R_SYM

#define R_NONE       R_MIPS_NONE
#define R_ABS64      R_MIPS_64

#else

#include <elf32.h>

typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Phdr Elf_Phdr;
typedef Elf32_Dyn  Elf_Dyn;
typedef Elf32_Sym  Elf_Sym;
typedef Elf32_Rel  Elf_Rel;
typedef Elf32_Rela Elf_Rela;

#define ELFCLASS ELFCLASS32
#define ELFEM    EM_MIPS_RS3_LE

#define ELF_ST_BIND ELF32_ST_BIND
#define ELF_ST_TYPE ELF32_ST_TYPE

#define ELF_R_TYPE ELF32_R_TYPE
#define ELF_R_SYM  ELF32_R_SYM

#define R_NONE       R_MIPS_NONE
#define R_RELATIVE32 R_MIPS_REL_32
#define R_ABS32      R_MIPS_32

#endif
