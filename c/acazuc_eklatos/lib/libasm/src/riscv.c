#include <libasm/riscv.h>

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#define FUNCT3(opcode)  ((opcode >> 12) & 0x007)
#define FUNCT5(opcode)  ((opcode >> 27) & 0x01F)
#define FUNCT7(opcode)  ((opcode >> 25) & 0x03F)
#define FUNCT12(opcode) ((opcode >> 20) & 0xFFF)
#define FMT(opcode)     ((opcode >> 25) & 0x003)
#define RM(opcode)      ((opcode >> 12) & 0x007)

#define RD(opcode)  ((opcode >>  7) & 0x1F)
#define RS1(opcode) ((opcode >> 15) & 0x1F)
#define RS2(opcode) ((opcode >> 20) & 0x1F)
#define RS3(opcode) ((opcode >> 27) & 0x1F)

#define CFUNCT2(opcode) ((opcode >>  5) & 0x03)
#define CFUNCT3(opcode) ((opcode >> 13) & 0x07)
#define CFUNCT4(opcode) ((opcode >> 12) & 0x0F)
#define CFUNCT6(opcode) ((opcode >> 10) & 0x3F)

#define CRD(opcode)    ((opcode >> 7) & 0x1F)
#define CRS2(opcode)   ((opcode >> 2) & 0x1F)
#define CRDP(opcode)  (((opcode >> 2) & 0x03) + 8)
#define CRS1P(opcode) (((opcode >> 7) & 0x03) + 8)

static const struct asm_riscv_opcode opcode_srai       = {"srai",       ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_srli       = {"srli",       ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_slli       = {"slli",       ASM_RISCV_ENCODING_SLLI,   ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_addi       = {"addi",       ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_slti       = {"slti",       ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sltiu      = {"sltiu",      ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_xori       = {"xori",       ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_ori        = {"ori",        ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_andi       = {"andi",       ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_add        = {"add",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sub        = {"sub",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_srl        = {"srl",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sra        = {"sra",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sll        = {"sll",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_slt        = {"slt",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sltu       = {"sltu",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_xor        = {"xor",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_or         = {"or",         ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_and        = {"and",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_auipc      = {"auipc",      ASM_RISCV_ENCODING_U,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_lui        = {"lui",        ASM_RISCV_ENCODING_U,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_jalr       = {"jalr",       ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_jal        = {"jal",        ASM_RISCV_ENCODING_J,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_lb         = {"lb",         ASM_RISCV_ENCODING_L,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_lh         = {"lh",         ASM_RISCV_ENCODING_L,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_lw         = {"lw",         ASM_RISCV_ENCODING_L,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_lbu        = {"lbu",        ASM_RISCV_ENCODING_L,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_lhu        = {"lhu",        ASM_RISCV_ENCODING_L,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sb         = {"sb",         ASM_RISCV_ENCODING_S,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sh         = {"sh",         ASM_RISCV_ENCODING_S,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sw         = {"sw",         ASM_RISCV_ENCODING_S,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_beq        = {"beq",        ASM_RISCV_ENCODING_B,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_bne        = {"bne",        ASM_RISCV_ENCODING_B,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_blt        = {"blt",        ASM_RISCV_ENCODING_B,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_bge        = {"bge",        ASM_RISCV_ENCODING_B,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_bltu       = {"bltu",       ASM_RISCV_ENCODING_B,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_bgeu       = {"bgeu",       ASM_RISCV_ENCODING_B,      ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_ecall      = {"ecall",      ASM_RISCV_ENCODING_NONE,   ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_ebreak     = {"ebreak",     ASM_RISCV_ENCODING_NONE,   ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sret       = {"sret",       ASM_RISCV_ENCODING_NONE,   ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_mret       = {"mret",       ASM_RISCV_ENCODING_NONE,   ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_wfi        = {"wfi",        ASM_RISCV_ENCODING_NONE,   ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_fence      = {"fence",      ASM_RISCV_ENCODING_FENCE,  ASM_RISCV_EXT_32I};
static const struct asm_riscv_opcode opcode_sfence_vma = {"sfence.vma", ASM_RISCV_ENCODING_SFENCE_VMA, ASM_RISCV_EXT_32I};

static const struct asm_riscv_opcode opcode_addiw      = {"addiw",      ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_slliw      = {"slliw",      ASM_RISCV_ENCODING_SLLI,   ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_srliw      = {"srliw",      ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_sraiw      = {"sraiw",      ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_addw       = {"addw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_subw       = {"subw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_sllw       = {"sllw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_srlw       = {"srlw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_sraw       = {"sraw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_ld         = {"ld",         ASM_RISCV_ENCODING_L,      ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_lwu        = {"lwu",        ASM_RISCV_ENCODING_L,      ASM_RISCV_EXT_64I};
static const struct asm_riscv_opcode opcode_sd         = {"sd",         ASM_RISCV_ENCODING_S,      ASM_RISCV_EXT_64I};

static const struct asm_riscv_opcode opcode_fence_i    = {"fence.i",    ASM_RISCV_ENCODING_NONE,   ASM_RISCV_EXT_ZIFENCEI};

static const struct asm_riscv_opcode opcode_csrrw      = {"csrrw",      ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_ZICSR};
static const struct asm_riscv_opcode opcode_csrrs      = {"csrrs",      ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_ZICSR};
static const struct asm_riscv_opcode opcode_csrrc      = {"csrrc",      ASM_RISCV_ENCODING_I,      ASM_RISCV_EXT_ZICSR};
static const struct asm_riscv_opcode opcode_csrrwi     = {"csrrwi",     ASM_RISCV_ENCODING_CSRRXI, ASM_RISCV_EXT_ZICSR};
static const struct asm_riscv_opcode opcode_csrrsi     = {"csrrsi",     ASM_RISCV_ENCODING_CSRRXI, ASM_RISCV_EXT_ZICSR};
static const struct asm_riscv_opcode opcode_csrrci     = {"csrrci",     ASM_RISCV_ENCODING_CSRRXI, ASM_RISCV_EXT_ZICSR};

static const struct asm_riscv_opcode opcode_mul        = {"mul",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32M};
static const struct asm_riscv_opcode opcode_mulh       = {"mulh",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32M};
static const struct asm_riscv_opcode opcode_mulhu      = {"mulhu",      ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32M};
static const struct asm_riscv_opcode opcode_mulhsu     = {"mulhsu",     ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32M};
static const struct asm_riscv_opcode opcode_div        = {"div",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32M};
static const struct asm_riscv_opcode opcode_divu       = {"divu",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32M};
static const struct asm_riscv_opcode opcode_rem        = {"rem",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32M};
static const struct asm_riscv_opcode opcode_remu       = {"remu",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_32M};

static const struct asm_riscv_opcode opcode_mulw       = {"mulw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64M};
static const struct asm_riscv_opcode opcode_divw       = {"divw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64M};
static const struct asm_riscv_opcode opcode_divuw      = {"divuw",      ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64M};
static const struct asm_riscv_opcode opcode_remw       = {"remw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64M};
static const struct asm_riscv_opcode opcode_remuw      = {"remuw",      ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_64M};

static const struct asm_riscv_opcode opcode_amoadd_w   = {"amoadd_w",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_amoswap_w  = {"amoswap_w",  ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_lr_w       = {"lr_w",       ASM_RISCV_ENCODING_AMO_LR, ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_sc_w       = {"sc_w",       ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_amoxor_w   = {"amoxor_w",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_amoor_w    = {"amoor_w",    ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_amoand_w   = {"amoand_w",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_amomin_w   = {"amomin_w",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_amomax_w   = {"amomax_w",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_amominu_w  = {"amominu_w",  ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};
static const struct asm_riscv_opcode opcode_amomaxu_w  = {"amomaxu_w",  ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_32A};

static const struct asm_riscv_opcode opcode_amoadd_d   = {"amoadd_d",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_amoswap_d  = {"amoswap_d",  ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_lr_d       = {"lr_d",       ASM_RISCV_ENCODING_AMO_LR, ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_sc_d       = {"sc_d",       ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_amoxor_d   = {"amoxor_d",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_amoor_d    = {"amoor_d",    ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_amoand_d   = {"amoand_d",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_amomin_d   = {"amomin_d",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_amomax_d   = {"amomax_d",   ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_amominu_d  = {"amominu_d",  ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};
static const struct asm_riscv_opcode opcode_amomaxu_d  = {"amomaxu_d",  ASM_RISCV_ENCODING_AMO,    ASM_RISCV_EXT_64A};

static const struct asm_riscv_opcode opcode_uret       = {"uret",       ASM_RISCV_ENCODING_NONE,   ASM_RISCV_EXT_N};

static const struct asm_riscv_opcode opcode_fmadd_s    = {"fmadd.s",    ASM_RISCV_ENCODING_FR3,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fmsub_s    = {"fmsub.s",    ASM_RISCV_ENCODING_FR3,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fnmadd_s   = {"fnmadd.s",   ASM_RISCV_ENCODING_FR3,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fnmsub_s   = {"fnmsub.s",   ASM_RISCV_ENCODING_FR3,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fadd_s     = {"fadd.s",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fsub_s     = {"fsub.s",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fmul_s     = {"fmul.s",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fdiv_s     = {"fdiv.s",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fsqrt_s    = {"fsqrt.s",    ASM_RISCV_ENCODING_FR1,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fsgnj_s    = {"fsgnj.s",    ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fsgnjn_s   = {"fsgnjn.s",   ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fsgnjx_s   = {"fsgnjx.s",   ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fmin_s     = {"fmin.s",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fmax_s     = {"fmax.s",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fcvt_w_s   = {"fcvt.w.s",   ASM_RISCV_ENCODING_FRF,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fcvt_s_w   = {"fcvt.s.w",   ASM_RISCV_ENCODING_FFR,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fcvt_wu_s  = {"fcvt.wu.s",  ASM_RISCV_ENCODING_FRF,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fcvt_s_wu  = {"fcvt.s.wu",  ASM_RISCV_ENCODING_FFR,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fmv_x_w    = {"fmv.x.w",    ASM_RISCV_ENCODING_FRF,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fmv_w_x    = {"fmv.w.x",    ASM_RISCV_ENCODING_FFR,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_feq_s      = {"feq.s",      ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_flt_s      = {"flt.s",      ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fle_s      = {"fle.s",      ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fclass_s   = {"fclass.s",   ASM_RISCV_ENCODING_FR1,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_flw        = {"flw",        ASM_RISCV_ENCODING_FLD,    ASM_RISCV_EXT_F};
static const struct asm_riscv_opcode opcode_fsw        = {"fsw",        ASM_RISCV_ENCODING_FST,    ASM_RISCV_EXT_F};

static const struct asm_riscv_opcode opcode_fmadd_d    = {"fmadd.d",    ASM_RISCV_ENCODING_FR3,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fmsub_d    = {"fmsub.d",    ASM_RISCV_ENCODING_FR3,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fnmadd_d   = {"fnmadd.d",   ASM_RISCV_ENCODING_FR3,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fnmsub_d   = {"fnmsub.d",   ASM_RISCV_ENCODING_FR3,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fadd_d     = {"fadd.d",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fsub_d     = {"fsub.d",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fmul_d     = {"fmul.d",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fdiv_d     = {"fdiv.d",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fsqrt_d    = {"fsqrt.d",    ASM_RISCV_ENCODING_FR1,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fsgnj_d    = {"fsgnj.d",    ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fsgnjn_d   = {"fsgnjn.d",   ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fsgnjx_d   = {"fsgnjx.d",   ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fmin_d     = {"fmin.d",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fmax_d     = {"fmax.d",     ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fcvt_w_d   = {"fcvt.w.d",   ASM_RISCV_ENCODING_FRF,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fcvt_d_w   = {"fcvt.d.w",   ASM_RISCV_ENCODING_FFR,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fcvt_s_d   = {"fcvt.s.d",   ASM_RISCV_ENCODING_FRF,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fcvt_d_s   = {"fcvt.d.s",   ASM_RISCV_ENCODING_FFR,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fcvt_wu_d  = {"fcvt.wu.d",  ASM_RISCV_ENCODING_FRF,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fcvt_d_wu  = {"fcvt.d.wu",  ASM_RISCV_ENCODING_FFR,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fmv_x_d    = {"fmv.x.d",    ASM_RISCV_ENCODING_FRF,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fmv_d_x    = {"fmv.d.x",    ASM_RISCV_ENCODING_FFR,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_feq_d      = {"feq.d",      ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_flt_d      = {"flt.d",      ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fle_d      = {"fle.d",      ASM_RISCV_ENCODING_FR2,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fclass_d   = {"fclass.d",   ASM_RISCV_ENCODING_FR1,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fld        = {"fld",        ASM_RISCV_ENCODING_FLD,    ASM_RISCV_EXT_D};
static const struct asm_riscv_opcode opcode_fsd        = {"fsd",        ASM_RISCV_ENCODING_FST,    ASM_RISCV_EXT_D};

static const struct asm_riscv_opcode opcode_c_lwsp     = {"c.lwsp",     ASM_RISCV_ENCODING_CI,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_flwsp    = {"c.flwsp",    ASM_RISCV_ENCODING_CI,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_fldsp    = {"c.fldsp",    ASM_RISCV_ENCODING_CI,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_swsp     = {"c.swsp",     ASM_RISCV_ENCODING_CSS,    ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_fswsp    = {"c.fswsp",    ASM_RISCV_ENCODING_CSS,    ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_fsdsp    = {"c.fsdsp",    ASM_RISCV_ENCODING_CSS,    ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_lw       = {"c.lw",       ASM_RISCV_ENCODING_CL,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_flw      = {"c.flw",      ASM_RISCV_ENCODING_CL,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_fld      = {"c.fld",      ASM_RISCV_ENCODING_CL,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_sw       = {"c.sw",       ASM_RISCV_ENCODING_CS,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_fsw      = {"c.fsw",      ASM_RISCV_ENCODING_CS,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_fsd      = {"c.fsd",      ASM_RISCV_ENCODING_CS,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_j        = {"c.j",        ASM_RISCV_ENCODING_CJ,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_jal      = {"c.jal",      ASM_RISCV_ENCODING_CJ,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_jr       = {"c.jr",       ASM_RISCV_ENCODING_CJR,    ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_jalr     = {"c.jalr",     ASM_RISCV_ENCODING_CJR,    ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_beqz     = {"c.beqz",     ASM_RISCV_ENCODING_CB,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_bnez     = {"c.bnez",     ASM_RISCV_ENCODING_CB,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_li       = {"c.li",       ASM_RISCV_ENCODING_CI,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_lui      = {"c.lui",      ASM_RISCV_ENCODING_CI,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_addi     = {"c.addi",     ASM_RISCV_ENCODING_CI,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_addi16sp = {"c.addi16sp", ASM_RISCV_ENCODING_CI,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_addi4spn = {"c.addi4spn", ASM_RISCV_ENCODING_CIW,    ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_slli     = {"c.slli",     ASM_RISCV_ENCODING_CSLLI,  ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_srli     = {"c.srli",     ASM_RISCV_ENCODING_CSRXI,  ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_srai     = {"c.srai",     ASM_RISCV_ENCODING_CSRXI,  ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_andi     = {"c.andi",     ASM_RISCV_ENCODING_CA,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_mv       = {"c.mv",       ASM_RISCV_ENCODING_CR,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_add      = {"c.add",      ASM_RISCV_ENCODING_CR,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_and      = {"c.and",      ASM_RISCV_ENCODING_CA,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_or       = {"c.or",       ASM_RISCV_ENCODING_CA,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_xor      = {"c.xor",      ASM_RISCV_ENCODING_CA,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_sub      = {"c.sub",      ASM_RISCV_ENCODING_CA,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_addw     = {"c.addw",     ASM_RISCV_ENCODING_CA,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_subw     = {"c.subw",     ASM_RISCV_ENCODING_CA,     ASM_RISCV_EXT_32C};
static const struct asm_riscv_opcode opcode_c_ebreak   = {"c.ebreak",   ASM_RISCV_ENCODING_NONE,   ASM_RISCV_EXT_32C};

static const struct asm_riscv_opcode opcode_add_uw     = {"add.uw",     ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBA};
static const struct asm_riscv_opcode opcode_andn       = {"andn",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_bclr       = {"bclr",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBS};
static const struct asm_riscv_opcode opcode_bclri      = {"bclri",      ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_ZBS};
static const struct asm_riscv_opcode opcode_bext       = {"bext",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBS};
static const struct asm_riscv_opcode opcode_bexti      = {"bexti",      ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_ZBS};
static const struct asm_riscv_opcode opcode_binv       = {"binv",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBS};
static const struct asm_riscv_opcode opcode_binvi      = {"binvi",      ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_ZBS};
static const struct asm_riscv_opcode opcode_bset       = {"bset",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBS};
static const struct asm_riscv_opcode opcode_bseti      = {"bseti",      ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_ZBS};
static const struct asm_riscv_opcode opcode_clmul      = {"clmul",      ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBC};
static const struct asm_riscv_opcode opcode_clmulh     = {"clmulh",     ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBC};
static const struct asm_riscv_opcode opcode_clmulr     = {"clmulr",     ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBC};
static const struct asm_riscv_opcode opcode_clz        = {"clz",        ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_clzw       = {"clzw",       ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_cpop       = {"cpop",       ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_cpopw      = {"cpopw",      ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_ctz        = {"ctz",        ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_ctzw       = {"ctzw",       ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_max        = {"max",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_maxu       = {"maxu",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_min        = {"min",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_minu       = {"minu",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_orc_b      = {"orc.b",      ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_orn        = {"orn",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_rev8       = {"rev8",       ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_rol        = {"rol",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_rolw       = {"rolw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_ror        = {"ror",        ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_rori       = {"rori",       ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_roriw      = {"roriw",      ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_rorw       = {"rorw",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_sext_b     = {"sext.b",     ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_sext_h     = {"sext.h",     ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_sh1add     = {"sh1add",     ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBA};
static const struct asm_riscv_opcode opcode_sh1add_uw  = {"sh1add.uw",  ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBA};
static const struct asm_riscv_opcode opcode_sh2add     = {"sh2add",     ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBA};
static const struct asm_riscv_opcode opcode_sh2add_uw  = {"sh2add.uw",  ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBA};
static const struct asm_riscv_opcode opcode_sh3add     = {"sh3add",     ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBA};
static const struct asm_riscv_opcode opcode_sh3add_uw  = {"sh3add.uw",  ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBA};
static const struct asm_riscv_opcode opcode_slli_uw    = {"slli.uw",    ASM_RISCV_ENCODING_SRXI,   ASM_RISCV_EXT_ZBA};
static const struct asm_riscv_opcode opcode_xnor       = {"xnor",       ASM_RISCV_ENCODING_R,      ASM_RISCV_EXT_ZBB};
static const struct asm_riscv_opcode opcode_zext_h     = {"zext.h",     ASM_RISCV_ENCODING_BC,     ASM_RISCV_EXT_ZBB};

static const char *
get_register_name(uint8_t id)
{
	static const char names[][5] =
	{
		"zero", "ra", "sp",  "gp",
		"tp",   "t0", "t1",  "t2",
		"fp",   "s1", "a0",  "a1",
		"a2",   "a3", "a4",  "a5",
		"a6",   "a7", "s2",  "s3",
		"s4",   "s5", "s6",  "s7",
		"s8",   "s9", "s10", "s11",
		"t3",   "t4", "t5",  "t6",
	};
	return names[id];
}

static const char *
get_fregister_name(uint8_t id)
{
	static const char names[][5] =
	{
		"f0",  "f1",  "f2",  "f3",
		"f4",  "f5",  "f6",  "f7",
		"f8",  "f9",  "f10", "f11",
		"f12", "f13", "f14", "f15",
		"f16", "f17", "f18", "f19",
		"f20", "f21", "f22", "f23",
		"f24", "f25", "f26", "f27",
		"f28", "f29", "f30", "f31",
	};
	return names[id];
}

static int32_t
get_imm5(uint32_t v)
{
	if (!(v & 0x10))
		return v;
	return -1 - (~v & 0xF);
}

static int32_t
get_imm6(uint32_t v)
{
	if (!(v & 0x20))
		return v;
	return -1 - (~v & 0x1F);
}

static int32_t
get_imm9(uint32_t v)
{
	if (!(v & 0x100))
		return v;
	return -1 - (~v & 0xFF);
}

static int32_t
get_imm12(uint32_t v)
{
	if (!(v & 0x800))
		return v;
	return -1 - (~v & 0x7FF);
}

static int32_t
get_imm20(uint32_t v)
{
	if (!(v & 0x100000))
		return v;
	return -1 - (~v & 0xFFFFF);
}

static void
print_r(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_register_name(RD (opcode));
	const char *rs1 = get_register_name(RS1(opcode));
	const char *rs2 = get_register_name(RS2(opcode));
	snprintf(buf, size, "%s %s,%s,%s", name, rd, rs1, rs2);
}

static void
print_i(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_register_name(RD (opcode));
	const char *rs1 = get_register_name(RS1(opcode));
	int32_t imm = get_imm12((opcode >> 20) & 0xFFF);
	snprintf(buf, size, "%s %s,%s,%" PRId32, name, rd, rs1, imm);
}

static void
print_s(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rs1 = get_register_name(RS1(opcode));
	const char *rs2 = get_register_name(RS2(opcode));
	int32_t imm = get_imm12(((opcode >> 20) & 0xFE0)
	                      | ((opcode >> 7 ) & 0x01F));
	snprintf(buf, size, "%s %s,%" PRId32 "(%s)", name, rs2, imm, rs1);
}

static void
print_b(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rs1 = get_register_name(RS1(opcode));
	const char *rs2 = get_register_name(RS2(opcode));
	int32_t imm = get_imm12(((opcode >> 19) & 0x1000)
	                      | ((opcode >> 20) & 0x07E0)
	                      | ((opcode >> 7 ) & 0x001E)
	                      | ((opcode << 5 ) & 0x0800));
	snprintf(buf, size, "%s %s,%s,%" PRId32, name, rs1, rs2, imm);
}

static void
print_u(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd = get_register_name(RD(opcode));
	int32_t imm = (int32_t)(opcode & 0xFFFFF000);
	snprintf(buf, size, "%s %s,%" PRId32, name, rd, imm);
}

static void
print_j(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd = get_register_name(RD(opcode));
	int32_t imm = get_imm20(((opcode >> 20) & 0x0007FE)
	                      | ((opcode >> 9 ) & 0x000800)
	                      | ((opcode >> 0 ) & 0x0FF000)
	                      | ((opcode >> 11) & 0x100000));
	snprintf(buf, size, "%s %s,%" PRId32, name, rd, imm);
}

static void
print_l(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd = get_register_name(RD(opcode));
	const char *rs1 = get_register_name(RS1(opcode));
	int32_t imm = get_imm12(((opcode >> 20) & 0xFE0)
	                      | ((opcode >> 7 ) & 0x01F));
	snprintf(buf, size, "%s %s,%" PRId32 "(%s)", name, rd, imm, rs1);
}

static void
print_slli(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_register_name(RD (opcode));
	const char *rs1 = get_register_name(RS1(opcode));
	int32_t imm = get_imm5((opcode >> 20) & 0x1F);
	snprintf(buf, size, "%s %s,%s,%" PRId32, name, rd, rs1, imm);
}

static void
print_srxi(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_register_name(RD (opcode));
	const char *rs1 = get_register_name(RS1(opcode));
	int32_t imm = get_imm5((opcode >> 20) & 0x1F); /* XXX use bit 25 in RV64 */
	snprintf(buf, size, "%s %s,%s,%" PRId32, name, rd, rs1, imm);
}

static void
print_none(char *buf, size_t size, uint32_t opcode, const char *name)
{
	(void)opcode;
	snprintf(buf, size, "%s", name);
}

static void
print_fence(char *buf, size_t size, uint32_t opcode, const char *name)
{
	(void)opcode;
	/* XXX decode pred / succ */
	snprintf(buf, size, "%s", name);
}

static void
print_csrrxi(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_register_name(RD(opcode));
	uint32_t uimm = (opcode >> 15) & 0x1F;
	int32_t imm = get_imm12((opcode >> 20) & 0xFFF);
	snprintf(buf, size, "%s %s,%" PRIu32 ",%" PRId32, name, rd, uimm, imm);
}

static void
print_amo(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_register_name(RD (opcode));
	const char *rs1 = get_register_name(RS1(opcode));
	const char *rs2 = get_register_name(RS2(opcode));
	/* XXX decode aq / rl */
	snprintf(buf, size, "%s %s,%s,%s", name, rd, rs1, rs2);
}

static void
print_amo_lr(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_register_name(RD (opcode));
	const char *rs1 = get_register_name(RS1(opcode));
	/* XXX decode aq / rl */
	snprintf(buf, size, "%s %s,%s", name, rd, rs1);
}

static void
print_fr3(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_fregister_name(RD (opcode));
	const char *rs1 = get_fregister_name(RS1(opcode));
	const char *rs2 = get_fregister_name(RS2(opcode));
	const char *rs3 = get_fregister_name(RS3(opcode));
	/* XXX decode rm */
	snprintf(buf, size, "%s %s,%s,%s,%s", name, rd, rs1, rs2, rs3);
}

static void
print_fr2(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_fregister_name(RD (opcode));
	const char *rs1 = get_fregister_name(RS1(opcode));
	const char *rs2 = get_fregister_name(RS2(opcode));
	/* XXX decode rm */
	snprintf(buf, size, "%s %s,%s,%s", name, rd, rs1, rs2);
}

static void
print_fr1(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_fregister_name(RD (opcode));
	const char *rs1 = get_fregister_name(RS1(opcode));
	/* XXX decode rm */
	snprintf(buf, size, "%s %s,%s", name, rd, rs1);
}

static void
print_frf(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_register_name(RD(opcode));
	const char *rs1 = get_fregister_name(RS1(opcode));
	/* XXX decode rm */
	snprintf(buf, size, "%s %s,%s", name, rd, rs1);
}

static void
print_ffr(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_fregister_name(RD(opcode));
	const char *rs1 = get_register_name(RS1(opcode));
	/* XXX decode rm */
	snprintf(buf, size, "%s %s,%s", name, rd, rs1);
}

static void
print_fld(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd  = get_fregister_name(RD(opcode));
	const char *rs1 = get_register_name(RS1(opcode));
	int32_t imm = get_imm12((opcode >> 20) & 0xFFF);
	/* XXX decode rm */
	snprintf(buf, size, "%s %s,%" PRId32 "(%s)", name, rd, imm, rs1);
}

static void
print_fst(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rs1 = get_register_name(RS1(opcode));
	const char *rs2 = get_fregister_name(RS2(opcode));
	int32_t imm = get_imm12(((opcode >> 20) & 0xFE0)
	                      | ((opcode >> 7 ) & 0x01F));
	/* XXX decode rm */
	snprintf(buf, size, "%s %s,%" PRId32 "(%s)", name, rs1, imm, rs2);
}

static void
print_cr(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rd  = get_register_name(CRD (opcode));
	const char *rs2 = get_register_name(CRS2(opcode));
	snprintf(buf, size, "%s %s,%s", name, rd, rs2);
}

static void
print_ci(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rd = get_register_name(CRD(opcode));
	int32_t imm = get_imm6(((opcode >> 2) & 0x1F)
	                     | ((opcode >> 7) & 0x20));
	snprintf(buf, size, "%s %s,%" PRId32, name, rd, imm);
}

static void
print_css(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rs2 = get_register_name(CRS2(opcode));
	uint32_t imm = ((opcode >> 7) & 0x3C)
	             | ((opcode >> 2) & 0xC0);
	snprintf(buf, size, "%s %s,%" PRIu32, name, rs2, imm);
}

static void
print_ciw(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rd = get_register_name(CRDP(opcode));
	uint32_t imm = ((opcode >> 4) & 0x004)
	             | ((opcode >> 2) & 0x008)
	             | ((opcode >> 6) & 0x030)
	             | ((opcode >> 2) & 0x3C0);
	snprintf(buf, size, "%s %s,%" PRIu32, name, rd, imm);
}

static void
print_cl(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rd  = get_register_name(CRDP (opcode));
	const char *rs1 = get_register_name(CRS1P(opcode));
	uint32_t imm = ((opcode >> 4) & 0x04)
	             | ((opcode >> 7) & 0x38)
	             | ((opcode << 1) & 0x40);
	snprintf(buf, size, "%s %s,%" PRIu32 "(%s)", name, rd, imm, rs1);
}

static void
print_cs(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rs1 = get_register_name(CRS1P(opcode));
	const char *rs2 = get_register_name(CRDP (opcode));
	uint32_t imm = ((opcode >> 4) & 0x04)
	             | ((opcode >> 7) & 0x38)
	             | ((opcode << 1) & 0x40);
	snprintf(buf, size, "%s %s,%" PRIu32 "(%s)", name, rs2, imm, rs1);
}

static void
print_ca(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rd  = get_register_name(CRS1P(opcode));
	const char *rs2 = get_register_name(CRDP (opcode));
	snprintf(buf, size, "%s %s,%s", name, rd, rs2);
}

static void
print_cb(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rs1 = get_register_name(CRS1P(opcode));
	int32_t imm = get_imm9(((opcode >> 2) & 0x006)
	                     | ((opcode >> 7) & 0x018)
	                     | ((opcode << 3) & 0x020)
	                     | ((opcode << 1) & 0x0C0)
	                     | ((opcode >> 4) & 0x100));
	snprintf(buf, size, "%s %s,%" PRId32, name, rs1, imm);
}

static void
print_cj(char *buf, size_t size, uint16_t opcode, const char *name)
{
	int32_t imm = get_imm12(((opcode >> 2) & 0x00E)
	                      | ((opcode >> 7) & 0x010)
	                      | ((opcode << 3) & 0x020)
	                      | ((opcode >> 1) & 0x040)
	                      | ((opcode << 1) & 0x080)
	                      | ((opcode << 1) & 0x300)
	                      | ((opcode << 2) & 0x400)
	                      | ((opcode >> 1) & 0x800));
	snprintf(buf, size, "%s %" PRId32, name, imm);
}

static void
print_cjr(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rs1 = get_register_name(CRD(opcode));
	snprintf(buf, size, "%s %s", name, rs1);
}

static void
print_cslli(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rd = get_register_name(CRD(opcode));
	uint32_t shamt = ((opcode >> 2) & 0x1F)
	               | ((opcode >> 7) & 0x20);
	snprintf(buf, size, "%s %s,%" PRIu32, name, rd, shamt);
}

static void
print_csrxi(char *buf, size_t size, uint16_t opcode, const char *name)
{
	const char *rd = get_register_name(CRS1P(opcode));
	uint32_t shamt = ((opcode >> 2) & 0x1F)
	               | ((opcode >> 7) & 0x20);
	snprintf(buf, size, "%s %s,%" PRIu32, name, rd, shamt);
}

static void
print_bc(char *buf, size_t size, uint32_t opcode, const char *name)
{
	const char *rd = get_register_name(RD (opcode));
	const char *rs = get_register_name(RS1(opcode));
	snprintf(buf, size, "%s %s,%s", name, rd, rs);
}

static const struct asm_riscv_opcode *
decode_load(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x0:
			return &opcode_lb;
		case 0x1:
			return &opcode_lh;
		case 0x2:
			return &opcode_lw;
		case 0x3:
			return &opcode_ld;
		case 0x4:
			return &opcode_lbu;
		case 0x5:
			return &opcode_lhu;
		case 0x6:
			return &opcode_lwu;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_load_fp(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x2:
			return &opcode_flw;
		case 0x3:
			return &opcode_fld;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_custom_0(uint32_t opcode)
{
	(void)opcode;
	/* XXX */
	return NULL;
}

static const struct asm_riscv_opcode *
decode_misc_mem(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x0:
			return &opcode_fence;
		case 0x1:
			return &opcode_fence_i;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_op_imm(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x0:
			return &opcode_addi;
		case 0x1:
			switch (FUNCT7(opcode))
			{
				case 0x00:
					return &opcode_slli;
				case 0x01:
					return &opcode_slli; /* XXX RV64 only */
				case 0x14:
					return &opcode_bseti;
				case 0x15:
					return &opcode_bseti; /* XXX RV64 only */
				case 0x24:
					return &opcode_bclri;
				case 0x25:
					return &opcode_bclri; /* XXX RV64 only */
				case 0x30:
					switch (RS2(opcode))
					{
						case 0x00:
							return &opcode_clz;
						case 0x01:
							return &opcode_ctz;
						case 0x02:
							return &opcode_cpop;
						case 0x04:
							return &opcode_sext_b;
						case 0x05:
							return &opcode_sext_h;
					}
					break;
				case 0x34:
					return &opcode_binvi;
				case 0x35:
					return &opcode_binvi; /* XXX RV64 only */
			}
			break;
		case 0x2:
			return &opcode_slti;
		case 0x3:
			return &opcode_sltiu;
		case 0x4:
			return &opcode_xori;
		case 0x5:
			switch (FUNCT7(opcode))
			{
				case 0x00:
					return &opcode_srli;
				case 0x01:
					return &opcode_srli; /* XXX RV64 only */
				case 0x14:
					switch (RS2(opcode))
					{
						case 0x07:
							return &opcode_orc_b;
					}
					break;
				case 0x20:
					return &opcode_srai;
				case 0x21:
					return &opcode_srai; /* XXX RV64 only */
				case 0x24:
					return &opcode_bexti;
				case 0x25:
					return &opcode_bexti; /* XXX RV64 only */
				case 0x30:
					return &opcode_rori;
				case 0x31:
					return &opcode_rori; /* XXX RV64 only */
				case 0x34: /* XXX RV32 only */
					switch (RS2(opcode))
					{
						case 0x18:
							return &opcode_rev8;
					}
					break;
				case 0x35: /* XXX RV64 only */
					switch (RS2(opcode))
					{
						case 0x18:
							return &opcode_rev8;
					}
					break;
			}
			break;
		case 0x6:
			return &opcode_ori;
		case 0x7:
			return &opcode_andi;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_op_imm_32(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x0:
			return &opcode_addiw;
		case 0x1:
			switch (FUNCT7(opcode))
			{
				case 0x00:
					return &opcode_slliw;
				case 0x04:
					return &opcode_slli_uw;
				case 0x05:
					return &opcode_slli_uw;
				case 0x30:
					switch (RS2(opcode))
					{
						case 0x00:
							return &opcode_clzw;
						case 0x01:
							return &opcode_ctzw;
						case 0x02:
							return &opcode_cpopw;
					}
					break;
			}
			break;
		case 0x5:
			switch (FUNCT7(opcode))
			{
				case 0x00:
					return &opcode_srliw;
				case 0x20:
					return &opcode_sraiw;
				case 0x30:
					return &opcode_roriw;
			}
			break;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_store(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x0:
			return &opcode_sb;
		case 0x1:
			return &opcode_sh;
		case 0x2:
			return &opcode_sw;
		case 0x03:
			return &opcode_sd;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_store_fp(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x2:
			return &opcode_fsw;
		case 0x3:
			return &opcode_fsd;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_custom_1(uint32_t opcode)
{
	(void)opcode;
	/* XXX */
	return NULL;
}

static const struct asm_riscv_opcode *
decode_amo(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	uint32_t funct5 = FUNCT5(opcode);
	switch (funct3)
	{
		case 0x2:
			switch (funct5)
			{
				case 0x00:
					return &opcode_amoadd_w;
				case 0x01:
					return &opcode_amoswap_w;
				case 0x02:
					if (!RS2(opcode))
						return &opcode_lr_w;
					break;
				case 0x03:
					return &opcode_sc_w;
				case 0x04:
					return &opcode_amoxor_w;
				case 0x08:
					return &opcode_amoor_w;
				case 0x0C:
					return &opcode_amoand_w;
				case 0x10:
					return &opcode_amomin_w;
				case 0x14:
					return &opcode_amomax_w;
				case 0x18:
					return &opcode_amominu_w;
				case 0x1C:
					return &opcode_amomaxu_w;
			}
			break;
		case 0x3:
			switch (funct5)
			{
				case 0x00:
					return &opcode_amoadd_d;
				case 0x01:
					return &opcode_amoswap_d;
				case 0x02:
					if (!RS2(opcode))
						return &opcode_lr_d;
					break;
				case 0x03:
					return &opcode_sc_d;
				case 0x04:
					return &opcode_amoxor_d;
				case 0x08:
					return &opcode_amoor_d;
				case 0x0C:
					return &opcode_amoand_d;
				case 0x10:
					return &opcode_amomin_d;
				case 0x14:
					return &opcode_amomax_d;
				case 0x18:
					return &opcode_amominu_d;
				case 0x1C:
					return &opcode_amomaxu_d;
			}
			break;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_op(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	uint32_t funct7 = FUNCT7(opcode);
	switch (funct3)
	{
		case 0x0:
			switch (funct7)
			{
				case 0x00:
					return &opcode_add;
				case 0x01:
					return &opcode_mul;
				case 0x20:
					return &opcode_sub;
			}
			break;
		case 0x1:
			switch (funct7)
			{
				case 0x00:
					return &opcode_sll;
				case 0x01:
					return &opcode_mulh;
				case 0x05:
					return &opcode_clmul;
				case 0x14:
					return &opcode_bset;
				case 0x24:
					return &opcode_bclr;
				case 0x30:
					return &opcode_rol;
				case 0x34:
					return &opcode_binv;
			}
			break;
		case 0x2:
			switch (funct7)
			{
				case 0x00:
					return &opcode_slt;
				case 0x01:
					return &opcode_mulhsu;
				case 0x05:
					return &opcode_clmulr;
				case 0x10:
					return &opcode_sh1add;
			}
			break;
		case 0x3:
			switch (funct7)
			{
				case 0x00:
					return &opcode_sltu;
				case 0x01:
					return &opcode_mulhu;
				case 0x05:
					return &opcode_clmulh;
			}
			break;
		case 0x4:
			switch (funct7)
			{
				case 0x00:
					return &opcode_xor;
				case 0x01:
					return &opcode_div;
				case 0x04:
					switch (RS2(opcode))
					{
						case 0x00:
							return &opcode_zext_h; /* XXX RV32 only */
					}
					break;
				case 0x05:
					return &opcode_min;
				case 0x10:
					return &opcode_sh2add;
				case 0x20:
					return &opcode_xnor;
			}
			break;
		case 0x5:
			switch (funct7)
			{
				case 0x00:
					return &opcode_sra;
				case 0x01:
					return &opcode_divu;
				case 0x05:
					return &opcode_minu;
				case 0x20:
					return &opcode_srl;
				case 0x24:
					return &opcode_bext;
				case 0x30:
					return &opcode_ror;
			}
			break;
		case 0x6:
			switch (funct7)
			{
				case 0x00:
					return &opcode_or;
				case 0x01:
					return &opcode_rem;
				case 0x05:
					return &opcode_max;
				case 0x10:
					return &opcode_sh3add;
				case 0x20:
					return &opcode_orn;
			}
			break;
		case 0x7:
			switch (funct7)
			{
				case 0x00:
					return &opcode_and;
				case 0x01:
					return &opcode_remu;
				case 0x05:
					return &opcode_maxu;
				case 0x20:
					return &opcode_andn;
			}
			break;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_op_32(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	uint32_t funct7 = FUNCT7(opcode);
	switch (funct3)
	{
		case 0x0:
			switch (funct7)
			{
				case 0x00:
					return &opcode_addw;
				case 0x01:
					return &opcode_mulw;
				case 0x04:
					return &opcode_add_uw;
				case 0x20:
					return &opcode_subw;
			}
			break;
		case 0x1:
			switch (funct7)
			{
				case 0x00:
					return &opcode_sllw;
				case 0x30:
					return &opcode_rolw;
			}
			break;
		case 0x2:
			switch (funct7)
			{
				case 0x10:
					return &opcode_sh1add_uw;
			}
			break;
		case 0x4:
			switch (funct7)
			{
				case 0x01:
					return &opcode_divw;
				case 0x04:
					switch (RS2(opcode))
					{
						case 0x00:
							return &opcode_zext_h; /* XXX RV64 only */
					}
					break;
				case 0x10:
					return &opcode_sh2add_uw;
			}
			break;
		case 0x5:
			switch (funct7)
			{
				case 0x00:
					return &opcode_srlw;
				case 0x01:
					return &opcode_divuw;
				case 0x20:
					return &opcode_sraw;
				case 0x30:
					return &opcode_rorw;
			}
			break;
		case 0x6:
			switch (funct7)
			{
				case 0x01:
					return &opcode_remw;
				case 0x10:
					return &opcode_sh3add_uw;
			}
			break;
		case 0x07:
			switch (funct7)
			{
				case 0x01:
					return &opcode_remuw;
			}
			break;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_madd(uint32_t opcode)
{
	uint32_t fmt = FMT(opcode);
	switch (fmt)
	{
		case 0x0:
			return &opcode_fmadd_s;
		case 0x1:
			return &opcode_fmadd_d;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_msub(uint32_t opcode)
{
	uint32_t fmt = FMT(opcode);
	switch (fmt)
	{
		case 0x0:
			return &opcode_fmsub_s;
		case 0x1:
			return &opcode_fmsub_d;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_nmsub(uint32_t opcode)
{
	uint32_t fmt = FMT(opcode);
	switch (fmt)
	{
		case 0x0:
			return &opcode_fnmsub_s;
		case 0x1:
			return &opcode_fnmsub_d;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_nmadd(uint32_t opcode)
{
	uint32_t fmt = FMT(opcode);
	switch (fmt)
	{
		case 0x0:
			return &opcode_fnmadd_s;
		case 0x1:
			return &opcode_fnmadd_d;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_op_fp(uint32_t opcode)
{
	uint32_t funct5 = FUNCT5(opcode);
	switch (funct5)
	{
		case 0x0:
			switch (FMT(opcode))
			{
				case 0x0:
					return &opcode_fadd_s;
				case 0x1:
					return &opcode_fadd_d;
			}
			break;
		case 0x1:
			switch (FMT(opcode))
			{
				case 0x0:
					return &opcode_fsub_s;
				case 0x1:
					return &opcode_fsub_d;
			}
			break;
		case 0x2:
			switch (FMT(opcode))
			{
				case 0x0:
					return &opcode_fmul_s;
				case 0x1:
					return &opcode_fmul_d;
			}
			break;
		case 0x3:
			switch (FMT(opcode))
			{
				case 0x0:
					return &opcode_fdiv_s;
				case 0x1:
					return &opcode_fdiv_d;
			}
			break;
		case 0x4:
			switch (FMT(opcode))
			{
				case 0x0:
					switch (RM(opcode))
					{
						case 0x0:
							return &opcode_fsgnj_s;
						case 0x1:
							return &opcode_fsgnjn_s;
						case 0x2:
							return &opcode_fsgnjx_s;
					}
					break;
				case 0x1:
					switch (RM(opcode))
					{
						case 0x0:
							return &opcode_fsgnj_d;
						case 0x1:
							return &opcode_fsgnjn_d;
						case 0x2:
							return &opcode_fsgnjx_d;
					}
					break;
			}
			break;
		case 0x5:
			switch (FMT(opcode))
			{
				case 0x0:
					switch (RM(opcode))
					{
						case 0x0:
							return &opcode_fmin_s;
						case 0x1:
							return &opcode_fmax_s;
					}
					break;
				case 0x1:
					switch (RM(opcode))
					{
						case 0x0:
							return &opcode_fmin_d;
						case 0x1:
							return &opcode_fmax_d;
					}
					break;
			}
			break;
		case 0x8:
			switch (FMT(opcode))
			{
				case 0x0:
					switch (RS2(opcode))
					{
						case 0x1:
							return &opcode_fcvt_s_d;
					}
					break;
				case 0x1:
					switch (RS2(opcode))
					{
						case 0x0:
							return &opcode_fcvt_d_s;
					}
					break;
			}
			break;
		case 0xB:
			switch (FMT(opcode))
			{
				case 0x0:
					return &opcode_fsqrt_s;
				case 0x1:
					return &opcode_fsqrt_d;
			}
			break;
		case 0x14:
			switch (FMT(opcode))
			{
				case 0x0:
					switch (RM(opcode))
					{
						case 0x0:
							return &opcode_fle_s;
						case 0x1:
							return &opcode_flt_s;
						case 0x2:
							return &opcode_feq_s;
					}
					break;
				case 0x1:
					switch (RM(opcode))
					{
						case 0x0:
							return &opcode_fle_d;
						case 0x1:
							return &opcode_flt_d;
						case 0x2:
							return &opcode_feq_d;
					}
					break;
			}
			break;
		case 0x18:
			switch (FMT(opcode))
			{
				case 0x0:
					switch (RS2(opcode))
					{
						case 0x0:
							return &opcode_fcvt_w_s;
						case 0x1:
							return &opcode_fcvt_wu_s;
					}
					break;
				case 0x1:
					switch (RS2(opcode))
					{
						case 0x0:
							return &opcode_fcvt_w_d;
						case 0x1:
							return &opcode_fcvt_wu_d;
					}
					break;
			}
			break;
		case 0x1A:
			switch (FMT(opcode))
			{
				case 0x0:
					switch (RS2(opcode))
					{
						case 0x0:
							return &opcode_fcvt_s_w;
						case 0x1:
							return &opcode_fcvt_s_wu;
					}
					break;
				case 0x1:
					switch (RS2(opcode))
					{
						case 0x0:
							return &opcode_fcvt_d_w;
						case 0x1:
							return &opcode_fcvt_d_wu;
					}
					break;
			}
			break;
		case 0x1C:
			switch (FMT(opcode))
			{
				case 0x0:
					switch (RM(opcode))
					{
						case 0x0:
							switch (RS2(opcode))
							{
								case 0x0:
									return &opcode_fmv_x_w;
							}
							break;
						case 0x1:
							switch (RS2(opcode))
							{
								case 0x0:
									return &opcode_fclass_s;
							}
							break;
					}
					break;
				case 0x1:
					switch (RM(opcode))
					{
						case 0x0:
							switch (RS2(opcode))
							{
								case 0x0:
									return &opcode_fmv_x_d;
							}
							break;
						case 0x1:
							switch (RS2(opcode))
							{
								case 0x0:
									return &opcode_fclass_d;
							}
							break;
					}
					break;
			}
			break;
		case 0x1E:
			switch (FMT(opcode))
			{
				case 0x0:
					switch (RM(opcode))
					{
						case 0x0:
							switch (RS2(opcode))
							{
								case 0x0:
									return &opcode_fmv_w_x;
							}
							break;
					}
					break;
				case 0x1:
					switch (RM(opcode))
					{
						case 0x0:
							switch (RS2(opcode))
							{
								case 0x0:
									return &opcode_fmv_d_x;
							}
							break;
					}
					break;
			}
			break;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_custom2(uint32_t opcode)
{
	(void)opcode;
	/* XXX */
	return NULL;
}

static const struct asm_riscv_opcode *
decode_branch(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x0:
			return &opcode_beq;
		case 0x1:
			return &opcode_bne;
		case 0x4:
			return &opcode_blt;
		case 0x5:
			return &opcode_bge;
		case 0x6:
			return &opcode_bltu;
		case 0x7:
			return &opcode_bgeu;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_jalr(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x0:
			return &opcode_jalr;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_system(uint32_t opcode)
{
	uint32_t funct3 = FUNCT3(opcode);
	switch (funct3)
	{
		case 0x0:
			switch (FUNCT12(opcode))
			{
				case 0x0:
					if (opcode == 0x73)
						return &opcode_ecall;
					break;
				case 0x2:
					if (opcode == 0x100073)
						return &opcode_ebreak;
					break;
				case 0x3:
					switch (FUNCT5(opcode))
					{
						case 0x0:
							return &opcode_uret;
						case 0x1:
							return &opcode_sret;
						case 0x2:
							return &opcode_mret;
					}
					break;
				case 0x5:
					switch (FUNCT5(opcode))
					{
						case 0x2:
							switch ((opcode >> 25) & 0x3)
							{
								case 0x0:
									return &opcode_wfi;
								case 0x1:
									return &opcode_sfence_vma;
							}
							break;
					}
					break;
			}
			break;
		case 0x1:
			return &opcode_csrrw;
		case 0x2:
			return &opcode_csrrs;
		case 0x3:
			return &opcode_csrrc;
		case 0x5:
			return &opcode_csrrwi;
		case 0x6:
			return &opcode_csrrsi;
		case 0x7:
			return &opcode_csrrci;
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_custom3(uint32_t opcode)
{
	(void)opcode;
	/* XXX */
	return NULL;
}

static const struct asm_riscv_opcode *
decode_32(uint32_t opcode)
{
	switch ((opcode >> 2) & 0x1F)
	{
		case 0x0:
			return decode_load(opcode);
		case 0x1:
			return decode_load_fp(opcode);
		case 0x2:
			return decode_custom_0(opcode);
		case 0x3:
			return decode_misc_mem(opcode);
		case 0x4:
			return decode_op_imm(opcode);
		case 0x5:
			return &opcode_auipc;
		case 0x6:
			return decode_op_imm_32(opcode);
		case 0x7:
			return NULL; /* 48b */
		case 0x8:
			return decode_store(opcode);
		case 0x9:
			return decode_store_fp(opcode);
		case 0xA:
			return decode_custom_1(opcode);
		case 0xB:
			return decode_amo(opcode);
		case 0xC:
			return decode_op(opcode);
		case 0xD:
			return &opcode_lui;
		case 0xE:
			return decode_op_32(opcode);
		case 0xF:
			return NULL; /* 64b */
		case 0x10:
			return decode_madd(opcode);
		case 0x11:
			return decode_msub(opcode);
		case 0x12:
			return decode_nmsub(opcode);
		case 0x13:
			return decode_nmadd(opcode);
		case 0x14:
			return decode_op_fp(opcode);
		case 0x15:
			return NULL; /* reserved */
		case 0x16:
			return decode_custom2(opcode);
		case 0x17:
			return NULL; /* 48b */
		case 0x18:
			return decode_branch(opcode);
		case 0x19:
			return decode_jalr(opcode);
		case 0x1A:
			return NULL; /* reserved */
		case 0x1B:
			return &opcode_jal;
		case 0x1C:
			return decode_system(opcode);
		case 0x1D:
			return NULL; /* reserved */
		case 0x1E:
			return decode_custom3(opcode);
		case 0x1F:
			return NULL; /* >= 80b */
	}
	return NULL;
}

static const struct asm_riscv_opcode *
decode_16(uint16_t opcode)
{
	switch (opcode & 0x3)
	{
		case 0x0:
			switch (CFUNCT3(opcode))
			{
				case 0x0:
					return &opcode_c_addi4spn;
				case 0x1:
					return &opcode_c_fld; /* XXX depend RVX */
				case 0x2:
					return &opcode_c_lw;
				case 0x3:
					return &opcode_c_flw; /* XXX depend RVX */
				case 0x5:
					return &opcode_c_fsd; /* XXX depend RVX */
				case 0x6:
					return &opcode_c_sw;
				case 0x7:
					return &opcode_c_fsw; /* XXX depend RVX */
			}
			break;
		case 0x1:
			switch (CFUNCT3(opcode))
			{
				case 0x0:
					return &opcode_c_addi;
				case 0x1:
					return &opcode_c_jal; /* XXX depend RVX */
				case 0x2:
					return &opcode_c_li;
				case 0x3:
					if (CRD(opcode) == 0x2)
						return &opcode_c_addi16sp;
					return &opcode_c_lui;
				case 0x4:
					switch ((opcode >> 10) & 0x7)
					{
						case 0x0:
							return &opcode_c_srli;
						case 0x1:
							return &opcode_c_srai;
						case 0x2:
							return &opcode_c_andi;
						case 0x3:
							switch (CFUNCT2(opcode))
							{
								case 0x0:
									return &opcode_c_sub;
								case 0x1:
									return &opcode_c_xor;
								case 0x2:
									return &opcode_c_or;
								case 0x3:
									return &opcode_c_and;
							}
							break;
						case 0x4:
							return &opcode_c_srli;
						case 0x5:
							return &opcode_c_srai;
						case 0x6:
							return &opcode_c_andi;
						case 0x7:
							switch (CFUNCT2(opcode))
							{
								case 0x0:
									return &opcode_c_subw;
								case 0x1:
									return &opcode_c_addw;
							}
							break;
					}
					break;
				case 0x5:
					return &opcode_c_j;
				case 0x6:
					return &opcode_c_beqz;
				case 0x7:
					return &opcode_c_bnez;
			}
			break;
		case 0x2:
			switch (CFUNCT3(opcode))
			{
				case 0x0:
					return &opcode_c_slli;
				case 0x1:
					return &opcode_c_fldsp; /* XXX depend RVX */
				case 0x2:
					return &opcode_c_lwsp;
				case 0x3:
					return &opcode_c_flwsp; /* XXX depend RVX */
				case 0x4:
					switch (CFUNCT4(opcode))
					{
						case 0x8:
							if (!CRS2(opcode))
								return &opcode_c_jr;
							return &opcode_c_mv;
						case 0x9:
							if (opcode == 0x9002)
								return &opcode_c_ebreak;
							if (!CRS2(opcode))
								return &opcode_c_jalr;
							return &opcode_c_add;
					}
					break;
				case 0x5:
					return &opcode_c_fsdsp; /* XXX depend RVX */
				case 0x6:
					return &opcode_c_swsp;
				case 0x7:
					return &opcode_c_fswsp; /* XXX depend RVX */
			}
			break;
	}
	return NULL;
}

int
asm_riscv_disas(char *buf, size_t size, const uint8_t *data, size_t pos)
{
	const struct asm_riscv_opcode *opcode;
	size_t length;

	switch (*data & 0x3)
	{
		case 0x3:
			opcode = decode_32(*(uint32_t*)data);
			length = 4;
			break;
		default:
			opcode = decode_16(*(uint16_t*)data);
			length = 2;
			break;
	}
	if (!opcode)
	{
		strlcpy(buf, "", size);
		return length;
	}
	switch (opcode->encoding)
	{
		case ASM_RISCV_ENCODING_R:
			print_r(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_I:
			print_i(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_S:
			print_s(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_B:
			print_b(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_U:
			print_u(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_J:
			print_j(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_L:
			print_l(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_SLLI:
			print_slli(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_SRXI:
			print_srxi(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_NONE:
			print_none(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_FENCE:
			print_fence(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CSRRXI:
			print_csrrxi(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_AMO:
			print_amo(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_AMO_LR:
			print_amo_lr(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_FR1:
			print_fr1(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_FR2:
			print_fr2(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_FR3:
			print_fr3(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_FFR:
			print_ffr(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_FRF:
			print_frf(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_FLD:
			print_fld(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_FST:
			print_fst(buf, size, *(uint32_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CR:
			print_cr(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CI:
			print_ci(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CSS:
			print_css(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CIW:
			print_ciw(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CL:
			print_cl(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CS:
			print_cs(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CA:
			print_ca(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CB:
			print_cb(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CJ:
			print_cj(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CJR:
			print_cjr(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CSLLI:
			print_cslli(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_CSRXI:
			print_csrxi(buf, size, *(uint16_t*)data, opcode->name);
			break;
		case ASM_RISCV_ENCODING_BC:
			print_bc(buf, size, *(uint32_t*)data, opcode->name);
			break;
		default:
			strlcpy(buf, "", size);
			break;
	}
	return length;
}
