#include "shaders.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define D3DSI_OPCODE_MASK      0x0000FFFF
#define D3DSI_INSTLENGTH_MASK  0x0F000000
#define D3DSI_INSTLENGTH_SHIFT 24

#define D3DSHADER_VERSION_MAJOR(version) (((version) >> 8) & 0xFF)
#define D3DSHADER_VERSION_MINOR(version) (((version) >> 0) & 0xFF)

#define D3DSP_REGTYPE_SHIFT        28
#define D3DSP_REGTYPE_SHIFT2       8
#define D3DSP_REGTYPE_MASK         (0x7 << D3DSP_REGTYPE_SHIFT)
#define D3DSP_REGTYPE_MASK2        0x00001800
#define D3DSP_REGNUM_MASK          0x000007FF
#define D3DSP_SWIZZLE_SHIFT        16
#define D3DSP_SWIZZLE_MASK         (0xFF << D3DSP_SWIZZLE_SHIFT)
#define D3DSP_SRCMOD_SHIFT         24
#define D3DSP_SRCMOD_MASK          (0xF << D3DSP_SRCMOD_SHIFT)
#define D3DSP_TEXTURETYPE_SHIFT    27
#define D3DSP_TEXTURETYPE_MASK     0x78000000
#define D3DSP_DCL_USAGE_SHIFT      0
#define D3DSP_DCL_USAGE_MASK       0x0000000F
#define D3DSP_DCL_USAGEINDEX_SHIFT 16
#define D3DSP_DCL_USAGEINDEX_MASK  0x000F0000
#define D3DSP_WRITEMASK_0          0x00010000
#define D3DSP_WRITEMASK_1          0x00020000
#define D3DSP_WRITEMASK_2          0x00040000
#define D3DSP_WRITEMASK_3          0x00080000
#define D3DSP_WRITEMASK_ALL        (D3DSP_WRITEMASK_0 | D3DSP_WRITEMASK_1 | D3DSP_WRITEMASK_2 | D3DSP_WRITEMASK_3)
#define D3DSP_DSTMOD_SHIFT         20
#define D3DSP_DSTMOD_MASK          (0xF << D3DSP_DSTMOD_SHIFT)

#define D3DSPDM_NONE             0
#define D3DSPDM_SATURATE         1
#define D3DSPDM_PARTIALPRECISION 2
#define D3DSPDM_MSAMPCENTROID    3

#define D3DSPSM_NONE    (0 << D3DSP_SRCMOD_SHIFT
#define D3DSPSM_NEG     (1 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_BIAS    (2 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_BIASNEG (3 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_SIGN    (4 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_SIGNNEG (5 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_COMP    (6 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_X2      (7 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_X2NEG   (8 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_DZ      (9 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_DW      (10 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_ABS     (11 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_ABSNEG  (12 << D3DSP_SRCMOD_SHIFT)
#define D3DSPSM_NOT     (13 << D3DSP_SRCMOD_SHIFT)

#define D3DSPR_TEMP        0
#define D3DSPR_INPUT       1
#define D3DSPR_CONST       2
#define D3DSPR_ADDR        3
#define D3DSPR_TEXTURE     3
#define D3DSPR_RASTOUT     4
#define D3DSPR_ATTROUT     5
#define D3DSPR_TEXCRDOUT   6
#define D3DSPR_OUTPUT      6
#define D3DSPR_CONSTINT    7
#define D3DSPR_COLOROUT    8
#define D3DSPR_DEPTHOUT    9
#define D3DSPR_SAMPLER     10
#define D3DSPR_CONST2      11
#define D3DSPR_CONST3      12
#define D3DSPR_CONST4      13
#define D3DSPR_CONSTBOOL   14
#define D3DSPR_LOOP        15
#define D3DSPR_TEMPFLOAT16 16
#define D3DSPR_MISCTYPE    17
#define D3DSPR_LABEL       18
#define D3DSPR_PREDICATE   19

#define D3DDST_UNKNOWN (0 << D3DSP_TEXTURETYPE_SHIFT)
#define D3DDST_1D      (1 << D3DSP_TEXTURETYPE_SHIFT)
#define D3DDST_2D      (2 << D3DSP_TEXTURETYPE_SHIFT)
#define D3DDST_CUBE    (3 << D3DSP_TEXTURETYPE_SHIFT)
#define D3DDST_VOLUME  (4 << D3DSP_TEXTURETYPE_SHIFT)

#define D3DDECLUSAGE_POSITION     0
#define D3DDECLUSAGE_BLENDWEIGHT  1
#define D3DDECLUSAGE_BLENDINDICES 2
#define D3DDECLUSAGE_NORMAL       3
#define D3DDECLUSAGE_PSIZE        4
#define D3DDECLUSAGE_TEXCOORD     5
#define D3DDECLUSAGE_TANGENT      6
#define D3DDECLUSAGE_BINORMAL     7
#define D3DDECLUSAGE_TESSFACTOR   8
#define D3DDECLUSAGE_POSITIONT    9
#define D3DDECLUSAGE_COLOR        10
#define D3DDECLUSAGE_FOG          11
#define D3DDECLUSAGE_DEPTH        12
#define D3DDECLUSAGE_SAMPLE       13

#define D3DSIO_NOP          0
#define D3DSIO_MOV          1
#define D3DSIO_ADD          2
#define D3DSIO_SUB          3
#define D3DSIO_MAD          4
#define D3DSIO_MUL          5
#define D3DSIO_RCP          6
#define D3DSIO_RSQ          7
#define D3DSIO_DP3          8
#define D3DSIO_DP4          9
#define D3DSIO_MIN          10
#define D3DSIO_MAX          11
#define D3DSIO_SLT          12
#define D3DSIO_SGE          13
#define D3DSIO_EXP          14
#define D3DSIO_LOG          15
#define D3DSIO_LIT          16
#define D3DSIO_DST          17
#define D3DSIO_LRP          18
#define D3DSIO_FRC          19
#define D3DSIO_M4x4         20
#define D3DSIO_M4x3         21
#define D3DSIO_M3x4         22
#define D3DSIO_M3x3         23
#define D3DSIO_M3x2         24
#define D3DSIO_CALL         25
#define D3DSIO_CALLNZ       26
#define D3DSIO_LOOP         27
#define D3DSIO_RET          28
#define D3DSIO_ENDLOOP      29
#define D3DSIO_LABEL        30
#define D3DSIO_DCL          31
#define D3DSIO_POW          32
#define D3DSIO_CRS          33
#define D3DSIO_SGN          34
#define D3DSIO_ABS          35
#define D3DSIO_NRM          36
#define D3DSIO_SINCOS       37
#define D3DSIO_REP          38
#define D3DSIO_ENDREP       39
#define D3DSIO_IF           40
#define D3DSIO_IFC          41
#define D3DSIO_ELSE         42
#define D3DSIO_ENDIF        43
#define D3DSIO_BREAK        44
#define D3DSIO_BREAKC       45
#define D3DSIO_MOVA         46
#define D3DSIO_DEFB         47
#define D3DSIO_DEFI         48
#define D3DSIO_TEXCOORD     64
#define D3DSIO_TEXKILL      65
#define D3DSIO_TEX          66
#define D3DSIO_TEXBEM       67
#define D3DSIO_TEXBEML      68
#define D3DSIO_TEXREG2AR    69
#define D3DSIO_TEXREG2GB    70
#define D3DSIO_TEXM3x2PAD   71
#define D3DSIO_TEXM3x2TEX   72
#define D3DSIO_TEXM3x3PAD   73
#define D3DSIO_TEXM3x3TEX   74
#define D3DSIO_RESERVED0    75
#define D3DSIO_TEXM3x3SPEC  76
#define D3DSIO_TEXM3x3VSPEC 77
#define D3DSIO_EXPP         78
#define D3DSIO_LOGP         79
#define D3DSIO_CND          80
#define D3DSIO_DEF          81
#define D3DSIO_TEXREG2RGB   82
#define D3DSIO_TEXDP3TEX    83
#define D3DSIO_TEXM3x2DEPTH 84
#define D3DSIO_TEXDP3       85
#define D3DSIO_TEXM3x3      86
#define D3DSIO_TEXDEPTH     87
#define D3DSIO_CMP          88
#define D3DSIO_BEM          89
#define D3DSIO_DP2ADD       90
#define D3DSIO_DSX          91
#define D3DSIO_DSY          92
#define D3DSIO_TEXLDD       93
#define D3DSIO_SETP         94
#define D3DSIO_TEXLDL       95
#define D3DSIO_BREAKP       96
#define D3DSIO_PHASE        65533
#define D3DSIO_COMMENT      65534
#define D3DSIO_END          65535

static void append(char **buffer, size_t *buffer_size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

static void append(char **buffer, size_t *buffer_size, const char *fmt, ...)
{
	va_list ap;

	if (*buffer_size <= 1)
		return;
	va_start(ap, fmt);
	size_t len = vsnprintf(*buffer, *buffer_size, fmt, ap);
	if (len >= *buffer_size)
		len = *buffer_size - 1;
	*buffer += len;
	*buffer_size -= len;
	va_end(ap);
}

static const char *decode_swizzle_part(uint8_t swizzle)
{
	switch (swizzle)
	{
		case 0:
			return "x";
		case 1:
			return "y";
		case 2:
			return "z";
		case 3:
			return "w";
	}
	return "";
}

static void decode_swizzle(char **buffer, size_t *buffer_size, uint32_t data)
{
	uint32_t swizzle = (data & D3DSP_SWIZZLE_MASK) >> D3DSP_SWIZZLE_SHIFT;
	if (swizzle == 0xE4) /* .xyzw */
		return;
	uint32_t sx = (swizzle >> 0) & 0x3;
	uint32_t sy = (swizzle >> 2) & 0x3;
	uint32_t sz = (swizzle >> 4) & 0x3;
	uint32_t sw = (swizzle >> 6) & 0x3;
	if (sx == sy && sy == sz && sz == sw)
	{
		append(buffer, buffer_size, "%s", decode_swizzle_part(sx));
		return;
	}
	append(buffer, buffer_size, "%s", decode_swizzle_part(sx));
	append(buffer, buffer_size, "%s", decode_swizzle_part(sy));
	append(buffer, buffer_size, "%s", decode_swizzle_part(sz));
	append(buffer, buffer_size, "%s", decode_swizzle_part(sw));
}

static void decode_write_mask(char **buffer, size_t *buffer_size, uint32_t data)
{
	if ((data & D3DSP_WRITEMASK_ALL) == D3DSP_WRITEMASK_ALL)
		return;
	if (data & D3DSP_WRITEMASK_0)
		append(buffer, buffer_size, "x");
	if (data & D3DSP_WRITEMASK_1)
		append(buffer, buffer_size, "y");
	if (data & D3DSP_WRITEMASK_2)
		append(buffer, buffer_size, "z");
	if (data & D3DSP_WRITEMASK_3)
		append(buffer, buffer_size, "w");
}

static void decode_operand(char **buffer, size_t *buffer_size, uint32_t data, const char *swizzle)
{
	uint32_t regtype = ((data & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) | ((data & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2);
	uint32_t regnum = data & D3DSP_REGNUM_MASK;
	switch (regtype)
	{
		case D3DSPR_TEMP:
			append(buffer, buffer_size, "r%" PRIu32, regnum);
			if (swizzle[0])
				append(buffer, buffer_size, ".%s", swizzle);
			break;
		case D3DSPR_INPUT:
			append(buffer, buffer_size, "v%" PRIu32, regnum);
			if (swizzle[0])
				append(buffer, buffer_size, ".%s", swizzle);
			break;
		case D3DSPR_CONST:
			append(buffer, buffer_size, "c%" PRIu32, regnum);
			if (swizzle[0])
				append(buffer, buffer_size, ".%s", swizzle);
			break;
		case D3DSPR_TEXTURE:
			append(buffer, buffer_size, "t%" PRIu32, regnum);
			break;
		case D3DSPR_RASTOUT:
			append(buffer, buffer_size, "oFog%" PRIu32, regnum);
			if (swizzle[0])
				append(buffer, buffer_size, ".%s", swizzle);
			break;
		case D3DSPR_ATTROUT:
			append(buffer, buffer_size, "oPos%" PRIu32, regnum);
			if (swizzle[0])
				append(buffer, buffer_size, ".%s", swizzle);
			break;
		case D3DSPR_TEXCRDOUT:
			append(buffer, buffer_size, "oT%" PRIu32, regnum);
			if (swizzle[0])
				append(buffer, buffer_size, ".%s", swizzle);
			break;
		case D3DSPR_CONSTINT:
			append(buffer, buffer_size, "i%" PRIu32, regnum);
			if (swizzle[0])
				append(buffer, buffer_size, ".%s", swizzle);
			break;
		case D3DSPR_COLOROUT:
			append(buffer, buffer_size, "oC%" PRIu32, regnum);
			if (swizzle[0])
				append(buffer, buffer_size, ".%s", swizzle);
			break;
		case D3DSPR_DEPTHOUT:
			append(buffer, buffer_size, "oDepth");
			break;
		case D3DSPR_SAMPLER:
			append(buffer, buffer_size, "s%" PRIu32, regnum);
			break;
		case D3DSPR_CONSTBOOL:
			append(buffer, buffer_size, "b%" PRIu32, regnum);
			break;
		case D3DSPR_LOOP:
			append(buffer, buffer_size, "aL");
			break;
		case D3DSPR_LABEL:
			append(buffer, buffer_size, "l%" PRIu32, regnum);
			break;
		case D3DSPR_PREDICATE:
			append(buffer, buffer_size, "p%" PRIu32, regnum);
			break;
		default:
			append(buffer, buffer_size, "unk(%" PRIu32 ")%" PRIu32, regtype, regnum);
	}
}

static void decode_operand_src(char **buffer, size_t *buffer_size, uint32_t data, bool display_swizzle)
{
	uint32_t mod = data & D3DSP_SRCMOD_MASK;
	switch (mod)
	{
		case D3DSPSM_NEG:
		case D3DSPSM_BIASNEG:
		case D3DSPSM_SIGNNEG:
		case D3DSPSM_X2NEG:
		case D3DSPSM_ABSNEG:
			append(buffer, buffer_size, "-");
			break;
		case D3DSPSM_COMP:
			append(buffer, buffer_size, "~");
			break;
		case D3DSPSM_NOT:
			append(buffer, buffer_size, "!");
			break;
	}
	switch (mod)
	{
		case D3DSPSM_ABS:
		case D3DSPSM_ABSNEG:
			append(buffer, buffer_size, "abs(");
			break;
	}
	char swizzle_buf[8] = "";
	char *swizzle = swizzle_buf;
	size_t swizzle_size = sizeof(swizzle_buf);
	if (display_swizzle)
		decode_swizzle(&swizzle, &swizzle_size, data);
	decode_operand(buffer, buffer_size, data, swizzle_buf);
	switch (mod)
	{
		case D3DSPSM_BIAS:
		case D3DSPSM_BIASNEG:
			append(buffer, buffer_size, "_bias");
			break;
		case D3DSPSM_SIGN:
		case D3DSPSM_SIGNNEG:
			append(buffer, buffer_size, "_bx2");
			break;
		case D3DSPSM_X2:
		case D3DSPSM_X2NEG:
			append(buffer, buffer_size, "_x2");
			break;
		case D3DSPSM_DZ:
			append(buffer, buffer_size, "_dz");
			break;
		case D3DSPSM_DW:
			append(buffer, buffer_size, "_dw");
			break;
		case D3DSPSM_ABS:
		case D3DSPSM_ABSNEG:
			append(buffer, buffer_size, ")");
			break;
	}
}

static void decode_operand_dst(char **buffer, size_t *buffer_size, uint32_t data, bool display_swizzle)
{
	char swizzle_buf[8] = "";
	char *swizzle = swizzle_buf;
	size_t swizzle_size = sizeof(swizzle_buf);
	if (display_swizzle)
		decode_write_mask(&swizzle, &swizzle_size, data);
	decode_operand(buffer, buffer_size, data, swizzle_buf);
}

static void decode_instr(char **buffer, size_t *buffer_size, const char *instr, const uint32_t *data, size_t *pos, size_t size, bool dst, uint8_t src)
{
	append(buffer, buffer_size, "%s", instr);
	if (dst)
	{
		(*pos)++;
		if (*pos >= size)
			return;
		switch ((data[*pos] & D3DSP_DSTMOD_MASK) >> D3DSP_DSTMOD_SHIFT)
		{
			case D3DSPDM_NONE:
				break;
			case D3DSPDM_SATURATE:
				append(buffer, buffer_size, "_SAT");
				break;
			case D3DSPDM_PARTIALPRECISION:
				append(buffer, buffer_size, "_PARTIAL");
				break;
			case D3DSPDM_MSAMPCENTROID:
				append(buffer, buffer_size, "_CENTROID");
				break;
		}
		append(buffer, buffer_size, " ");
		decode_operand_dst(buffer, buffer_size, data[*pos], true);
	}
	for (uint8_t i = 0; i < src; ++i)
	{
		(*pos)++;
		if (*pos >= size)
			return;
		if (dst || i != 0)
			append(buffer, buffer_size, ",");
		append(buffer, buffer_size, " ");
		decode_operand_src(buffer, buffer_size, data[*pos], true);
	}
	(*pos)++;
}

static void decode_defb(char **buffer, size_t *buffer_size, const uint32_t *data, size_t *pos, size_t size)
{
	if (size - *pos < 2)
		return;
	append(buffer, buffer_size, "DEF ");
	(*pos)++;
	decode_operand(buffer, buffer_size, data[*pos], "");
	(*pos)++;
	append(buffer, buffer_size, ", %s", *(uint32_t*)&data[(*pos)++] ? "TRUE" : "FALSE");
}

static void decode_defi(char **buffer, size_t *buffer_size, const uint32_t *data, size_t *pos, size_t size)
{
	if (size - *pos < 5)
		return;
	append(buffer, buffer_size, "DEF ");
	(*pos)++;
	decode_operand(buffer, buffer_size, data[*pos], "");
	(*pos)++;
	append(buffer, buffer_size, ", %" PRIu32, *(int32_t*)&data[(*pos)++]);
	append(buffer, buffer_size, ", %" PRIu32, *(int32_t*)&data[(*pos)++]);
	append(buffer, buffer_size, ", %" PRIu32, *(int32_t*)&data[(*pos)++]);
	append(buffer, buffer_size, ", %" PRIu32, *(int32_t*)&data[(*pos)++]);
}

static void decode_def(char **buffer, size_t *buffer_size, const uint32_t *data, size_t *pos, size_t size)
{
	if (size - *pos < 5)
		return;
	append(buffer, buffer_size, "DEF ");
	(*pos)++;
	decode_operand(buffer, buffer_size, data[*pos], "");
	(*pos)++;
	append(buffer, buffer_size, ", %f", *(float*)&data[(*pos)++]);
	append(buffer, buffer_size, ", %f", *(float*)&data[(*pos)++]);
	append(buffer, buffer_size, ", %f", *(float*)&data[(*pos)++]);
	append(buffer, buffer_size, ", %f", *(float*)&data[(*pos)++]);
}

static void decode_dcl_sampler(char **buffer, size_t *buffer_size, const uint32_t *data, size_t *pos, size_t size)
{
	if (size - *pos < 2)
		return;
	append(buffer, buffer_size, "DCL_");
	(*pos)++;
	switch (data[*pos] & D3DSP_TEXTURETYPE_MASK)
	{
		case D3DDST_UNKNOWN:
			append(buffer, buffer_size, "unknown");
			break;
		case D3DDST_1D:
			append(buffer, buffer_size, "1d");
			break;
		case D3DDST_2D:
			append(buffer, buffer_size, "2d");
			break;
		case D3DDST_CUBE:
			append(buffer, buffer_size, "cube");
			break;
		case D3DDST_VOLUME:
			append(buffer, buffer_size, "volume");
			break;
	}
	(*pos)++;
	append(buffer, buffer_size, " ");
	char mask_buf[8] = "";
	char *mask = mask_buf;
	size_t mask_size = sizeof(mask_buf);
	decode_write_mask(&mask, &mask_size, data[*pos]);
	decode_operand(buffer, buffer_size, data[*pos], mask_buf);
	(*pos)++;
}

static void decode_dcl_attribute(char **buffer, size_t *buffer_size, const uint32_t *data, size_t *pos, size_t size)
{
	if (size - *pos < 2)
		return;
	append(buffer, buffer_size, "DCL_");
	(*pos)++;
	switch ((data[*pos] & D3DSP_DCL_USAGE_MASK) >> D3DSP_DCL_USAGE_SHIFT)
	{
		case D3DDECLUSAGE_POSITION:
			append(buffer, buffer_size, "position");
			break;
		case D3DDECLUSAGE_BLENDWEIGHT:
			append(buffer, buffer_size, "blendweight");
			break;
		case D3DDECLUSAGE_BLENDINDICES:
			append(buffer, buffer_size, "blendindices");
			break;
		case D3DDECLUSAGE_NORMAL:
			append(buffer, buffer_size, "normal");
			break;
		case D3DDECLUSAGE_PSIZE:
			append(buffer, buffer_size, "psize");
			break;
		case D3DDECLUSAGE_TEXCOORD:
			append(buffer, buffer_size, "texcoord");
			break;
		case D3DDECLUSAGE_TANGENT:
			append(buffer, buffer_size, "tangent");
			break;
		case D3DDECLUSAGE_BINORMAL:
			append(buffer, buffer_size, "binormal");
			break;
		case D3DDECLUSAGE_TESSFACTOR:
			append(buffer, buffer_size, "tessfactor");
			break;
		case D3DDECLUSAGE_POSITIONT:
			append(buffer, buffer_size, "posititiont");
			break;
		case D3DDECLUSAGE_COLOR:
			append(buffer, buffer_size, "color");
			break;
		case D3DDECLUSAGE_FOG:
			append(buffer, buffer_size, "fog");
			break;
		case D3DDECLUSAGE_DEPTH:
			append(buffer, buffer_size, "depth");
			break;
		case D3DDECLUSAGE_SAMPLE:
			append(buffer, buffer_size, "sample");
			break;
	}
	append(buffer, buffer_size, "%d", (data[*pos] & D3DSP_DCL_USAGEINDEX_MASK) >> D3DSP_DCL_USAGEINDEX_SHIFT);
	(*pos)++;
	append(buffer, buffer_size, " ");
	char mask_buf[8] = "";
	char *mask = mask_buf;
	size_t mask_size = sizeof(mask_buf);
	decode_write_mask(&mask, &mask_size, data[*pos]);
	decode_operand(buffer, buffer_size, data[*pos], mask_buf);
	(*pos)++;
}

static void decode_dcl(char **buffer, size_t *buffer_size, const uint32_t *data, size_t *pos, size_t size)
{
	if (size - *pos < 2)
		return;
	if ((((data[*pos + 2] & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) | ((data[*pos + 2] & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2)) == D3DSPR_SAMPLER)
		decode_dcl_sampler(buffer, buffer_size, data, pos, size);
	else
		decode_dcl_attribute(buffer, buffer_size, data, pos, size);
}

void decode_dx9_shader(char *buffer, size_t buffer_size, const void *d, size_t s)
{
	if (s < 4)
		return;
	const uint32_t *data = (const uint32_t*)d;
	size_t size = s / 4;
	uint32_t major = D3DSHADER_VERSION_MAJOR(data[0]);
	uint32_t minor = D3DSHADER_VERSION_MINOR(data[0]);
	switch (data[0] & 0xFFFF0000)
	{
		case 0xFFFF0000:
			append(&buffer, &buffer_size, "ps_%d_%d\n", major, minor);
			break;
		case 0xFFFE0000:
			append(&buffer, &buffer_size, "vs_%d_%d\n", major, minor);
			break;
		default:
			return;
	}
	size_t i = 1;
	while (i < size)
	{
		char line_buf[128];
		char *line = line_buf;
		size_t line_size = sizeof(line_buf);
		size_t prev = i;
		uint32_t opcode = data[i] & D3DSI_OPCODE_MASK;
		uint32_t instlen = (data[i] & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT;
		switch (opcode)
		{
#define DECODE_INSTR(name, dst, src) \
			case D3DSIO_##name: \
				decode_instr(&line, &line_size, #name, data, &i, size, dst, src); \
				break

			DECODE_INSTR(NOP, false, 0);
			DECODE_INSTR(MOV, true, 1);
			DECODE_INSTR(ADD, true, 2);
			DECODE_INSTR(SUB, true, 2);
			DECODE_INSTR(MAD, true, 3);
			DECODE_INSTR(MUL, true, 2);
			DECODE_INSTR(RCP, true, 1);
			DECODE_INSTR(RSQ, true, 1);
			DECODE_INSTR(DP3, true, 2);
			DECODE_INSTR(DP4, true, 2);
			DECODE_INSTR(MIN, true, 2);
			DECODE_INSTR(MAX, true, 2);
			DECODE_INSTR(SLT, true, 2);
			DECODE_INSTR(SGE, true, 2);
			DECODE_INSTR(EXP, true, 1);
			DECODE_INSTR(LOG, true, 1);
			DECODE_INSTR(LIT, true, 1);
			DECODE_INSTR(DST, true, 2);
			DECODE_INSTR(LRP, true, 3);
			DECODE_INSTR(FRC, true, 1);
			DECODE_INSTR(M4x4, true, 2);
			DECODE_INSTR(M4x3, true, 2);
			DECODE_INSTR(M3x4, true, 2);
			DECODE_INSTR(M3x3, true, 2);
			DECODE_INSTR(M3x2, true, 2);
			DECODE_INSTR(CALL, false, 1);
			DECODE_INSTR(CALLNZ, false, 2);
			DECODE_INSTR(LOOP, false, 2);
			DECODE_INSTR(RET, false, 0);
			DECODE_INSTR(ENDLOOP, false, 0);
			DECODE_INSTR(LABEL, false, 1);
			case D3DSIO_DCL:
				decode_dcl(&line, &line_size, data, &i, size);
				break;
			DECODE_INSTR(POW, true, 2);
			DECODE_INSTR(CRS, true, 2);
			DECODE_INSTR(SGN, true, 3);
			DECODE_INSTR(ABS, true, 1);
			DECODE_INSTR(NRM, true, 1);
			DECODE_INSTR(SINCOS, true, 3);
			DECODE_INSTR(REP, false, 1);
			DECODE_INSTR(ENDREP, false, 0);
			DECODE_INSTR(IF, false, 1);
			DECODE_INSTR(IFC, false, 2);
			DECODE_INSTR(ELSE, false, 0);
			DECODE_INSTR(ENDIF, false, 0);
			DECODE_INSTR(BREAK, false, 0);
			DECODE_INSTR(BREAKC, false, 2);
			DECODE_INSTR(MOVA, true, 1);
			case D3DSIO_DEFB:
				decode_defb(&line, &line_size, data, &i, size);
				break;
			case D3DSIO_DEFI:
				decode_defi(&line, &line_size, data, &i, size);
				break;
			case D3DSIO_TEXCOORD:
				if (major == 1 && major == 4)
					decode_instr(&line, &line_size, "TEXCOORD", data, &i, size, true, 0);
				else
					decode_instr(&line, &line_size, "TEXCRD", data, &i, size, true, 1);
				break;
			DECODE_INSTR(TEXKILL, true, 0);
			case D3DSIO_TEX:
				if (major >= 2)
					decode_instr(&line, &line_size, "TEXID", data, &i, size, true, 2);
				else if (minor == 4)
					decode_instr(&line, &line_size, "TEXID", data, &i, size, true, 1);
				else
					decode_instr(&line, &line_size, "TEX", data, &i, size, false, 1);
				break;
			DECODE_INSTR(TEXBEM, true, 1);
			DECODE_INSTR(TEXBEML, true, 1);
			DECODE_INSTR(TEXREG2AR, true, 1);
			DECODE_INSTR(TEXREG2GB, true, 1);
			DECODE_INSTR(TEXM3x2PAD, true, 1);
			DECODE_INSTR(TEXM3x2TEX, true, 1);
			DECODE_INSTR(TEXM3x3PAD, true, 1);
			DECODE_INSTR(TEXM3x3TEX, true, 1);
			DECODE_INSTR(TEXM3x3SPEC, true, 2);
			DECODE_INSTR(TEXM3x3VSPEC, true, 1);
			DECODE_INSTR(EXPP, true, 1);
			DECODE_INSTR(LOGP, true, 1);
			DECODE_INSTR(CND, true, 3);
			case D3DSIO_DEF:
				decode_def(&line, &line_size, data, &i, size);
				break;
			DECODE_INSTR(TEXREG2RGB, true, 1);
			DECODE_INSTR(TEXDP3TEX, true, 1);
			DECODE_INSTR(TEXM3x2DEPTH, true, 1);
			DECODE_INSTR(TEXDP3, true, 1);
			DECODE_INSTR(TEXM3x3, true, 1);
			DECODE_INSTR(TEXDEPTH, true, 0);
			DECODE_INSTR(CMP, true, 3);
			DECODE_INSTR(BEM, true, 2);
			DECODE_INSTR(DP2ADD, true, 3);
			DECODE_INSTR(DSX, true, 1);
			DECODE_INSTR(DSY, true, 1);
			DECODE_INSTR(TEXLDD, true, 4);
			DECODE_INSTR(SETP, true, 2);
			DECODE_INSTR(TEXLDL, true, 2);
			DECODE_INSTR(BREAKP, true, 0);
			DECODE_INSTR(PHASE, false, 0);
			/* XXX COMMENT */
			case D3DSIO_END:
				append(&line, &line_size, "END");
				i++;
				break;
			default:
				i++;
				break;
		}
		size_t j;
		for (j = strlen(line_buf); j < 60; ++j)
			line_buf[j] = ' ';
		line_buf[j] = '\0';
		append(&buffer, &buffer_size, "%s; opcode: %#6" PRIx32 ", len: %2" PRIu32 ", val: %#10" PRIx32 "\n", line_buf, opcode, instlen, data[prev]);
	}
}
