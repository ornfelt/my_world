#ifndef LIBELF32_H
#define LIBELF32_H

#include <libelf.h>

typedef Elf32_Addr ElfN_Addr;
typedef Elf32_Half ElfN_Half;
typedef Elf32_Off ElfN_Off;
typedef Elf32_Sword ElfN_Sword;
typedef Elf32_Word ElfN_Word;

typedef Elf32_Ehdr ElfN_Ehdr;
typedef Elf32_Shdr ElfN_Shdr;
typedef Elf32_Sym ElfN_Sym;
typedef Elf32_Rel ElfN_Rel;
typedef Elf32_Rela ElfN_Rela;
typedef Elf32_Phdr ElfN_Phdr;
typedef Elf32_Dyn ElfN_Dyn;
typedef Elf32_Verdef ElfN_Verdef;
typedef Elf32_Verdaux ElfN_Verdaux;
typedef Elf32_Verneed ElfN_Verneed;
typedef Elf32_Vernaux ElfN_Vernaux;
typedef Elf32_Chdr ElfN_Chdr;

#define ELFCLASS ELFCLASS32

#define ELFN_R_SYM  ELF32_R_SYM
#define ELFN_R_TYPE ELF32_R_TYPE
#define ELFN_R_INFO ELF32_R_INFO

#define ELFN_ST_BIND       ELF32_ST_BIND
#define ELFN_ST_TYPE       ELF32_ST_TYPE
#define ELFN_ST_INFO       ELF32_ST_INFO
#define ELFN_ST_VISIBILITY ELF32_ST_VISIBILITY

#define elfN                elf32
#define elfN_symtab         elf32_symtab
#define elfN_readat         elf32_readat
#define elfN_load_phdr      elf32_load_phdr
#define elfN_load_shdr      elf32_load_shdr
#define elfN_load_shstr     elf32_load_shstr
#define elfN_load_dynstr    elf32_load_dynstr
#define elfN_load_dynsym    elf32_load_dynsym
#define elfN_load_dyns      elf32_load_dyns
#define elfN_open           elf32_open
#define elfN_open_fd        elf32_open_fd
#define elfN_free           elf32_free
#define elfN_read_section   elf32_read_section
#define elfN_get_ehdr       elf32_get_ehdr
#define elfN_get_shnum      elf32_get_shnum
#define elfN_get_shoff      elf32_get_shoff
#define elfN_get_shdr       elf32_get_shdr
#define elfN_get_phnum      elf32_get_phnum
#define elfN_get_phoff      elf32_get_phoff
#define elfN_get_phdr       elf32_get_phdr
#define elfN_get_dynnum     elf32_get_dynnum
#define elfN_get_dyn        elf32_get_dyn
#define elfN_get_dynid      elf32_get_dynid
#define elfN_get_dynsymnum  elf32_get_dynsymnum
#define elfN_get_dynsym     elf32_get_dynsym
#define elfN_get_shname     elf32_get_shname
#define elfN_get_dynstr_str elf32_get_dynstr_str
#define elfN_symtab_read    elf32_symtab_read
#define elfN_symtab_sym     elf32_symtab_sym
#define elfN_symtab_str     elf32_symtab_str
#define elfN_symtab_free    elf32_symtab_free

#endif
