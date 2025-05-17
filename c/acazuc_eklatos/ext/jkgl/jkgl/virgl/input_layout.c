#include "virgl.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

int
virgl_input_layout_alloc(struct jkg_ctx *ctx,
                         const struct jkg_input_layout_create_info *create_info,
                         struct jkg_input_layout **input_layoutp)
{
	struct jkg_input_layout *input_layout;
	uint32_t *request;
	int ret;

	input_layout = calloc(1, sizeof(*input_layout));
	if (!input_layout)
		return -ENOMEM;
	ret = virgl_alloc_id(ctx, &input_layout->id);
	if (ret)
		goto err;
	ret = virgl_req_alloc(ctx,
	                      VIRGL_CCMD_CREATE_OBJECT,
	                      VIRGL_OBJECT_VERTEX_ELEMENTS,
	                      VIRGL_OBJ_VERTEX_ELEMENTS_SIZE(create_info->nattribs),
	                      &request);
	if (ret)
		goto err;
	request[VIRGL_OBJ_VERTEX_ELEMENTS_HANDLE] = input_layout->id;
	for (size_t i = 0; i < create_info->nattribs; ++i)
	{
		const struct jkg_input_layout_attrib *attrib = &create_info->attribs[i];
		/* XXX enable */
		request[VIRGL_OBJ_VERTEX_ELEMENTS_V0_SRC_OFFSET(i)] = attrib->offset;
		request[VIRGL_OBJ_VERTEX_ELEMENTS_V0_INSTANCE_DIVISOR(i)] = attrib->divisor;
		request[VIRGL_OBJ_VERTEX_ELEMENTS_V0_VERTEX_BUFFER_INDEX(i)] = attrib->binding;
		request[VIRGL_OBJ_VERTEX_ELEMENTS_V0_SRC_FORMAT(i)] = virgl_get_format(attrib->format);
	}
	*input_layoutp = input_layout;
	return 0;

err:
	if (input_layout->id)
		virgl_free_id(ctx, input_layout->id);
	free(input_layout);
	return ret;
}

int
virgl_input_layout_bind(struct jkg_ctx *ctx,
                        struct jkg_input_layout *input_layout)
{
	return virgl_bind_object(ctx, VIRGL_OBJECT_VERTEX_ELEMENTS, input_layout->id);
}

void
virgl_input_layout_free(struct jkg_ctx *ctx,
                        struct jkg_input_layout *input_layout)
{
	if (virgl_destroy_object(ctx, VIRGL_OBJECT_VERTEX_ELEMENTS, input_layout->id))
		assert(!"failed to destroy rasterizer state");
	virgl_free_id(ctx, input_layout->id);
	free(input_layout);
}
