#include "soft.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

int
soft_input_layout_alloc(struct jkg_ctx *ctx,
                        const struct jkg_input_layout_create_info *create_info,
                        struct jkg_input_layout **input_layoutp)
{
	struct jkg_input_layout *input_layout;

	(void)ctx;
	input_layout = malloc(sizeof(*input_layout));
	if (!input_layout)
		return -ENOMEM;
	memcpy(input_layout->attribs,
	       create_info->attribs,
	       sizeof(*input_layout->attribs) * create_info->nattribs);
	input_layout->nattribs = create_info->nattribs;
	*input_layoutp = input_layout;
	return 0;
}

int
soft_input_layout_bind(struct jkg_ctx *ctx,
                       struct jkg_input_layout *input_layout)
{
	ctx->input_layout = input_layout;
	return 0;
}

void
soft_input_layout_free(struct jkg_ctx *ctx,
                       struct jkg_input_layout *input_layout)
{
	(void)ctx;
	free(input_layout);
}
