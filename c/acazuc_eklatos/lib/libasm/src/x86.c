#include <libasm/x86.h>

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

static const char *get_reg_8_name(uint8_t v)
{
	static const char names[8][4] =
	{
		"al",
		"cl",
		"dl",
		"bl",
		"ah",
		"ch",
		"dh",
		"bh",
	};
	return names[v];
}

static const char *get_reg_16_name(uint8_t v)
{
	static const char names[8][4] =
	{
		"ax",
		"cx",
		"dx",
		"bx",
		"sp",
		"bp",
		"si",
		"di",
	};
	return names[v];
}

static const char *get_reg_32_name(uint8_t v)
{
	static const char names[8][4] =
	{
		"eax",
		"ecx",
		"edx",
		"ebx",
		"esp",
		"ebp",
		"esi",
		"edi",
	};
	return names[v];
}

static const char *get_reg_x87_name(uint8_t v)
{
	static const char names[8][6] =
	{
		"st(0)",
		"st(1)",
		"st(2)",
		"st(3)",
		"st(4)",
		"st(5)",
		"st(6)",
		"st(7)",
	};
	return names[v];
}

static const char *get_reg_mmx_name(uint8_t v)
{
	static const char names[8][5] =
	{
		"mmx0",
		"mmx1",
		"mmx2",
		"mmx3",
		"mmx4",
		"mmx5",
		"mmx6",
		"mmx7",
	};
	return names[v];
}

static const char *get_reg_xmm_name(uint8_t v)
{
	static const char names[8][5] =
	{
		"xmm0",
		"xmm1",
		"xmm2",
		"xmm3",
		"xmm4",
		"xmm5",
		"xmm6",
		"xmm7",
	};
	return names[v];
}

static const char *get_reg_ctrl_name(uint8_t v)
{
	static const char names[8][4] =
	{
		"cr0",
		"cr1",
		"cr2",
		"cr3",
		"cr4",
		"cr5",
		"cr6",
		"cr7",
	};
	return names[v];
}

static const char *get_reg_dbg_name(uint8_t v)
{
	static const char names[8][4] =
	{
		"dr0",
		"dr1",
		"dr2",
		"dr3",
		"dr4",
		"dr5",
		"dr6",
		"dr7",
	};
	return names[v];
}

static const char *get_reg_name(enum asm_x86_reg_size size, uint8_t v)
{
	switch (size)
	{
		case ASM_X86_REG_8:
			return get_reg_8_name(v);
		case ASM_X86_REG_16:
			return get_reg_16_name(v);
		case ASM_X86_REG_32:
			return get_reg_32_name(v);
		case ASM_X86_REG_X87:
			return get_reg_x87_name(v);
		case ASM_X86_REG_MMX:
			return get_reg_mmx_name(v);
		case ASM_X86_REG_XMM:
			return get_reg_xmm_name(v);
		case ASM_X86_REG_CTRL:
			return get_reg_ctrl_name(v);
		case ASM_X86_REG_DBG:
			return get_reg_dbg_name(v);
	}
	return "";
}

static const char *get_segment_name(enum asm_x86_segment segment)
{
	static const char names[8][3] =
	{
		"",
		"es",
		"cs",
		"ss",
		"ds",
		"fs",
		"gs",
	};
	return names[segment];
}

static int print_sib0(char *buf, size_t size, const uint8_t *data,
                      enum asm_x86_reg_size addr_size, const char *seg_str)
{
	uint8_t sib = *data;
	switch ((sib >> 3) & 0x7)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x5:
		case 0x6:
		case 0x7:
			switch (sib & 0x7)
			{
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x6:
				case 0x7:
					snprintf(buf, size, "%s(%%%s,%%%s,%d)",
					         seg_str,
					         get_reg_name(addr_size, (sib >> 0) & 0x7),
					         get_reg_name(addr_size, (sib >> 3) & 0x7),
					         1 << (sib >> 6));
					return 2;
				case 0x5:
					snprintf(buf, size, "%s(0x%" PRIx32 ",%%%s,%d)",
					         seg_str, *(uint32_t*)&data[1],
					         get_reg_name(addr_size, (sib >> 3) & 0x7),
					         1 << (sib >> 6));
					return 6;
			}
			break;
		case 0x4:
			switch (sib & 0x7)
			{
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x6:
				case 0x7:
					snprintf(buf, size, "%s(%%%s)",
					         seg_str,
					         get_reg_name(addr_size, sib & 0x7));
					return 2;
				case 0x5:
					snprintf(buf, size, "%s(0x%" PRIx32 ")",
					         seg_str,
					         *(uint32_t*)&data[1]);
					return 6;
			}
			break;
	}
	return 2;
}

static int print_sib1(char *buf, size_t size, const uint8_t *data,
                      enum asm_x86_reg_size reg_size, const char *seg_str)
{
	uint8_t sib = *data;
	switch ((sib >> 3) & 0x7)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x5:
		case 0x6:
		case 0x7:
			snprintf(buf, size, "%s0x%" PRIx8 "(%%%s,%%%s,%d)",
			         seg_str, *(uint8_t*)&data[1],
			         get_reg_name(reg_size, (sib >> 0) & 0x7),
			         get_reg_name(reg_size, (sib >> 3) & 0x7),
			         1 << (sib >> 6));
			return 3;
		case 0x4:
			if ((sib & 0x7) == 0x4)
				snprintf(buf, size, "%s0x%" PRIx8 "(%%esp)",
			             seg_str, *(uint8_t*)&data[1]);
			else
				snprintf(buf, size, "%s0x%" PRIx8 "(%%%s,%%eiz,1)",
				         seg_str, *(uint8_t*)&data[1],
				         get_reg_name(reg_size, sib & 0x7));
			return 3;
	}
	return 2;
}

static int print_sib2(char *buf, size_t size, const uint8_t *data,
                      enum asm_x86_reg_size addr_size, const char *seg_str)
{
	uint8_t sib = *data;
	switch ((sib >> 3) & 0x7)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x5:
		case 0x6:
		case 0x7:
			snprintf(buf, size, "%s0x%" PRIx32 "(%%%s,%%%s,%d)",
			         seg_str, *(uint32_t*)&data[1],
			         get_reg_name(addr_size, (sib >> 0) & 0x7),
			         get_reg_name(addr_size, (sib >> 3) & 0x7),
			         1 << (sib >> 6));
			return 6;
		case 0x4:
			if ((sib & 0x7) == 0x4)
				snprintf(buf, size, "%s0x%" PRIx32 "(%%esp)",
			             seg_str, *(uint32_t*)&data[1]);
			else
				snprintf(buf, size, "%s0x%" PRIx32 "(%%%s,%%eiz,1)",
				         seg_str, *(uint32_t*)&data[1],
				         get_reg_name(addr_size, sib & 0x7));
			return 6;
	}
	return 2;
}

static void get_segment_prefix(char *dst, size_t size,
                               enum asm_x86_segment segment)
{
	const char *segment_name = get_segment_name(segment);
	if (!segment_name || !segment_name[0])
	{
		*dst = '\0';
		return;
	}
	snprintf(dst, size, "%s:", segment_name);
}

static size_t print_mrm(char *buf, size_t size, const uint8_t *data,
                        enum asm_x86_reg_size operand_size,
                        enum asm_x86_reg_size addr_size,
                        enum asm_x86_segment segment)
{
	uint8_t mrm = *data;
	char seg_str[5];
	get_segment_prefix(seg_str, sizeof(seg_str), segment);
	switch (mrm & 0xC0)
	{
		case 0x00:
			switch (mrm & 0x7)
			{
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x6:
				case 0x7:
					snprintf(buf, size, "%s(%%%s)",
					         seg_str,
					         get_reg_name(addr_size, mrm & 0x7));
					return 1;
				case 0x4:
					return print_sib0(buf, size, &data[1],
					                  addr_size, seg_str);
				case 0x5:
					snprintf(buf, size, "%s(0x%" PRIx32 ")",
					         seg_str, *(uint32_t*)&data[1]);
					return 5;
			}
			break;
		case 0x40:
			switch (mrm & 0x7)
			{
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x5:
				case 0x6:
				case 0x7:
				{
					int8_t v = *(int8_t*)&data[1];
					if (v >= 0)
						snprintf(buf, size, "%s0x%" PRIx8 "(%%%s)",
						         seg_str, v,
						         get_reg_name(addr_size, mrm & 0x7));
					else
						snprintf(buf, size, "%s-0x%" PRIx8 "(%%%s)",
						         seg_str, -v,
						         get_reg_name(addr_size, mrm & 0x7));
					return 2;
				}
				case 0x4:
					return print_sib1(buf, size, &data[1],
					                  addr_size, seg_str);
			}
			break;
		case 0x80:
			switch (mrm & 0x7)
			{
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x5:
				case 0x6:
				case 0x7:
				{
					int32_t v = *(int32_t*)&data[1];
					if (v >= 0)
						snprintf(buf, size, "%s0x%" PRIx32 "(%%%s)",
						         seg_str, v,
						         get_reg_name(addr_size, mrm & 0x7));
					else
						snprintf(buf, size, "%s-0x%" PRIx32 "(%%%s)",
						         seg_str, -v,
						         get_reg_name(addr_size, mrm & 0x7));
					return 5;
				}
				case 0x4:
					return print_sib2(buf, size, &data[1],
					                  addr_size, seg_str);
			}
			break;
		case 0xC0:
			snprintf(buf, size, "%%%s",
			         get_reg_name(operand_size, mrm & 0x7));
			return 1;
	}
	return 0;
}

static const struct asm_x86_opcode opcodes_0[256] =
{
	[0x00] = {{"add" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x01] = {{"add" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x02] = {{"add" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG_8}},
	[0x03] = {{"add" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x04] = {{"add" }, ASM_X86_OPCODE_EXTRA_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0x05] = {{"add" }, ASM_X86_OPCODE_EXTRA_IMM , 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0x06] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_ES}},
	[0x07] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_ES}},
	[0x08] = {{"or"  }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x09] = {{"or"  }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x0A] = {{"or"  }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG_8}},
	[0x0B] = {{"or"  }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x0C] = {{"or"  }, ASM_X86_OPCODE_EXTRA_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0x0D] = {{"or"  }, ASM_X86_OPCODE_EXTRA_IMM , 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0x0E] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_CS}},
	[0x10] = {{"adc" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x11] = {{"adc" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x12] = {{"adc" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG_8}},
	[0x13] = {{"adc" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x14] = {{"adc" }, ASM_X86_OPCODE_EXTRA_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0x15] = {{"adc" }, ASM_X86_OPCODE_EXTRA_IMM , 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0x16] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SS}},
	[0x17] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SS}},
	[0x18] = {{"sbb" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x19] = {{"sbb" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x1A] = {{"sbb" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG_8}},
	[0x1B] = {{"sbb" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x1C] = {{"sbb" }, ASM_X86_OPCODE_EXTRA_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0x1D] = {{"sbb" }, ASM_X86_OPCODE_EXTRA_IMM , 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0x1E] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DS}},
	[0x1F] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DS}},
	[0x20] = {{"and" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x21] = {{"and" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x22] = {{"and" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG_8}},
	[0x23] = {{"and" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x24] = {{"and" }, ASM_X86_OPCODE_EXTRA_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0x25] = {{"and" }, ASM_X86_OPCODE_EXTRA_IMM , 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0x27] = {{"daa" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x28] = {{"sub" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x29] = {{"sub" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x2A] = {{"sub" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG_8}},
	[0x2B] = {{"sub" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x2C] = {{"sub" }, ASM_X86_OPCODE_EXTRA_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0x2D] = {{"sub" }, ASM_X86_OPCODE_EXTRA_IMM , 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0x2F] = {{"das" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x30] = {{"xor" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x31] = {{"xor" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x32] = {{"xor" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG_8}},
	[0x33] = {{"xor" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x34] = {{"xor" }, ASM_X86_OPCODE_EXTRA_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0x35] = {{"xor" }, ASM_X86_OPCODE_EXTRA_IMM , 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0x37] = {{"aaa" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x38] = {{"cmp" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x39] = {{"cmp" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x3A] = {{"cmp" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG_8}},
	[0x3B] = {{"cmp" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x3C] = {{"cmp" }, ASM_X86_OPCODE_EXTRA_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0x3D] = {{"cmp" }, ASM_X86_OPCODE_EXTRA_IMM , 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0x40] = {{"inc" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_AX}},
	[0x41] = {{"inc" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_CX}},
	[0x42] = {{"inc" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DX}},
	[0x43] = {{"inc" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_BX}},
	[0x44] = {{"inc" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SP}},
	[0x45] = {{"inc" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_BP}},
	[0x46] = {{"inc" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SI}},
	[0x47] = {{"inc" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DI}},
	[0x48] = {{"dec" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_AX}},
	[0x49] = {{"dec" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_CX}},
	[0x4A] = {{"dec" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DX}},
	[0x4B] = {{"dec" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_BX}},
	[0x4C] = {{"dec" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SP}},
	[0x4D] = {{"dec" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_BP}},
	[0x4E] = {{"dec" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SI}},
	[0x4F] = {{"dec" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DI}},
	[0x50] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_AX}},
	[0x51] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_CX}},
	[0x52] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DX}},
	[0x53] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_BX}},
	[0x54] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SP}},
	[0x55] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_BP}},
	[0x56] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SI}},
	[0x57] = {{"push"}, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DI}},
	[0x58] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_AX}},
	[0x59] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_CX}},
	[0x5A] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DX}},
	[0x5B] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_BX}},
	[0x5C] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SP}},
	[0x5D] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_BP}},
	[0x5E] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_SI}},
	[0x5F] = {{"pop" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_DI}},
	[0x60] = {{"pusha"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x61] = {{"popa"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x68] = {{"pushw", "pushl"}, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_IMM}},
	[0x69] = {{"imul"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x6A] = {{"push"}, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_IMM}},
	[0x6B] = {{"imul"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x6C] = {{"insb"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x6D] = {{"insw", "insd"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x70] = {{"jo"  }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x71] = {{"jno" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x72] = {{"jb"  }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x73] = {{"jae" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x74] = {{"je"  }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x75] = {{"jne" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x76] = {{"jbe" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x77] = {{"ja"  }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x78] = {{"js"  }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x79] = {{"jns" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x7A] = {{"jp"  }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x7B] = {{"jpo" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x7C] = {{"jl"  }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x7D] = {{"jge" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x7E] = {{"jle" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x7F] = {{"jg"  }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0x84] = {{"test"}, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x85] = {{"test"}, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x88] = {{"mov" }, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_REG_8, ASM_X86_OPERAND_MRM}},
	[0x89] = {{"mov" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0x8B] = {{"mov" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x8D] = {{"lea" }, ASM_X86_OPCODE_EXTRA_MRM , 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x90] = {{"nop" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x91] = {{"xchg"}, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_CX, ASM_X86_OPERAND_AX}},
	[0x92] = {{"xchg"}, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_DX, ASM_X86_OPERAND_AX}},
	[0x93] = {{"xchg"}, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_BX, ASM_X86_OPERAND_AX}},
	[0x94] = {{"xchg"}, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_SP, ASM_X86_OPERAND_AX}},
	[0x95] = {{"xchg"}, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_BP, ASM_X86_OPERAND_AX}},
	[0x96] = {{"xchg"}, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_SI, ASM_X86_OPERAND_AX}},
	[0x97] = {{"xchg"}, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_DI, ASM_X86_OPERAND_AX}},
	[0x9A] = {{"lcall"}, ASM_X86_OPCODE_EXTRA_FAR, 2, {ASM_X86_OPERAND_FARSEG, ASM_X86_OPERAND_FARADDR}},
	[0x9C] = {{"pushf"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x9D] = {{"popf"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0x9E] = {{"sahf"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xA4] = {{"movsb"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xA5] = {{"movsw", "movsd"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xA8] = {{"test"}, ASM_X86_OPCODE_EXTRA_IMM8, 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0xA9] = {{"test"}, ASM_X86_OPCODE_EXTRA_IMM , 3, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0xAA] = {{"stosb"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xAB] = {{"stosw", "stosd"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xAC] = {{"ldosb"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xAD] = {{"ldosw", "ldosd"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xB0] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0xB1] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_CL}},
	[0xB2] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_DL}},
	[0xB3] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_BL}},
	[0xB4] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AH}},
	[0xB5] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_CH}},
	[0xB6] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_DH}},
	[0xB7] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_BH}},
	[0xB8] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM , 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0xB9] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM , 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_CX}},
	[0xBA] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM , 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_DX}},
	[0xBB] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM , 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_BX}},
	[0xBC] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM , 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_SP}},
	[0xBD] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM , 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_BP}},
	[0xBE] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM , 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_SI}},
	[0xBF] = {{"mov" }, ASM_X86_OPCODE_EXTRA_IMM , 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_SI}},
	[0xC2] = {{"ret" }, ASM_X86_OPCODE_EXTRA_IMM16, 1, {ASM_X86_OPERAND_IMM}},
	[0xC3] = {{"ret" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xC4] = {{"les" }, ASM_X86_OPCODE_EXTRA_MRM_FAR, 3, {ASM_X86_OPERAND_FARSEG, ASM_X86_OPERAND_FARADDR, ASM_X86_OPERAND_REG}},
	[0xC5] = {{"lds" }, ASM_X86_OPCODE_EXTRA_MRM_FAR, 3, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_FARSEG, ASM_X86_OPERAND_FARADDR}},
	[0xC6] = {{"movb"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	[0xC7] = {{"movw", "movl"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	[0xC8] = {{"enter"}, ASM_X86_OPCODE_EXTRA_IMM16_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_IMM}},
	[0xC9] = {{"leave"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xCA] = {{"lret"}, ASM_X86_OPCODE_EXTRA_IMM16, 1, {ASM_X86_OPERAND_IMM}},
	[0xCB] = {{"lret"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xCC] = {{"int" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_0X3}},
	[0xCD] = {{"int" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_IMM}},
	[0xCE] = {{"into"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xCF] = {{"iret"}, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xE3] = {{"jcxz"}, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0xE4] = {{"in"  }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0xE5] = {{"in"  }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0xE6] = {{"out" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AL}},
	[0xE7] = {{"out" }, ASM_X86_OPCODE_EXTRA_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_AX}},
	[0xE8] = {{"call"}, ASM_X86_OPCODE_EXTRA_IMM , 1, {ASM_X86_OPERAND_REL}},
	[0xE9] = {{"jmp" }, ASM_X86_OPCODE_EXTRA_IMM , 1, {ASM_X86_OPERAND_REL}},
	[0xEA] = {{"ljmp"}, ASM_X86_OPCODE_EXTRA_FAR , 2, {ASM_X86_OPERAND_FARSEG, ASM_X86_OPERAND_FARADDR}},
	[0xEB] = {{"jmp" }, ASM_X86_OPCODE_EXTRA_IMM8, 1, {ASM_X86_OPERAND_REL8}},
	[0xEC] = {{"in"  }, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_AL, ASM_X86_OPERAND_DX}},
	[0xED] = {{"in"  }, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_AX, ASM_X86_OPERAND_DX}},
	[0xEE] = {{"out" }, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_AL, ASM_X86_OPERAND_DX}},
	[0xEF] = {{"out" }, ASM_X86_OPCODE_EXTRA_NONE, 2, {ASM_X86_OPERAND_AX, ASM_X86_OPERAND_DX}},
	[0xF1] = {{"int" }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_0X1}},
	[0xF4] = {{"hlt" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xF5] = {{"cmc" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xF8] = {{"clc" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xF9] = {{"stc" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xFA] = {{"cli" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xFB] = {{"sti" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xFC] = {{"cld" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xFD] = {{"std" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
};

static const struct asm_x86_opcode opcodes_0F[256] =
{
	[0x40] = {{"cmovo" }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x41] = {{"cmovno"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x42] = {{"cmovb" }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x43] = {{"cmovae"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x44] = {{"cmove" }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x45] = {{"cmovne"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x46] = {{"cmovbe"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x47] = {{"cmova" }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x48] = {{"cmovs" }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x49] = {{"cmovns"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x4A] = {{"cmovp" }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x4B] = {{"cmovpo"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x4C] = {{"cmovl" }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x4D] = {{"cmovge"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x4E] = {{"cmovle"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x4F] = {{"cmovg" }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0x80] = {{"jo"    }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x81] = {{"jno"   }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x82] = {{"jb"    }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x83] = {{"jae"   }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x84] = {{"je"    }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x85] = {{"jne"   }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x86] = {{"jbe"   }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x87] = {{"ja"    }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x88] = {{"js"    }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x89] = {{"jns"   }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x8A] = {{"jp"    }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x8B] = {{"jpo"   }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x8C] = {{"jl"    }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x8D] = {{"jge"   }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x8E] = {{"jle"   }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x8F] = {{"jg"    }, ASM_X86_OPCODE_EXTRA_IMM, 1, {ASM_X86_OPERAND_REL}},
	[0x90] = {{"seto"  }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x91] = {{"setno" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x92] = {{"setb"  }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x93] = {{"setae" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x94] = {{"sete"  }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x95] = {{"setne" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x96] = {{"setbe" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x97] = {{"setnbe"}, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x98] = {{"sets"  }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x99] = {{"setns" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x9A] = {{"setp"  }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x9B] = {{"setpo" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x9C] = {{"setnge"}, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x9D] = {{"setnl" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x9E] = {{"setng" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0x9F] = {{"setnle"}, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	[0xA0] = {{"push"  }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_FS}},
	[0xA1] = {{"pop"   }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_FS}},
	[0xA2] = {{"cpuid" }, ASM_X86_OPCODE_EXTRA_NONE, 0, {}},
	[0xA8] = {{"push"  }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_GS}},
	[0xA9] = {{"pop"   }, ASM_X86_OPCODE_EXTRA_NONE, 1, {ASM_X86_OPERAND_GS}},
	[0xAF] = {{"imul"  }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0xB0] = {{"cmpxchg"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0xB1] = {{"cmpxchg"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_REG, ASM_X86_OPERAND_MRM}},
	[0xB6] = {{"movzbl"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0xB7] = {{"movzwl"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0xBD] = {{"bsd"   }, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
	[0xBE] = {{"movsbl"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_MRM, ASM_X86_OPERAND_REG}},
};

static const struct asm_x86_opcode opcodes_80[8] =
{
	{{"add"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"or "}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"adc"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"sbb"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"and"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"sub"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"xor"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"cmp"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_81[8] =
{
	{{"add"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"or "}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"adc"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"sbb"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"and"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"sub"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"xor"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"cmp"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_83[8] =
{
	{{"add"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"or" }, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"adc"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"sbb"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"and"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"sub"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"xor"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"cmp"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_C0[8] =
{
	{{"rol"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"ror"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"rcl"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"rcr"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"shl"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"shr"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{},
	{{"sar"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_C1[8] =
{
	{{"rol"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"ror"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"rcl"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"rcr"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"shl"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{{"shr"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{},
	{{"sar"}, ASM_X86_OPCODE_EXTRA_MRM_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_D0[8] =
{
	{{"rol"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"ror"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"rcl"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"rcr"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"shl"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"shr"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{},
	{{"sar"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_D1[8] =
{
	{{"rol"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"ror"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"rcl"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"rcr"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"shl"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{{"shr"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
	{},
	{{"sar"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_0X1, ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_D2[8] =
{
	{{"rol"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"ror"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"rcl"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"rcr"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"shl"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"shr"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{},
	{{"sar"}, ASM_X86_OPCODE_EXTRA_MRM8, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_D3[8] =
{
	{{"rol"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"ror"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"rcl"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"rcr"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"shl"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{{"shr"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
	{},
	{{"sar"}, ASM_X86_OPCODE_EXTRA_MRM, 2, {ASM_X86_OPERAND_CL, ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_F6[8] =
{
	{{"test"}, ASM_X86_OPCODE_EXTRA_MRM8_IMM8, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{},
	{{"not" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	{{"neg" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	{{"mul" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	{{"imul"}, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	{{"div" }, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	{{"idiv"}, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_F7[8] =
{
	{{"test"}, ASM_X86_OPCODE_EXTRA_MRM_IMM, 2, {ASM_X86_OPERAND_IMM, ASM_X86_OPERAND_MRM}},
	{},
	{{"not" }, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{{"neg" }, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{{"mull"}, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{{"imul"}, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{{"div" }, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{{"idiv"}, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_FE[8] =
{
	{{"inc"}, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
	{{"dec"}, ASM_X86_OPCODE_EXTRA_MRM8, 1, {ASM_X86_OPERAND_MRM}},
};

static const struct asm_x86_opcode opcodes_FF[8] =
{
	{{"inc"  }, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{{"dec"  }, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{{"call" }, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{{"call" }, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_FMRM}},
	{{"jmp"  }, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{{"jmp"  }, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_FMRM}},
	{{"pushl"}, ASM_X86_OPCODE_EXTRA_MRM, 1, {ASM_X86_OPERAND_MRM}},
	{},
};

static void disas_operand(char *buf, size_t size, uint8_t operand,
                          char *mrm, size_t *imms, int mrm_reg,
                          size_t pos, size_t bytes,
                          enum asm_x86_reg_size operand_size)
{
	switch (operand)
	{
		case ASM_X86_OPERAND_REG:
			snprintf(buf, size, "%%%s",
			         get_reg_name(operand_size, mrm_reg));
			break;
		case ASM_X86_OPERAND_REG_8:
			snprintf(buf, size, "%%%s",
			         get_reg_name(ASM_X86_REG_8, mrm_reg));
			break;
		case ASM_X86_OPERAND_MRM:
			strlcpy(buf, mrm, size);
			break;
		case ASM_X86_OPERAND_IMM:
			snprintf(buf, size, "$0x%zx", imms[0]);
			break;
		case ASM_X86_OPERAND_REL:
			snprintf(buf, size, "%zx", pos + bytes + imms[0]);
			break;
		case ASM_X86_OPERAND_REL8:
			snprintf(buf, size, "%zx", pos + bytes + (ssize_t)(int8_t)(uint8_t)imms[0]);
			break;
		case ASM_X86_OPERAND_CS:
			strlcpy(buf, "%cs", size);
			break;
		case ASM_X86_OPERAND_DS:
			strlcpy(buf, "%ds", size);
			break;
		case ASM_X86_OPERAND_ES:
			strlcpy(buf, "%es", size);
			break;
		case ASM_X86_OPERAND_FS:
			strlcpy(buf, "%fs", size);
			break;
		case ASM_X86_OPERAND_GS:
			strlcpy(buf, "%gs", size);
			break;
		case ASM_X86_OPERAND_SS:
			strlcpy(buf, "%ss", size);
			break;
		case ASM_X86_OPERAND_AL:
			strlcpy(buf, "%al", size);
			break;
		case ASM_X86_OPERAND_AX:
			if (operand_size == ASM_X86_REG_32)
				strlcpy(buf, "%eax", size);
			else
				strlcpy(buf, "%ax", size);
			break;
		case ASM_X86_OPERAND_CL:
			strlcpy(buf, "%bl", size);
			break;
		case ASM_X86_OPERAND_CX:
			if (operand_size == ASM_X86_REG_32)
				strlcpy(buf, "%ecx", size);
			else
				strlcpy(buf, "%cx", size);
			break;
		case ASM_X86_OPERAND_DL:
			strlcpy(buf, "%dl", size);
			break;
		case ASM_X86_OPERAND_DX:
			if (operand_size == ASM_X86_REG_32)
				strlcpy(buf, "%edx", size);
			else
				strlcpy(buf, "%dx", size);
			break;
		case ASM_X86_OPERAND_BL:
			strlcpy(buf, "%bl", size);
			break;
		case ASM_X86_OPERAND_BX:
			if (operand_size == ASM_X86_REG_32)
				strlcpy(buf, "%ebx", size);
			else
				strlcpy(buf, "%bx", size);
			break;
		case ASM_X86_OPERAND_AH:
			strlcpy(buf, "%ah", size);
			break;
		case ASM_X86_OPERAND_SP:
			if (operand_size == ASM_X86_REG_32)
				strlcpy(buf, "%esp", size);
			else
				strlcpy(buf, "%sp", size);
			break;
		case ASM_X86_OPERAND_CH:
			strlcpy(buf, "%ch", size);
			break;
		case ASM_X86_OPERAND_BP:
			if (operand_size == ASM_X86_REG_32)
				strlcpy(buf, "%ebp", size);
			else
				strlcpy(buf, "%bp", size);
			break;
		case ASM_X86_OPERAND_DH:
			strlcpy(buf, "%dh", size);
			break;
		case ASM_X86_OPERAND_SI:
			if (operand_size == ASM_X86_REG_32)
				strlcpy(buf, "%esi", size);
			else
				strlcpy(buf, "%si", size);
			break;
		case ASM_X86_OPERAND_BH:
			strlcpy(buf, "%bh", size);
			break;
		case ASM_X86_OPERAND_DI:
			if (operand_size == ASM_X86_REG_32)
				strlcpy(buf, "%edi", size);
			else
				strlcpy(buf, "%di", size);
			break;
		case ASM_X86_OPERAND_0X1:
			strlcpy(buf, "$0x3", size);
			break;
		case ASM_X86_OPERAND_0X3:
			strlcpy(buf, "$0x3", size);
			break;
		case ASM_X86_OPERAND_FARSEG:
			snprintf(buf, size, "$0x%zx", imms[0]);
			break;
		case ASM_X86_OPERAND_FARADDR:
			if (operand_size == ASM_X86_REG_32)
				snprintf(buf, size, "$0x%zx", imms[1]);
			else
				snprintf(buf, size, "$0x%zx", imms[1]);
			break;
		case ASM_X86_OPERAND_FMRM:
			snprintf(buf, size, "*%s", mrm);
			break;
	}
}

int asm_x86_disas(char *buf, size_t size, const uint8_t *data, size_t pos)
{
	const struct asm_x86_opcode *opcode;
	char operands[4][64];
	char mrm[32];
	size_t imms[2] = {0, 0};
	uint8_t mrm_reg = 0;
	size_t bytes = 0;
	enum asm_x86_reg_size operand_size = ASM_X86_REG_32;
	enum asm_x86_reg_size addr_size = ASM_X86_REG_32;
	enum asm_x86_segment segment = ASM_X86_SEGMENT_NONE;
	int lock = 0;
	int repne = 0;
	int rep = 0;

	strlcpy(buf, "", size);
	while (1)
	{
		switch (data[bytes])
		{
			case 0x26:
				segment = ASM_X86_SEGMENT_ES;
				bytes++;
				continue;
			case 0x2E:
				segment = ASM_X86_SEGMENT_CS;
				bytes++;
				continue;
			case 0x36:
				segment = ASM_X86_SEGMENT_SS;
				bytes++;
				continue;
			case 0x3E:
				segment = ASM_X86_SEGMENT_DS;
				bytes++;
				continue;
			case 0x64:
				segment = ASM_X86_SEGMENT_FS;
				bytes++;
				continue;
			case 0x65:
				segment = ASM_X86_SEGMENT_GS;
				bytes++;
				continue;
			case 0x66:
				operand_size = ASM_X86_REG_16;
				bytes++;
				continue;
			case 0x67:
				addr_size = ASM_X86_REG_16;
				bytes++;
				continue;
			case 0xF0:
				lock = 1;
				bytes++;
				continue;
			case 0xF2:
				repne = 1;
				bytes++;
				continue;
			case 0xF3:
				rep = 1;
				bytes++;
				continue;
			default:
				break;
		}
		break;
	}
	switch (data[bytes])
	{
		default:
			opcode = &opcodes_0[data[bytes]];
			bytes++;
			break;
		case 0x0F:
			opcode = &opcodes_0F[data[bytes + 1]];
			bytes += 2;
			break;
		case 0x80:
			opcode = &opcodes_80[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0x81:
			opcode = &opcodes_81[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0x83:
			opcode = &opcodes_83[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xC0:
			opcode = &opcodes_C0[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xC1:
			opcode = &opcodes_C1[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xD0:
			opcode = &opcodes_D0[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xD1:
			opcode = &opcodes_D1[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xD2:
			opcode = &opcodes_D2[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xD3:
			opcode = &opcodes_D3[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xF6:
			opcode = &opcodes_F6[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xF7:
			opcode = &opcodes_F7[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xFE:
			opcode = &opcodes_FE[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
		case 0xFF:
			opcode = &opcodes_FF[(data[bytes + 1] >> 3) & 0x7];
			bytes++;
			break;
	}
	if (!opcode->names[0][0])
		return bytes;
	switch (opcode->extra)
	{
		case ASM_X86_OPCODE_EXTRA_MRM:
		case ASM_X86_OPCODE_EXTRA_MRM_IMM:
		case ASM_X86_OPCODE_EXTRA_MRM_IMM8:
		case ASM_X86_OPCODE_EXTRA_MRM_FAR:
			mrm_reg = (data[bytes] >> 3) & 0x7;
			bytes += print_mrm(mrm, sizeof(mrm), &data[bytes],
			                   operand_size, addr_size, segment);
			break;
	}
	switch (opcode->extra)
	{
		case ASM_X86_OPCODE_EXTRA_MRM8:
		case ASM_X86_OPCODE_EXTRA_MRM8_IMM8:
			mrm_reg = (data[bytes] >> 3) & 0x7;
			bytes += print_mrm(mrm, sizeof(mrm), &data[bytes],
			                   ASM_X86_REG_8, addr_size, segment);
			break;
	}
	switch (opcode->extra)
	{
		case ASM_X86_OPCODE_EXTRA_IMM:
		case ASM_X86_OPCODE_EXTRA_MRM_IMM:
			if (operand_size == ASM_X86_REG_32)
			{
				imms[0] = *(uint32_t*)&data[bytes];
				bytes += 4;
			}
			else
			{
				imms[0] = *(uint16_t*)&data[bytes];
				bytes += 2;
			}
			break;
	}
	switch (opcode->extra)
	{
		case ASM_X86_OPCODE_EXTRA_IMM8:
		case ASM_X86_OPCODE_EXTRA_MRM_IMM8:
		case ASM_X86_OPCODE_EXTRA_MRM8_IMM8:
			imms[0] = *(uint8_t*)&data[bytes];
			bytes += 1;
			break;
	}
	if (opcode->extra == ASM_X86_OPCODE_EXTRA_IMM16)
	{
		imms[0] = *(uint16_t*)&data[bytes];
		bytes += 2;
	}
	switch (opcode->extra)
	{
		case ASM_X86_OPCODE_EXTRA_FAR:
		case ASM_X86_OPCODE_EXTRA_MRM_FAR:
			if (operand_size == ASM_X86_REG_32)
			{
				imms[0] = *(uint32_t*)&data[bytes];
				bytes += 4;
			}
			else
			{
				imms[0] = *(uint16_t*)&data[bytes];
				bytes += 2;
			}
			imms[1] = *(uint16_t*)&data[bytes];
			bytes += 2;
			break;
	}
	if (opcode->extra == ASM_X86_OPCODE_EXTRA_IMM16_IMM8)
	{
		imms[1] = *(uint16_t*)&data[bytes];
		bytes += 2;
		imms[0] = *(uint8_t*)&data[bytes];
		bytes++;
	}
	for (size_t i = 0; i < opcode->operands_count; ++i)
		disas_operand(operands[i], sizeof(operands[i]),
		              opcode->operands[i], mrm, imms,
		              mrm_reg, pos, bytes, operand_size);
	if (lock)
		strlcat(buf, "lock ", size);
	if (rep)
		strlcat(buf, "rep ", size);
	if (repne)
		strlcat(buf, "repne ", size);
	const char *name;
	if (operand_size == ASM_X86_REG_32 && opcode->names[1][0])
		name = opcode->names[1];
	else
		name = opcode->names[0];
	strlcat(buf, name, size);
	for (size_t i = 0; i < opcode->operands_count; ++i)
	{
		strlcat(buf, i ? "," : " ", size);
		strlcat(buf, operands[i], size);
	}
	return bytes;
}
