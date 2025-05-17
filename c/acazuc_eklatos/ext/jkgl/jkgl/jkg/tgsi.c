#include "tgsi.h"
#include "jkg.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct parse_ctx
{
	const char *str;
	size_t line;
	size_t column;
	uint32_t *output;
	size_t output_size;
	size_t output_len;
};

static void
skip_spaces(struct parse_ctx *ctx)
{
	while (isspace(*ctx->str))
	{
		if (*ctx->str == '\n')
		{
			ctx->line++;
			ctx->column = 0;
		}
		else
		{
			ctx->column++;
		}
		ctx->str++;
	}
}

static int
get_opcode(struct parse_ctx *ctx,
           const char **opcode,
           size_t *opcode_len)
{
	skip_spaces(ctx);
	*opcode = ctx->str;
	while (isalnum(*ctx->str))
	{
		ctx->column++;
		ctx->str++;
	}
	if (*ctx->str && !isspace(*ctx->str))
		return 1;
	*opcode_len = ctx->str - *opcode;
	return 0;
}

static int
get_processor_type(struct parse_ctx *ctx, enum tgsi_processor_type *typep)
{
	const char *opcode;
	size_t opcode_len;

	if (get_opcode(ctx, &opcode, &opcode_len))
		return 1;
	if (!memcmp(opcode, "VERT", 5))
	{
		*typep = TGSI_PROCESSOR_VERTEX;
		return 0;
	}
	if (!memcmp(opcode, "FRAG", 5))
	{
		*typep = TGSI_PROCESSOR_FRAGMENT;
		return 0;
	}
	return 1;
}

static int
parse_processor_type(struct parse_ctx *ctx)
{
	enum tgsi_processor_type type;

	if (get_processor_type(ctx, &type))
		return 1;
	/* XXX append struct tgsi_processor */
	return 0;
}

static int
parse_declaration(struct parse_ctx *ctx)
{
	return 0;
}

static int
parse_token(struct parse_ctx *ctx)
{
	const char *opcode;
	size_t opcode_len;

	if (get_opcode(ctx, &opcode, &opcode_len))
		return 1;
	if (!memcmp(opcode, "DCL", 4))
	{
		if (parse_declaration(ctx))
			return 1;
	}
	return 1;
}

void
tgsi_assemble(const char *str)
{
	struct parse_ctx ctx;

	memset(&ctx, 0, sizeof(ctx));
	ctx.str = str;
	if (parse_processor_type(&ctx))
		goto err;
	while (1)
	{
		if (parse_token(&ctx))
			goto err;
	}

err:
}
