#include <elf64.h>

typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Phdr Elf_Phdr;
typedef Elf64_Dyn  Elf_Dyn;
typedef Elf64_Sym  Elf_Sym;
typedef Elf64_Rel  Elf_Rel;
typedef Elf64_Rela Elf_Rela;

#define ELFCLASS ELFCLASS64
#define ELFEM    EM_X86_64

#define ELF_ST_BIND ELF64_ST_BIND
#define ELF_ST_TYPE ELF64_ST_TYPE

#define ELF_R_TYPE ELF64_R_TYPE
#define ELF_R_SYM  ELF64_R_SYM

#define R_NONE       R_X86_64_NONE
#define R_RELATIVE64 R_X86_64_RELATIVE
#define R_JMP_SLOT64 R_X86_64_JUMP_SLOT
#define R_GLOB_DAT64 R_X86_64_GLOB_DAT
#define R_ABS32      R_X86_64_32
#define R_ABS64      R_X86_64_64
