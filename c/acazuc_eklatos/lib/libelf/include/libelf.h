#ifndef LIBELF_H
#define LIBELF_H

#include <stddef.h>
#include <elf32.h>
#include <elf64.h>

#ifdef __cplusplus
extern "C" {
#endif

struct elf32;
struct elf64;
struct elf32_symtab;
struct elf64_symtab;

const char *elf_ehdr_class_str(uint8_t v);
const char *elf_ehdr_data_str(uint8_t v);
const char *elf_ehdr_abi_str(uint8_t v);
const char *elf_ehdr_type_str(uint16_t v);
const char *elf_ehdr_machine_str(uint16_t v);
const char *elf_phdr_type_str(uint32_t v);
const char *elf_phdr_flags_str(uint8_t v);
const char *elf_shdr_type_str(uint32_t v);
const char *elf_shdr_flags_str(char *buf, size_t size, uint32_t v);
const char *elf_dyn_str(uint32_t v);
const char *elf_dyn_flags_str(char *buf, size_t size, uint32_t v);
const char *elf_dyn_flags1_str(char *buf, size_t size, uint32_t v);
const char *elf_stb_str(uint8_t v);
const char *elf_stt_str(uint8_t v);
const char *elf_stv_str(uint8_t v);
const char *elf_r_str(uint32_t v);
const char *elf_r_386_str(uint32_t v);
const char *elf_r_x86_64_str(uint32_t v);
const char *elf_r_arm_str(uint32_t v);
const char *elf_r_aarch64_str(uint32_t v);
const char *elf_r_riscv_str(uint32_t v);
const char *elf_r_mips_str(uint32_t v);
const char *elf_ver_flags_str(char *buf, size_t size, uint32_t v);

struct elf32 *elf32_open(const char *path);
struct elf32 *elf32_open_fd(int fd);
void elf32_free(struct elf32 *elf);

int elf32_read_section(struct elf32 *elf, const Elf32_Shdr *shdr,
                       void **data, Elf32_Word *size);

Elf32_Ehdr *elf32_get_ehdr(struct elf32 *elf);
Elf32_Half elf32_get_shnum(struct elf32 *elf);
Elf32_Off elf32_get_shoff(struct elf32 *elf);
Elf32_Shdr *elf32_get_shdr(struct elf32 *elf, Elf32_Half idx);
Elf32_Half elf32_get_phnum(struct elf32 *elf);
Elf32_Off elf32_get_phoff(struct elf32 *elf);
Elf32_Phdr *elf32_get_phdr(struct elf32 *elf, Elf32_Half idx);
Elf32_Word elf32_get_dynnum(struct elf32 *elf);
Elf32_Dyn *elf32_get_dyn(struct elf32 *elf, Elf32_Word idx);
Elf32_Dyn *elf32_get_dynid(struct elf32 *elf, Elf32_Sword id);
Elf32_Word elf32_get_dynsymnum(struct elf32 *elf);
Elf32_Sym *elf32_get_dynsym(struct elf32 *elf, Elf32_Word idx);

const char *elf32_get_shname(struct elf32 *elf, const Elf32_Shdr *shdr);
const char *elf32_get_dynstr_str(struct elf32 *elf, Elf32_Word val);

struct elf32_symtab *elf32_symtab_read(struct elf32 *elf, const Elf32_Shdr *shdr);
Elf32_Sym *elf32_symtab_sym(struct elf32_symtab *symtab, Elf32_Word value);
const char *elf32_symtab_str(struct elf32_symtab *symtab, const Elf32_Sym *sym);
void elf32_symtab_free(struct elf32_symtab *symtab);

struct elf64 *elf64_open(const char *path);
struct elf64 *elf64_open_fd(int fd);
void elf64_free(struct elf64 *elf);

int elf64_read_section(struct elf64 *elf, const Elf64_Shdr *shdr,
                       void **data, Elf64_Word *size);

Elf64_Ehdr *elf64_get_ehdr(struct elf64 *elf);
Elf64_Half elf64_get_shnum(struct elf64 *elf);
Elf64_Off elf64_get_shoff(struct elf64 *elf);
Elf64_Shdr *elf64_get_shdr(struct elf64 *elf, Elf64_Half idx);
Elf64_Half elf64_get_phnum(struct elf64 *elf);
Elf64_Off elf64_get_phoff(struct elf64 *elf);
Elf64_Phdr *elf64_get_phdr(struct elf64 *elf, Elf64_Half idx);
Elf64_Word elf64_get_dynnum(struct elf64 *elf);
Elf64_Dyn *elf64_get_dyn(struct elf64 *elf, Elf64_Word idx);
Elf64_Dyn *elf64_get_dynid(struct elf64 *elf, Elf64_Sword id);
Elf64_Word elf64_get_dynsymnum(struct elf64 *elf);
Elf64_Sym *elf64_get_dynsym(struct elf64 *elf, Elf64_Word idx);

const char *elf64_get_shname(struct elf64 *elf, const Elf64_Shdr *shdr);
const char *elf64_get_dynstr_str(struct elf64 *elf, Elf64_Word val);

struct elf64_symtab *elf64_symtab_read(struct elf64 *elf, const Elf64_Shdr *shdr);
Elf64_Sym *elf64_symtab_sym(struct elf64_symtab *symtab, Elf64_Word value);
const char *elf64_symtab_str(struct elf64_symtab *symtab, const Elf64_Sym *sym);
void elf64_symtab_free(struct elf64_symtab *symtab);

#ifdef __cplusplus
}
#endif

#endif
