#ifndef ELF64_H
#define ELF64_H

#include <types.h>
#include <elf.h>

typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Off;
typedef int32_t Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef int64_t Elf64_Sxword;
typedef uint64_t Elf64_Xword;

typedef struct
{
	unsigned char e_ident[EI_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
	Elf64_Word sh_name;
	Elf64_Word sh_type;
	Elf64_Xword sh_flags;
	Elf64_Addr sh_addr;
	Elf64_Off sh_offset;
	Elf64_Xword sh_size;
	Elf64_Word sh_link;
	Elf64_Word sh_info;
	Elf64_Xword sh_addralign;
	Elf64_Xword sh_entsize;
} Elf64_Shdr;

typedef struct
{
	Elf64_Word st_name;
	unsigned char st_info;
	unsigned char st_other;
	Elf64_Half st_shndx;
	Elf64_Addr st_value;
	Elf64_Xword st_size;
} Elf64_Sym;

typedef struct
{
	Elf64_Addr r_offset;
	Elf64_Xword r_info;
} Elf64_Rel;

typedef struct
{
	Elf64_Addr r_offset;
	Elf64_Xword r_info;
	Elf64_Sxword r_addend;
} Elf64_Rela;

#define ELF64_R_SYM(i)     ((i) >> 32)
#define ELF64_R_TYPE(i)    ((i) & 0xFFFFFFFFL)
#define ELF64_R_INFO(s, t) (((s) << 32) + ((t) & 0xFFFFFFFFL))

typedef struct
{
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf64_Phdr;

typedef struct
{
	Elf64_Sxword d_tag;
	union
	{
		Elf64_Xword d_val;
		Elf64_Addr d_ptr;
	} d_un;
} Elf64_Dyn;

#define ELF64_ST_BIND(i)       ((i) >> 4)
#define ELF64_ST_TYPE(i)       ((i) & 0xF)
#define ELF64_ST_INFO(b, t)    (((b) << 4) + ((t) & 0xF))
#define ELF64_ST_VISIBILITY(o) ((o) & 0x3)

typedef struct
{
	Elf64_Half vd_version;
	Elf64_Half vd_flags;
	Elf64_Half vd_ndx;
	Elf64_Half vd_cnt;
	Elf64_Word vd_hash;
	Elf64_Word vd_aux;
	Elf64_Word vd_next;
} Elf64_Verdef;

typedef struct
{
	Elf64_Word vda_name;
	Elf64_Word vda_next;
} Elf64_Verdaux;

typedef struct
{
	Elf64_Half vn_version;
	Elf64_Half vn_cnt;
	Elf64_Word vn_file;
	Elf64_Word vn_aux;
	Elf64_Word vn_next;
} Elf64_Verneed;

typedef struct
{
	Elf64_Word vna_hash;
	Elf64_Half vna_flags;
	Elf64_Half vna_other;
	Elf64_Word vna_name;
	Elf64_Word vna_next;
} Elf64_Vernaux;

typedef struct
{
	Elf64_Word ch_type;
	Elf64_Word ch_reserved;
	Elf64_Xword ch_size;
	Elf64_Xword ch_addralign;
} Elf64_Chdr;

#endif
