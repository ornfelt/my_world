#ifndef LIBELF64_H
#define LIBELF64_H

#include <libelf.h>

typedef Elf64_Addr ElfN_Addr;
typedef Elf64_Half ElfN_Half;
typedef Elf64_Off ElfN_Off;
typedef Elf64_Sword ElfN_Sword;
typedef Elf64_Word ElfN_Word;

typedef Elf64_Ehdr ElfN_Ehdr;
typedef Elf64_Shdr ElfN_Shdr;
typedef Elf64_Sym ElfN_Sym;
typedef Elf64_Rel ElfN_Rel;
typedef Elf64_Rela ElfN_Rela;
typedef Elf64_Phdr ElfN_Phdr;
typedef Elf64_Dyn ElfN_Dyn;
typedef Elf64_Verdef ElfN_Verdef;
typedef Elf64_Verdaux ElfN_Verdaux;
typedef Elf64_Verneed ElfN_Verneed;
typedef Elf64_Vernaux ElfN_Vernaux;
typedef Elf64_Chdr ElfN_Chdr;

#define ELFCLASS ELFCLASS64

#define ELFN_R_SYM  ELF64_R_SYM
#define ELFN_R_TYPE ELF64_R_TYPE
#define ELFN_R_INFO ELF64_R_INFO

#define ELFN_ST_BIND       ELF64_ST_BIND
#define ELFN_ST_TYPE       ELF64_ST_TYPE
#define ELFN_ST_INFO       ELF64_ST_INFO
#define ELFN_ST_VISIBILITY ELF64_ST_VISIBILITY

#define elfN                elf64
#define elfN_symtab         elf64_symtab
#define elfN_readat         elf64_readat
#define elfN_load_phdr      elf64_load_phdr
#define elfN_load_shdr      elf64_load_shdr
#define elfN_load_shstr     elf64_load_shstr
#define elfN_load_dynstr    elf64_load_dynstr
#define elfN_load_dynsym    elf64_load_dynsym
#define elfN_load_dyns      elf64_load_dyns
#define elfN_open           elf64_open
#define elfN_open_fd        elf64_open_fd
#define elfN_free           elf64_free
#define elfN_read_section   elf64_read_section
#define elfN_get_ehdr       elf64_get_ehdr
#define elfN_get_shnum      elf64_get_shnum
#define elfN_get_shoff      elf64_get_shoff
#define elfN_get_shdr       elf64_get_shdr
#define elfN_get_phnum      elf64_get_phnum
#define elfN_get_phoff      elf64_get_phoff
#define elfN_get_phdr       elf64_get_phdr
#define elfN_get_dynnum     elf64_get_dynnum
#define elfN_get_dyn        elf64_get_dyn
#define elfN_get_dynid      elf64_get_dynid
#define elfN_get_dynsymnum  elf64_get_dynsymnum
#define elfN_get_dynsym     elf64_get_dynsym
#define elfN_get_shname     elf64_get_shname
#define elfN_get_dynstr_str elf64_get_dynstr_str
#define elfN_symtab_read    elf64_symtab_read
#define elfN_symtab_sym     elf64_symtab_sym
#define elfN_symtab_str     elf64_symtab_str
#define elfN_symtab_free    elf64_symtab_free

#endif
