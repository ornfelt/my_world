#include "shaders.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#define GL_ZERO                         0x0000
#define GL_NONE                         0x0000
#define GL_FOG                          0x0B60
#define GL_ALPHA                        0x1906
#define GL_TEXTURE0_ARB                 0x84C0
#define GL_TEXTURE1_ARB                 0x84C1
#define GL_TEXTURE2_ARB                 0x84C2
#define GL_TEXTURE3_ARB                 0x84C3
#define GL_TEXTURE4_ARB                 0x84C4
#define GL_TEXTURE5_ARB                 0x84C5
#define GL_TEXTURE6_ARB                 0x84C6
#define GL_TEXTURE7_ARB                 0x84C7
#define GL_CONSTANT_COLOR0_NV           0x852A
#define GL_CONSTANT_COLOR1_NV           0x852B
#define GL_PRIMARY_COLOR_NV             0x852C
#define GL_SECONDARY_COLOR_NV           0x852D
#define GL_SPARE0_NV                    0x852E
#define GL_SPARE1_NV                    0x852F
#define GL_DISCARD_NV                   0x8530
#define GL_E_TIMES_F_NV                 0x8531
#define GL_UNSIGNED_IDENTITY_NV         0x8536
#define GL_UNSIGNED_INVERT_NV           0x8537
#define GL_EXPAND_NORMAL_NV             0x8538
#define GL_EXPAND_NEGATE_NV             0x8539
#define GL_HALF_BIAS_NORMAL_NV          0x853A
#define GL_HALF_BIAS_NEGATE_NV          0x853B
#define GL_SIGNED_IDENTITY_NV           0x853C
#define GL_SIGNED_NEGATE_NV             0x853D
#define GL_SCALE_BY_TWO_NV              0x853E
#define GL_SCALE_BY_FOUR_NV             0x853F
#define GL_SCALE_BY_ONE_HALF_NV         0x8540
#define GL_BIAS_BY_NEGATIVE_ONE_HALF_NV 0x8541

struct register_input
{
	uint32_t source;
	uint32_t mapping;
	uint32_t usage;
};

struct register_output
{
	uint32_t ab;
	uint32_t cd;
	uint32_t sum;
	uint32_t scale;
	uint32_t bias;
	uint8_t ab_dot;
	uint8_t cd_dot;
	uint8_t mux_sum;
};

struct register_combiner
{
	struct register_input input_rgb[4];
	struct register_output output_rgb;
	struct register_input input_alpha[4];
	struct register_output output_alpha;
	float constant_color0[4];
	float constant_color1[4];
};

struct register_shader
{
	/* 0x000 */ uint32_t magic;
	/* 0x004 */ uint32_t count;
	/* 0x008 */ uint8_t clamp_color;
	/* 0x009 */ uint8_t per_stage_constants;
	/* 0x00C */ struct register_combiner combiners[8];
	/* 0x58C */ struct register_input inputs[7];
	/* 0x5E0 */ float constant_color0[4];
	/* 0x5F0 */ float constant_color1[4];
	/* 0x600 */ uint8_t unk[0x14];
};

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

static void decode_register(char **buffer, size_t *buffer_size, uint32_t reg)
{
	switch (reg)
	{
		case GL_ZERO:
			append(buffer, buffer_size, "vec4(0)");
			return;
		case GL_CONSTANT_COLOR0_NV:
			append(buffer, buffer_size, "color0");
			return;
		case GL_CONSTANT_COLOR1_NV:
			append(buffer, buffer_size, "color1");
			return;
		case GL_FOG:
			append(buffer, buffer_size, "fog");
			return;
		case GL_PRIMARY_COLOR_NV:
			append(buffer, buffer_size, "primary_color");
			return;
		case GL_SECONDARY_COLOR_NV:
			append(buffer, buffer_size, "secondary_color");
			return;
		case GL_SPARE0_NV:
			append(buffer, buffer_size, "spare0");
			return;
		case GL_SPARE1_NV:
			append(buffer, buffer_size, "spare1");
			return;
		case GL_TEXTURE0_ARB:
		case GL_TEXTURE1_ARB:
		case GL_TEXTURE2_ARB:
		case GL_TEXTURE3_ARB:
		case GL_TEXTURE4_ARB:
		case GL_TEXTURE5_ARB:
		case GL_TEXTURE6_ARB:
		case GL_TEXTURE7_ARB:
			append(buffer, buffer_size, "texture%d", reg - GL_TEXTURE0_ARB);
			return;
		case GL_E_TIMES_F_NV:
			append(buffer, buffer_size, "(E * F)");
			return;
	}
	append(buffer, buffer_size, "unk(%d)", reg);
}

static void decode_register_input(char **buffer, size_t *buffer_size, const struct register_input *input, size_t in, bool alpha)
{
	char val_buf[128];
	char *val = &val_buf[0];
	size_t val_size = sizeof(val_buf);
	append(buffer, buffer_size, "%c", 'A' + (int)in);
	if (alpha)
		append(buffer, buffer_size, ".a = ");
	else
		append(buffer, buffer_size, ".rgb = ");
	decode_register(&val, &val_size, input->source);
	if (input->mapping == GL_ALPHA)
		append(buffer, buffer_size, ".a");
	else
		append(buffer, buffer_size, ".rgb");
	switch (input->mapping)
	{
		case GL_UNSIGNED_IDENTITY_NV:
			append(buffer, buffer_size, "max(0.0, %s)", val_buf);
			break;
		case GL_UNSIGNED_INVERT_NV:
			append(buffer, buffer_size, "1.0 - min(max(%s, 0.0), 1.0)", val_buf);
			break;
		case GL_EXPAND_NORMAL_NV:
			append(buffer, buffer_size, "2.0 * max(0.0, %s) - 1.0", val_buf);
			break;
		case GL_EXPAND_NEGATE_NV:
			append(buffer, buffer_size, "-2.0 * max(0.0, %s) + 1.0", val_buf);
			break;
		case GL_HALF_BIAS_NORMAL_NV:
			append(buffer, buffer_size, "max(0.0, %s) - 0.5", val_buf);
			break;
		case GL_HALF_BIAS_NEGATE_NV:
			append(buffer, buffer_size, "-max(0.0, %s) + 0.5", val_buf);
			break;
		case GL_SIGNED_IDENTITY_NV:
			append(buffer, buffer_size, "%s", val_buf);
			break;
		case GL_SIGNED_NEGATE_NV:
			append(buffer, buffer_size, "-%s", val_buf);
			break;
	}
}

static void decode_register_output(char **buffer, size_t *buffer_size, int gcc, const char *bias, const char *scale, bool alpha)
{
	append(buffer, buffer_size, "min(max(");
	if (bias)
		append(buffer, buffer_size, "(");
	append(buffer, buffer_size, "gcc%d", gcc);
	if (alpha)
		append(buffer, buffer_size, ".a");
	else
		append(buffer, buffer_size, ".rgb");
	if (bias)
		append(buffer, buffer_size, "%s)", bias);
	if (scale)
		append(buffer, buffer_size, "%s", scale);
	append(buffer, buffer_size, ", 1), -1)");
}

static const char *decode_register_gcc(int n, bool alpha)
{
	switch (n)
	{
		case 0:
			if (alpha)
				return "gcc1.a = A.a * B.a";
			return "gcc1.rgb = A.rgb * B.rgb";
		case 1:
			if (alpha)
				return "gcc2.a = C.a * D.a";
			return "gcc2.rgb = vec3(dot(A.rgb, B.rgb))";
		case 2:
			if (alpha)
				return "gcc3.a = gcc1.a + gcc2.a";
			return "gcc3.rgb = C.rgb * D.rgb";
		case 3:
			if (alpha)
				return "gcc4.a = gcc1.a or gcc2.a";
			return "gcc4.rgb = vec3(dot(C.rgb, D.rgb))";
		case 4:
			return "gcc5.rgb = gcc1.rgb + gcc3.rgb";
		case 5:
			return "gcc6.rgb = gcc1.rgb or gcc3.rgb";
	}
	return "";
}

static void decode_combiner_target(char **buffer, size_t *buffer_size, const struct register_input *inputs, const struct register_output *output, bool alpha)
{
	const char *scale;
	const char *bias;
	char out_buf[1024] =  "";
	char *out = &out_buf[0];
	size_t out_size = sizeof(out_buf);
	bool used_gcc[6] = {false};
	bool used_inputs[4] = {false};
	switch (output->scale)
	{
		case GL_NONE:
		default:
			scale = NULL;
			break;
		case GL_SCALE_BY_TWO_NV:
			scale = " * 2";
			break;
		case GL_SCALE_BY_FOUR_NV:
			scale = " * 4";
			break;
		case GL_SCALE_BY_ONE_HALF_NV:
			scale = " / 2";
			break;
	}
	switch (output->bias)
	{
		case GL_NONE:
		default:
			bias = NULL;
			break;
		case GL_BIAS_BY_NEGATIVE_ONE_HALF_NV:
			bias = " - 0.5";
			break;
	}
	if (output->ab != GL_DISCARD_NV)
	{
		decode_register(&out, &out_size, output->ab);
		if (output->ab_dot)
		{
			used_gcc[1] = true;
			append(&out, &out_size, ".rgb = ");
			decode_register_output(&out, &out_size, 2, bias, scale, false);
		}
		else
		{
			if (alpha)
			{
				used_gcc[0] = true;
				append(&out, &out_size, ".a = ");
				decode_register_output(&out, &out_size, 1, bias, scale, true);
			}
			else
			{
				used_gcc[0] = true;
				append(&out, &out_size, ".rgb = ");
				decode_register_output(&out, &out_size, 1, bias, scale, false);
			}
		}
	}
	if (output->cd != GL_DISCARD_NV)
	{
		decode_register(&out, &out_size, output->cd);
		if (output->cd_dot)
		{
			used_gcc[3] = true;
			append(&out, &out_size, ".rgb = ");
			decode_register_output(&out, &out_size, 4, bias, scale, false);
		}
		else
		{
			if (alpha)
			{
				used_gcc[1] = true;
				append(&out, &out_size, ".a = ");
				decode_register_output(&out, &out_size, 2, bias, scale, true);
			}
			else
			{
				used_gcc[2] = true;
				append(&out, &out_size, ".rgb = ");
				decode_register_output(&out, &out_size, 3, bias, scale, false);
			}
		}
	}
	if (output->sum != GL_DISCARD_NV)
	{
		decode_register(&out, &out_size, output->sum);
		if (output->mux_sum)
		{
			if (alpha)
			{
				used_gcc[3] = true;
				append(&out, &out_size, ".a = ");
				decode_register_output(&out, &out_size, 4, bias, scale, true);
			}
			else
			{
				used_gcc[5] = true;
				append(&out, &out_size, ".rgb = ");
				decode_register_output(&out, &out_size, 6, bias, scale, false);
			}
		}
		else
		{
			if (alpha)
			{
				used_gcc[2] = true;
				append(&out, &out_size, ".a = ");
				decode_register_output(&out, &out_size, 3, bias, scale, true);
			}
			else
			{
				used_gcc[4] = true;
				append(&out, &out_size, ".rgb = ");
				decode_register_output(&out, &out_size, 5, bias, scale, false);
			}
		}
	}
	if (!out[0])
		return;
	if (alpha)
	{
		if (used_gcc[2] || used_gcc[3])
		{
			used_gcc[0] = true;
			used_gcc[1] = true;
		}
	}
	else
	{
		if (used_gcc[4] || used_gcc[5])
		{
			used_gcc[0] = true;
			used_gcc[2] = true;
		}
	}
	if (used_gcc[0] || used_gcc[1])
	{
		used_inputs[0] = true;
		used_inputs[1] = true;
	}
	if (used_gcc[2] || used_gcc[3])
	{
		used_inputs[2] = true;
		used_inputs[3] = true;
	}
	if (used_gcc[4] || used_gcc[5])
	{
		used_inputs[0] = true;
		used_inputs[1] = true;
	}
	for (size_t i = 0; i < 4; ++i)
	{
		if (used_inputs[i])
		{
			append(buffer, buffer_size, ";input\n");
			decode_register_input(buffer, buffer_size, &inputs[i], i, alpha);
			append(buffer, buffer_size, "\n");
		}
	}
	for (size_t i = 0; i < 6; ++i)
	{
		if (used_gcc[i])
			append(buffer, buffer_size, ";gcc\n%s\n", decode_register_gcc(i, alpha));
	}
	append(buffer, buffer_size, ";output\n%s\n", out_buf);
}

static void decode_combiner(char **buffer, size_t *buffer_size, const struct register_combiner *combiner)
{
	decode_combiner_target(buffer, buffer_size, &combiner->input_rgb[0], &combiner->output_rgb, false);
	decode_combiner_target(buffer, buffer_size, &combiner->input_alpha[0], &combiner->output_alpha, true);
}

static void decode_final_combiner(char **buffer, size_t *buffer_size, const struct register_input *input, size_t n)
{
	append(buffer, buffer_size, "%c", 'A' + (int)n);
	if (n == 6) /* G */
		append(buffer, buffer_size, ".a");
	else
		append(buffer, buffer_size, ".rgb");
	append(buffer, buffer_size, " = ");
	switch (input->mapping)
	{
		case GL_UNSIGNED_IDENTITY_NV:
			append(buffer, buffer_size, "max(0.0, ");
			break;
		case GL_UNSIGNED_INVERT_NV:
			append(buffer, buffer_size, "1.0 - min(max(");
			break;
	}
	decode_register(buffer, buffer_size, input->source);
	if (input->usage == GL_ALPHA)
		append(buffer, buffer_size, ".a");
	else
		append(buffer, buffer_size, ".rgb");
	switch (input->mapping)
	{
		case GL_UNSIGNED_IDENTITY_NV:
			append(buffer, buffer_size, ")");
			break;
		case GL_UNSIGNED_INVERT_NV:
			append(buffer, buffer_size, ", 0.0), 1.0)");
			break;
	}
}

void decode_nv_register_shader(char *buffer, size_t buffer_size, const void *data, size_t size)
{
	if (size < sizeof(struct register_shader))
	{
		append(&buffer, &buffer_size, "invalid register shader (%d / %d)", (int)size, (int)sizeof(struct register_shader));
		return;
	}
	const struct register_shader *shader = data;
	for (size_t i = 0; i < shader->count; ++i)
	{
		size_t old = buffer_size;
		decode_combiner(&buffer, &buffer_size, &shader->combiners[i]);
		if (buffer_size != old)
			append(&buffer, &buffer_size, "\n");
	}
	for (size_t i = 0; i < 7; ++i)
	{
		size_t old = buffer_size;
		decode_final_combiner(&buffer, &buffer_size, &shader->inputs[i], i);
		if (buffer_size != old)
			append(&buffer, &buffer_size, "\n");
	}
	append(&buffer, &buffer_size, "output.rgba = vec4(min(A.rgb * B.rgb + (1 - A.rgb) * C.rgb + D, 1), G)\n");
}
