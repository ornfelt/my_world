#ifndef ELF32_H
#define ELF32_H

#include <types.h>
#include <elf.h>

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef struct
{
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct
{
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct
{
	Elf32_Word st_name;
	Elf32_Addr st_value;
	Elf32_Word st_size;
	unsigned char st_info;
	unsigned char st_other;
	Elf32_Half st_shndx;
} Elf32_Sym;

typedef struct
{
	Elf32_Addr r_offset;
	Elf32_Word r_info;
} Elf32_Rel;

typedef struct
{
	Elf32_Addr r_offset;
	Elf32_Word r_info;
	Elf32_Sword r_addend;
} Elf32_Rela;

#define ELF32_R_SYM(i)     ((i) >> 8)
#define ELF32_R_TYPE(i)    ((i) & 0xFF)
#define ELF32_R_INFO(s, t) (((s) << 8) + ((t) & 0xFF))

typedef struct
{
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;

typedef struct
{
	Elf32_Sword d_tag;
	union
	{
		Elf32_Word d_val;
		Elf32_Addr d_ptr;
	} d_un;
} Elf32_Dyn;

#define ELF32_ST_BIND(i)       ((i) >> 4)
#define ELF32_ST_TYPE(i)       ((i) & 0xF)
#define ELF32_ST_INFO(b, t)    (((b) << 4) + ((t) & 0xF))
#define ELF32_ST_VISIBILITY(o) ((o) & 0x3)

typedef struct
{
	Elf32_Half vd_version;
	Elf32_Half vd_flags;
	Elf32_Half vd_ndx;
	Elf32_Half vd_cnt;
	Elf32_Word vd_hash;
	Elf32_Word vd_aux;
	Elf32_Word vd_next;
} Elf32_Verdef;

typedef struct
{
	Elf32_Word vda_name;
	Elf32_Word vda_next;
} Elf32_Verdaux;

typedef struct
{
	Elf32_Half vn_version;
	Elf32_Half vn_cnt;
	Elf32_Word vn_file;
	Elf32_Word vn_aux;
	Elf32_Word vn_next;
} Elf32_Verneed;

typedef struct
{
	Elf32_Word vna_hash;
	Elf32_Half vna_flags;
	Elf32_Half vna_other;
	Elf32_Word vna_name;
	Elf32_Word vna_next;
} Elf32_Vernaux;

typedef struct
{
	Elf32_Word ch_type;
	Elf32_Word ch_size;
	Elf32_Word ch_addralign;
} Elf32_Chdr;

#endif
