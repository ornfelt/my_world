#ifndef ELF_H
#define ELF_H

#ifdef __cplusplus
extern "C" {
#endif

#define EI_NIDENT 16

#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2
#define ET_DYN  3
#define ET_CORE 4

#define EM_NONE         0
#define EM_M32          1
#define EM_SPARC        2
#define EM_386          3
#define EM_68K          4
#define EM_88K          5
#define EM_860          7
#define EM_MIPS         8
#define EM_MIPS_RS3_LE  10
#define EM_ARM          40
#define EM_X86_64       62
#define EM_AARCH64      183
#define EM_RISCV        243

#define EV_NONE    0
#define EV_CURRENT 1

#define EI_MAG0       0
#define EI_MAG1       1
#define EI_MAG2       2
#define EI_MAG3       3
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_OSABI      7
#define EI_ABIVERSION 8
#define EI_PAD        9

#define ELFOSABI_NONE    0
#define ELFOSABI_HPUX    1
#define ELFOSABI_NETBSD  2
#define ELFOSABI_LINUX   3
#define ELFOSABI_SOLARIS 6
#define ELFOSABI_AIX     7
#define ELFOSABI_IRIX    8
#define ELFOSABI_FREEBSD 9
#define ELFOSABI_TRU64   10
#define ELFOSABI_MODESTO 11
#define ELFOSABI_OPENBSD 12
#define ELFOSABI_OPENVMS 13
#define ELFOSABI_NSK     14

#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define SHN_UNDEF     0
#define SHN_LORESERVE 0xFF00
#define SHN_LOPROC    0xFF00
#define SHN_HIPROC    0xFF1F
#define SHN_ABS       0xFFF1
#define SHN_COMMON    0xFFF2
#define SHN_HIRESERVE 0xFFFF

#define SHT_NULL             0
#define SHT_PROGBITS         1
#define SHT_SYMTAB           2
#define SHT_STRTAB           3
#define SHT_RELA             4
#define SHT_HASH             5
#define SHT_DYNAMIC          6
#define SHT_NOTE             7
#define SHT_NOBITS           8
#define SHT_REL              9
#define SHT_SHLIB            10
#define SHT_DYNSYM           11
#define SHT_INIT_ARRAY       14
#define SHT_FINI_ARRAY       15
#define SHT_GNU_HASH         0x6FFFFFF6
#define SHT_VERDEF           0x6FFFFFFD
#define SHT_VERNEED          0x6FFFFFFE
#define SHT_VERSYM           0x6FFFFFFF
#define SHT_LOPROC           0x70000000
#define SHT_RISCV_ATTRIBUTES 0x70000003
#define SHT_HIPROC           0x7FFFFFFF
#define SHT_LOUSER           0x80000000
#define SHT_HIUSER           0xFFFFFFFF

#define SHF_WRITE            0x001
#define SHF_ALLOC            0x002
#define SHF_EXECINSTR        0x004
#define SHF_MERGE            0x010
#define SHF_STRINGS          0x020
#define SHF_INFO_LINK        0x040
#define SHF_LINK_ORDER       0x080
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP            0x200
#define SHF_TLS              0x400
#define SHF_COMPRESSED       0x800
#define SHF_MASKPROC         0xF0000000

#define STN_UNDEF 0

#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2
#define STB_LOPROC 13
#define STB_HIPROC 15

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_COMMON  5
#define STT_TLS     6
#define STT_LOPROC  13
#define STT_HIPROC  15

#define STV_DEFAULT   0
#define STV_INTERNAL  1
#define STV_HIDDEN    2
#define STV_PROTECTED 3

#define PT_NULL             0
#define PT_LOAD             1
#define PT_DYNAMIC          2
#define PT_INTERP           3
#define PT_NOTE             4
#define PT_SHLIB            5
#define PT_PHDR             6
#define PT_TLS              7
#define PT_GNU_EH_FRAME     0x6474E550
#define PT_GNU_STACK        0x6474E551
#define PT_GNU_RELRO        0x6474E552
#define PT_LOPROC           0x70000000
#define PT_EXIDX            0x70000001
#define PT_RISCV_ATTRIBUTES 0x70000003
#define PT_HIPROC           0x7FFFFFFF

#define PF_X (1 << 0)
#define PF_W (1 << 1)
#define PF_R (1 << 2)

#define DT_NULL         0x0
#define DT_NEEDED       0x1
#define DT_PLTRELSZ     0x2
#define DT_PLTGOT       0x3
#define DT_HASH         0x4
#define DT_STRTAB       0x5
#define DT_SYMTAB       0x6
#define DT_RELA         0x7
#define DT_RELASZ       0x8
#define DT_RELAENT      0x9
#define DT_STRSZ        0xA
#define DT_SYMENT       0xB
#define DT_INIT         0xC
#define DT_FINI         0xD
#define DT_SONAME       0xE
#define DT_RPATH        0xF
#define DT_SYMBOLIC     0x10
#define DT_REL          0x11
#define DT_RELSZ        0x12
#define DT_RELENT       0x13
#define DT_PLTREL       0x14
#define DT_DEBUG        0x15
#define DT_TEXTREL      0x16
#define DT_JMPREL       0x17
#define DT_BIND_NOW     0x18
#define DT_INIT_ARRAY   0x19
#define DT_FINI_ARRAY   0x1A
#define DT_INIT_ARRAYSZ 0x1B
#define DT_FINI_ARRAYSZ 0x1C
#define DT_FLAGS        0x1E
#define DT_GNU_HASH     0x6FFFFEF5
#define DT_VERSYM       0x6FFFFFF0
#define DT_RELACOUNT    0x6FFFFFF9
#define DT_RELCOUNT     0x6FFFFFFA
#define DT_FLAGS_1      0x6FFFFFFB
#define DT_VERDEF       0x6FFFFFFC
#define DT_VERDEFNUM    0x6FFFFFFD
#define DT_VERNEED      0x6FFFFFFE
#define DT_VERNEEDNUM   0x6FFFFFFF
#define DT_LOPROC       0x70000000
#define DT_HIPROC       0x7FFFFFFF

#define DF_ORIGIN     (1 << 0)
#define DF_SYMBOLIC   (1 << 1)
#define DF_TEXTREL    (1 << 2)
#define DF_BIND_NOW   (1 << 3)
#define DF_STATIC_TLS (1 << 4)

#define DF_1_NOW        (1 << 0)
#define DF_1_GLOBAL     (1 << 1)
#define DF_1_GROUP      (1 << 2)
#define DF_1_NODELETE   (1 << 3)
#define DF_1_LOADFLTR   (1 << 4)
#define DF_1_INITFIRST  (1 << 5)
#define DF_1_NOOPEN     (1 << 6)
#define DF_1_ORIGIN     (1 << 7)
#define DF_1_DIRECT     (1 << 8)
#define DF_1_TRANS      (1 << 9)
#define DF_1_INTERPOSE  (1 << 10)
#define DF_1_NODEFLIB   (1 << 11)
#define DF_1_NODUMP     (1 << 12)
#define DF_1_CONFALT    (1 << 13)
#define DF_1_ENDFILTEE  (1 << 14)
#define DF_1_DISPRELDNE (1 << 15)
#define DF_1_DIPPRELPND (1 << 16)
#define DF_1_NODIRECT   (1 << 17)
#define DF_1_IGNMULDEF  (1 << 18)
#define DF_1_NOKSYMS    (1 << 19)
#define DF_1_NOHDR      (1 << 20)
#define DF_1_EDITED     (1 << 21)
#define DF_1_NORELOC    (1 << 22)
#define DF_1_SYMINTPOSE (1 << 23)
#define DF_1_GLOBAUDIT  (1 << 24)
#define DF_1_SINGLETON  (1 << 25)
#define DF_1_STUB       (1 << 26)
#define DF_1_PIE        (1 << 27)
#define DF_1_KMOD       (1 << 28)
#define DF_1_WEAKFILTER (1 << 29)
#define DF_1_NOCOMMON   (1 << 30)

#define R_386_NONE         0
#define R_386_32           1
#define R_386_PC32         2
#define R_386_GOT32        3
#define R_386_PLT32        4
#define R_386_COPY         5
#define R_386_GLOB_DAT     6
#define R_386_JMP_SLOT     7
#define R_386_RELATIVE     8
#define R_386_GOTOFF       9
#define R_386_GOTPC        10
#define R_386_32PLT        11
#define R_386_TLS_GD_PLT   12
#define R_386_TLS_LDM_PLT  13
#define R_386_TLS_TPOFF    14
#define R_386_TLS_IE       15
#define R_386_TLS_GOTIE    16
#define R_386_TLS_LE       17
#define R_386_TLS_GD       18
#define R_386_TLS_LDM      19
#define R_386_16           20
#define R_386_PC16         21
#define R_386_8            22
#define R_386_PC8          23
#define R_386_TLS_LDO_32   32
#define R_386_TLS_DTPMOD32 35
#define R_386_TLS_DTPOFF32 36
#define R_386_TLS_TPOFF32  37
#define R_386_SIZE32       38

#define R_ARM_NONE               0
#define R_ARM_PC24               1
#define R_ARM_ABS32              2
#define R_ARM_REL32              3
#define R_ARM_LDR_PC_G0          4
#define R_ARM_ABS16              5
#define R_ARM_ABS12              6
#define R_ARM_THM_ABS5           7
#define R_ARM_ABS8               8
#define R_ARM_SBREL32            9
#define R_ARM_THM_CALL           10
#define R_ARM_THM_PC8            11
#define R_ARM_BREL_ADJ           12
#define R_ARM_TLS_DESC           13
#define R_ARM_THM_SWI8           14
#define R_ARM_XPC25              15
#define R_ARM_THM_XPC22          16
#define R_ARM_TLS_DTPMOD32       17
#define R_ARM_TLS_DTPOFF32       18
#define R_ARM_TLS_TPOFF32        19
#define R_ARM_COPY               20
#define R_ARM_GLOB_DAT           21
#define R_ARM_JUMP_SLOT          22
#define R_ARM_RELATIVE           23
#define R_ARM_GOTOFF32           24
#define R_ARM_BASE_PREL          25
#define R_ARM_GOT_BREL           26
#define R_ARM_PLT32              27
#define R_ARM_CALL               28
#define R_ARM_JUMP24             29
#define R_ARM_THM_JUMP24         30
#define R_ARM_BASE_ABS           31
#define R_ARM_ALU_PCREL_7_0      32
#define R_ARM_ALU_PCREL_15_8     33
#define R_ARM_ALU_PCREL_23_15    34
#define R_ARM_LDR_SBREL_11_0_NC  35
#define R_ARM_ALU_SBREL_19_12_NC 36
#define R_ARM_ALU_SBREL_27_20_CK 37
#define R_ARM_TARGET1            38
#define R_ARM_SBREL31            39
#define R_ARM_V4BX               40
#define R_ARM_TARGET2            41
#define R_ARM_PREL31             42
#define R_ARM_MOVW_ABS_NC        43
#define R_ARM_MOVT_ABS           44
#define R_ARM_MOVW_PREL_NC       45
#define R_ARM_MOVT_PREL          46
#define R_ARM_THM_MOVW_ABS_NC    47
#define R_ARM_THM_MOVT_ABS       48
#define R_ARM_THM_MOVW_PREL_NC   49
#define R_ARM_THM_MOVT_PREL      50
#define R_ARM_THM_JUMP19         51
#define R_ARM_THM_JUMP6          52
#define R_ARM_THM_ALU_PREL_11_0  53
#define R_ARM_THM_PC12           54
#define R_ARM_ABS32_NOI          55
#define R_ARM_REL32_NOI          56
#define R_ARM_ALU_PC_G0_NC       57
#define R_ARM_ALU_PC_G0          58
#define R_ARM_ALU_PC_G1_NC       59
#define R_ARM_ALU_PC_G1          60
#define R_ARM_ALU_PC_G2          61
#define R_ARM_LDR_PC_G1          62
#define R_ARM_LDR_PC_G2          63
#define R_ARM_LDRS_PC_G0         64
#define R_ARM_LDRS_PC_G1         65
#define R_ARM_LDRS_PC_G2         66
#define R_ARM_LDC_PC_G0          67
#define R_ARM_LDC_PC_G1          68
#define R_ARM_LDC_PC_G2          69
#define R_ARM_ALU_SB_G0_NC       70
#define R_ARM_ALU_SB_G0          71
#define R_ARM_ALU_SB_G1_NC       72
#define R_ARM_ALU_SB_G1          73
#define R_ARM_ALU_SB_G2          74
#define R_ARM_LDR_SB_G0          75
#define R_ARM_LDR_SB_G1          76
#define R_ARM_LDR_SB_G2          77
#define R_ARM_LDRS_SB_G0         78
#define R_ARM_LDRS_SB_G1         79
#define R_ARM_LDRS_SB_G2         80
#define R_ARM_LDC_SB_G0          81
#define R_ARM_LDC_SB_G1          82
#define R_ARM_LDC_SB_G2          83
#define R_ARM_MOVW_BREL_NC       84
#define R_ARM_MOVT_BREL          85
#define R_ARM_MOVW_BREL          86
#define R_ARM_THM_MOVW_BREL_NC   87
#define R_ARM_THM_MOVT_BREL      88
#define R_ARM_THM_MOVW_BREL      89
#define R_ARM_TLS_GOTDESC        90
#define R_ARM_TLS_CALL           91
#define R_ARM_TLS_DESCSEQ        92
#define R_ARM_THM_TLS_CALL       93
#define R_ARM_PLT32_ABS          94
#define R_ARM_GOT_ABS            95
#define R_ARM_GOT_PREL           96
#define R_ARM_GOT_BREL12         97
#define R_ARM_GOTOFF12           98
#define R_ARM_GOTRELAX           99
#define R_ARM_GNU_VTENTRY        100
#define R_ARM_GNU_VTINHERIT      101
#define R_ARM_THM_JUMP11         102
#define R_ARM_THM_JUMP8          103
#define R_ARM_TLS_GD32           104
#define R_ARM_TLS_LDM32          105
#define R_ARM_TLS_LDO32          106
#define R_ARM_TLS_IE32           107
#define R_ARM_TLS_LE32           108
#define R_ARM_TLS_LDO12          109
#define R_ARM_TLS_LE12           110
#define R_ARM_TLS_IE12GP         111
#define R_ARM_PRIVATE_0          112
#define R_ARM_PRIVATE_1          113
#define R_ARM_PRIVATE_2          114
#define R_ARM_PRIVATE_3          115
#define R_ARM_PRIVATE_4          116
#define R_ARM_PRIVATE_5          117
#define R_ARM_PRIVATE_6          118
#define R_ARM_PRIVATE_7          119
#define R_ARM_PRIVATE_8          120
#define R_ARM_PRIVATE_9          121
#define R_ARM_PRIVATE_10         122
#define R_ARM_PRIVATE_11         123
#define R_ARM_PRIVATE_12         124
#define R_ARM_PRIVATE_13         125
#define R_ARM_PRIVATE_14         126
#define R_ARM_PRIVATE_15         127
#define R_ARM_ME_TOO             128
#define R_ARM_THM_TLS_DESCSEQ16  129
#define R_ARM_THM_TLS_DESCSEQ32  130
#define R_ARM_THM_GOT_BREL12     131
#define R_ARM_THM_ALU_ABS_G0_NC  132
#define R_ARM_THM_ALU_ABS_G1_NC  133
#define R_ARM_THM_ALU_ABS_G2_NC  134
#define R_ARM_THM_ALU_ABS_G3     135
#define R_ARM_THM_BF16           136
#define R_ARM_THM_BF12           137
#define R_ARM_THM_BF18           138
#define R_ARM_IRELATIVE          160
#define R_ARM_PRIVATE_16         161
#define R_ARM_PRIVATE_17         162
#define R_ARM_PRIVATE_18         163
#define R_ARM_PRIVATE_19         164
#define R_ARM_PRIVATE_20         165
#define R_ARM_PRIVATE_21         166
#define R_ARM_PRIVATE_22         167
#define R_ARM_PRIVATE_23         168
#define R_ARM_PRIVATE_24         169
#define R_ARM_PRIVATE_25         170
#define R_ARM_PRIVATE_26         171
#define R_ARM_PRIVATE_27         172
#define R_ARM_PRIVATE_28         173
#define R_ARM_PRIVATE_29         174
#define R_ARM_PRIVATE_30         175
#define R_ARM_PRIVATE_31         176

#define R_X86_64_NONE            0
#define R_X86_64_64              1
#define R_X86_64_PC32            2
#define R_X86_64_GOT32           3
#define R_X86_64_PLT32           4
#define R_X86_64_COPY            5
#define R_X86_64_GLOB_DAT        6
#define R_X86_64_JUMP_SLOT       7
#define R_X86_64_RELATIVE        8
#define R_X86_64_GOTPCREL        9
#define R_X86_64_32              10
#define R_X86_64_32S             11
#define R_X86_64_16              12
#define R_X86_64_PC16            13
#define R_X86_64_8               14
#define R_X86_64_PC8             15
#define R_X86_64_DTPMOD64        16
#define R_X86_64_DTPOFF64        17
#define R_X86_64_TPOFF64         18
#define R_X86_64_TLSGD           19
#define R_X86_64_TLSLD           20
#define R_X86_64_DTPOFF32        21
#define R_X86_64_GOTTPOFF        22
#define R_X86_64_TPOFF32         23
#define R_X86_64_PC64            24
#define R_X86_64_GOTOFF64        25
#define R_X86_64_GOTPC32         26
#define R_X86_64_SIZE32          32
#define R_X86_64_SIZE64          33
#define R_X86_64_GOTPC32_TLSDESC 34
#define R_X86_64_TLSDESC_CALL    35
#define R_X86_64_TLSDESC         36
#define R_X86_64_IRELATIVE       37
#define R_X86_64_RELATIVE64      38
#define R_X86_64_GOTPCRELX       41
#define R_X86_64_REX_GOTPCRELX   42

#define R_AARCH64_NONE                         0
#define R_AARCH64_ABS64                        257
#define R_AARCH64_ABS32                        258
#define R_AARCH64_ABS16                        259
#define R_AARCH64_PREL64                       260
#define R_AARCH64_PREL32                       261
#define R_AARCH64_PREL16                       262
#define R_AARCH64_MOVW_UABS_G0                 263
#define R_AARCH64_MOVW_UABS_G0_NC              264
#define R_AARCH64_MOVW_UABS_G1                 265
#define R_AARCH64_MOVW_UABS_G1_NC              266
#define R_AARCH64_MOVW_UABS_G2                 267
#define R_AARCH64_MOVW_UABS_G2_NC              268
#define R_AARCH64_MOVW_UABS_G3                 269
#define R_AARCH64_MOVW_SABS_G0                 270
#define R_AARCH64_MOVW_SABS_G1                 271
#define R_AARCH64_MOVW_SABS_G2                 272
#define R_AARCH64_LD_PREL_LO19                 273
#define R_AARCH64_ADR_PREL_LO21                274
#define R_AARCH64_ADR_PREL_PG_HI21             275
#define R_AARCH64_ADR_PREL_PG_HI21_NC          276
#define R_AARCH64_ADD_ABS_LO12_NC              277
#define R_AARCH64_LDST8_ABS_LO12_NC            278
#define R_AARCH64_TSTBR14                      279
#define R_AARCH64_CONDBR19                     280
#define R_AARCH64_JUMP26                       282
#define R_AARCH64_CALL26                       283
#define R_AARCH64_LDST16_ABS_LO12_NC           284
#define R_AARCH64_LDST32_ABS_LO12_NC           285
#define R_AARCH64_LDST64_ABS_LO12_NC           286
#define R_AARCH64_MOVW_PREL_G0                 287
#define R_AARCH64_MOVW_PREL_G0_NC              288
#define R_AARCH64_MOVW_PREL_G1                 289
#define R_AARCH64_MOVW_PREL_G1_NC              290
#define R_AARCH64_MOVW_PREL_G2                 291
#define R_AARCH64_MOVW_PREL_G2_NC              292
#define R_AARCH64_MOVW_PREL_G3                 293
#define R_AARCH64_LDST128_ABS_LO12_NC          299
#define R_AARCH64_MOVW_GOTOFF_G0               300
#define R_AARCH64_MOVW_GOTOFF_G0_NC            301
#define R_AARCH64_MOVW_GOTOFF_G1               302
#define R_AARCH64_MOVW_GOTOFF_G1_NC            303
#define R_AARCH64_MOVW_GOTOFF_G2               304
#define R_AARCH64_MOVW_GOTOFF_G2_NC            305
#define R_AARCH64_MOVW_GOTOFF_G3               306
#define R_AARCH64_GOTREL64                     307
#define R_AARCH64_GOTREL32                     308
#define R_AARCH64_GOT_LD_PREL19                309
#define R_AARCH64_LD64_GOTOFF_LO15             310
#define R_AARCH64_ADR_GOT_PAGE                 311
#define R_AARCH64_LD64_GOT_LO12_NC             312
#define R_AARCH64_LD64_GOTPAGE_LO15            313
#define R_AARCH64_PLT32                        314
#define R_AARCH64_GOTPREL32                    315
#define R_AARCH64_TLSGD_ADR_PREL21             512
#define R_AARCH64_TLSGD_ADR_PAGE21             513
#define R_AARCH64_TLSGD_ADD_LO12_NC            514
#define R_AARCH64_TLSGD_MOVW_G1                515
#define R_AARCH64_TLSGD_MOVW_G0_NC             516
#define R_AARCH64_TLSLD_ADR_PREL21             517
#define R_AARCH64_TLSLD_ADR_PAGE21             518
#define R_AARCH64_TLSLD_ADD_LO12_NC            519
#define R_AARCH64_TLSLD_MOVW_G1                520
#define R_AARCH64_TLSLD_MOVW_G0_NC             521
#define R_AARCH64_TLSLD_LD_PREL19              522
#define R_AARCH64_TLSLD_MOVW_DTPREL_G2         523
#define R_AARCH64_TLSLD_MOVW_DTPREL_G1         524
#define R_AARCH64_TLSLD_MOVW_DTPRLE_G1_NC      525
#define R_AARCH64_TLSLD_MOVW_DTPRLE_G0         526
#define R_AARCH64_TLSLD_MOVW_DTPRLE_G0_NC      527
#define R_AARCH64_TLSLD_ADD_DTPREL_HI12        528
#define R_AARCH64_TLSLD_ADD_DTPREL_LO12        529
#define R_AARCH64_TLSLD_ADD_DTPREL_LO12_NC     530
#define R_AARCH64_TLSLD_LDST8_DTPREL_LO12      531
#define R_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC   532
#define R_AARCH64_TLSLD_LDST16_DTPREL_LO12     533
#define R_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC  534
#define R_AARCH64_TLSLD_LDST32_DTPREL_LO12     535
#define R_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC  536
#define R_AARCH64_TLSLD_LDST64_DTPREL_LO12     537
#define R_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC  538
#define R_AARCH64_TLSIE_MOVW_GOTTPREL_G1       539
#define R_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC    540
#define R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21    541
#define R_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC  542
#define R_AARCH64_TLSIE_LD_GOTTPREL_PREL19     543
#define R_AARCH64_TLSLE_MOVW_TPREL_G2          544
#define R_AARCH64_TLSLE_MOVW_TPREL_G1          545
#define R_AARCH64_TLSLE_MOVW_TPREL_G1_NC       546
#define R_AARCH64_TLSLE_MOVW_TPREL_G0          547
#define R_AARCH64_TLSLE_MOVW_TPREL_G0_NC       548
#define R_AARCH64_TLSLE_ADD_TPREL_HI12         549
#define R_AARCH64_TLSLE_ADD_TPREL_LO12         550
#define R_AARCH64_TLSLE_ADD_TPREL_LO12_NC      551
#define R_AARCH64_TLSLE_LDST8_TPREL_LO12       552
#define R_AARCH64_TLSLE_LDST8_TPREL_LO12_NC    553
#define R_AARCH64_TLSLE_LDST16_TPREL_LO12      554
#define R_AARCH64_TLSLE_LDST16_TPREL_LO12_NC   555
#define R_AARCH64_TLSLE_LDST32_TPREL_LO12      556
#define R_AARCH64_TLSLE_LDST32_TPREL_LO12_NC   557
#define R_AARCH64_TLSLE_LDST64_TPREL_LO12      558
#define R_AARCH64_TLSLE_LDST64_TPREL_LO12_NC   559
#define R_AARCH64_TLSDESC_LD_PREL19            560
#define R_AARCH64_TLSDESC_ADR_PREL21           561
#define R_AARCH64_TLSDSEC_ADR_PAGE21           562
#define R_AARCH64_TLSDESC_LD64_LO12            563
#define R_AARCH64_TLSDESC_ADD_LO12             564
#define R_AARCH64_TLSDESC_OFF_G1               565
#define R_AARCH64_TLSDESC_OFF_G0_NC            566
#define R_AARCH64_TLSDESC_LDR                  567
#define R_AARCH64_TLSDESC_ADD                  568
#define R_AARCH64_TLSDESC_CALL                 569
#define R_AARCH64_TLSLE_LDST128_TPREL_LO12     570
#define R_AARCH64_TLSLE_LDST128_TPREL_LO12_NC  571
#define R_AARCH64_TLSLD_LDST128_DTPREL_LO12    572
#define R_AARCH64_TLSLD_LDST128_DTPREL_LO12_NC 573
#define R_AARCH64_AUTH_ABS64                   580
#define R_AARCH64_COPY                         1024
#define R_AARCH64_GLOB_DAT                     1025
#define R_AARCH64_JUMP_SLOT                    1026
#define R_AARCH64_RELATIVE                     1027
#define R_AARCH64_TLS_DTPMOD                   1028
#define R_AARCH64_TLS_DTPREL                   1029
#define R_AARCH64_TLS_TPREL                    1030
#define R_AARCH64_TLSDESC                      1031
#define R_AARCH64_IRELATIVE                    1032
#define R_AARCH64_AUTH_RELATIVE                1041

#define R_RISCV_NONE              0
#define R_RISCV_32                1
#define R_RISCV_64                2
#define R_RISCV_RELATIVE          3
#define R_RISCV_COPY              4
#define R_RISCV_JUMP_SLOT         5
#define R_RISCV_TLS_DTPMOD32      6
#define R_RISCV_TLS_DTPMOD64      7
#define R_RISCV_TLS_DTPREL32      8
#define R_RISCV_TLS_DTPREL64      9
#define R_RISCV_TLS_TPREL32       10
#define R_RISCV_TLS_TPREL64       11
#define R_RISCV_TLS_TLSDESC       12
#define R_RISCV_BRANCH            16
#define R_RISCV_JAL               17
#define R_RISCV_CALL              18
#define R_RISCV_CALL_PLT          19
#define R_RISCV_GOT_HI20          20
#define R_RISCV_TLS_GOT_HI20      21
#define R_RISCV_TLS_GD_HI20       22
#define R_RISCV_PCREL_HI20        23
#define R_RISCV_PCREL_LO12_I      24
#define R_RISCV_PCREL_LO12_S      25
#define R_RISCV_HI20              26
#define R_RISCV_LO20_I            27
#define R_RISCV_LO12_S            28
#define R_RISCV_TPREL_HI20        29
#define R_RISCV_TPREL_LO12_I      30
#define R_RISCV_TPREL_LO12_S      31
#define R_RISCV_TPREL_ADD         32
#define R_RISCV_ADD8              33
#define R_RISCV_ADD16             34
#define R_RISCV_ADD32             35
#define R_RISCV_ADD64             36
#define R_RISCV_SUB8              37
#define R_RISCV_SUB16             38
#define R_RISCV_SUB32             39
#define R_RISCV_SUB64             40
#define R_RISCV_GOT32_PCREL       41
#define R_RISCV_ALIGN             43
#define R_RISCV_RVC_BRANCH        44
#define R_RISCV_RVC_JUMP          45
#define R_RISCV_RELAX             51
#define R_RISCV_SUB6              52
#define R_RISCV_SET6              53
#define R_RISCV_SET8              54
#define R_RISCV_SET16             55
#define R_RISCV_SET32             56
#define R_RISCV_32_PCREL          57
#define R_RISCV_IRELATIVE         58
#define R_RISCV_PLT32             59
#define R_RISCV_SET_ULEB128       60
#define R_RISCV_SUB_ULEB128       61
#define R_RISCV_TLSDESC_HI20      62
#define R_RISCV_TLSDESC_LOAD_LO12 63
#define R_RISCV_TLSDEC_ADD_LO12   64
#define R_RISCV_TLSDESC_CALL      65

#define R_MIPS_NONE            0
#define R_MIPS_16              1
#define R_MIPS_32              2
#define R_MIPS_REL_32          3
#define R_MIPS_26              4
#define R_MIPS_HI16            5
#define R_MIPS_LO16            6
#define R_MIPS_GPREL16         7
#define R_MIPS_LITERAL         8
#define R_MIPS_GOT16           9
#define R_MIPS_PC16            10
#define R_MIPS_CALL16          11
#define R_MIPS_GPREL32         12
#define R_MIPS_SHIFT5          16
#define R_MIPS_SHIFT6          17
#define R_MIPS_64              18
#define R_MIPS_GOT_DISP        19
#define R_MIPS_GOT_PAGE        20
#define R_MIPS_GOT_OFST        21
#define R_MIPS_GOT_HI16        22
#define R_MIPS_GOT_LO16        23
#define R_MIPS_SUB             24
#define R_MIPS_INSERT_A        25
#define R_MIPS_INSERT_B        26
#define R_MIPS_DELETE          27
#define R_MIPS_HIGHER          28
#define R_MIPS_HIGHEST         29
#define R_MIPS_CALL_HI16       30
#define R_MIPS_CALL_LO16       31
#define R_MIPS_SCN_DISP        32
#define R_MIPS_REL16           33
#define R_MIPS_ADD_IMMEDIATE   34
#define R_MIPS_PJUMP           35
#define R_MIPS_RELGOT          36
#define R_MIPS_JALR            37
#define R_MIPS_TLS_DTPMOD32    38
#define R_MIPS_TLS_DTPREL32    39
#define R_MIPS_TLS_DTPMOD64    40
#define R_MIPS_TLS_DTPREL64    41
#define R_MIPS_TLS_GD          42
#define R_MIPS_TLS_LDM         43
#define R_MIPS_TLS_DTPREL_HI16 44
#define R_MIPS_TLS_DTPREL_LO16 45
#define R_MIPS_TLS_GOTTPREL    46
#define R_MIPS_TLS_TPREL32     47
#define R_MIPS_TLS_TPREL64     48
#define R_MIPS_TLS_TPREL_HI16  49
#define R_MIPS_TLS_TPREL_LO16  50
#define R_MIPS_GLOB_DAT        51
#define R_MIPS_PC10            52
#define R_MIPS_COPY            126
#define R_MIPS_JUMP_SLOT       127
#define R_MIPS_PC32            238
#define R_MIPS_EH              249

#define VER_DEF_NONE    0
#define VER_DEF_CURRENT 1

#define VER_FLG_BASE 1
#define VER_FLG_WEAK 2

#define VER_NDX_LOCAL  0
#define VER_NDX_GLOBAL 1

#define VER_NEED_NONE    0
#define VER_NEED_CURRENT 1

#define ELFCOMPRESS_ZLIB 1
#define ELFCOMPRESS_ZSTD 2

#ifdef __cplusplus
}
#endif

#endif
