#include "shaders.h"
#include "voxel.h"
#include "log.h"

#include <gfx/device.h>

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

static uint8_t *read_file(const char *path, uint32_t *len)
{
	uint8_t *buf = NULL;
	size_t buf_size = 0;
	FILE *file = NULL;
	size_t tmp;

	*len = 0;
	file = fopen(path, "rb");
	if (!file)
	{
		LOG_ERROR("failed to open file \"%s\"", path);
		return NULL;
	}
	do
	{
		if (*len == buf_size)
		{
			buf = (uint8_t*)realloc(buf, buf_size + 4096);
			if (!buf)
			{
				LOG_ERROR("realloc failed");
				goto cleanup;
			}
			buf_size += 4096;
		}
		tmp = fread(buf + *len, 1, buf_size - *len, file);
		*len += tmp;
	} while (tmp > 0 && buf_size < 1024 * 64 && !feof(file));

	if (ferror(file) || !feof(file))
	{
		free(buf);
		buf = NULL;
	}

	if (*len == buf_size)
	{
		buf = (uint8_t*)realloc(buf, buf_size + 1);
		if (!buf)
		{
			LOG_ERROR("realloc failed");
			goto cleanup;
		}
	}

cleanup:
	fclose(file);
	return buf;
}

static int load_shader(gfx_shader_t *shader, const char *name, enum gfx_shader_type type)
{
	int ret = false;
	char fn[1024];
	uint8_t *data = NULL;
	uint32_t data_len;
	const char *type_str;

	switch (type)
	{
		case GFX_SHADER_VERTEX:
			type_str = "vs";
			break;
		case GFX_SHADER_FRAGMENT:
			type_str = "fs";
			break;
		default:
			LOG_ERROR("invalid shader type: %d", (int)type);
			return 0;
	}
	if (snprintf(fn, sizeof(fn), "data/shaders/%s.%s", name, type_str) == sizeof(fn))
	{
		LOG_ERROR("shader path is too long");
		return 0;
	}
	data = read_file(fn, &data_len);
	if (!data)
	{
		ret = -1;
		goto cleanup;
	}
	if (!gfx_create_shader(g_voxel->device, shader, type, data, data_len))
	{
		ret = 0;
		goto cleanup;
	}

	ret = 1;

cleanup:
	free(data);
	return ret;
}

static bool load_shader_state(gfx_shader_state_t *shader_state, const char *name, const struct gfx_shader_attribute *attributes, const struct gfx_shader_constant *constants, const struct gfx_shader_sampler *samplers)
{
	const gfx_shader_t *shaders[2];
	uint32_t shaders_count = 0;
	gfx_shader_t fragment_shader = GFX_SHADER_INIT();
	gfx_shader_t vertex_shader = GFX_SHADER_INIT();
	bool ret = false;

	switch (load_shader(&vertex_shader, name, GFX_SHADER_VERTEX))
	{
		case -1:
		case 0:
			goto cleanup;
		case 1:
			break;
	}
	switch (load_shader(&fragment_shader, name, GFX_SHADER_FRAGMENT))
	{
		case -1:
		case 0:
			goto cleanup;
		case 1:
			break;
	}
	*shader_state = GFX_SHADER_STATE_INIT();
	shaders[shaders_count++] = &vertex_shader;
	shaders[shaders_count++] = &fragment_shader;
	ret = gfx_create_shader_state(g_voxel->device, shader_state, shaders, shaders_count, attributes, constants, samplers);

cleanup:
	gfx_delete_shader(g_voxel->device, &fragment_shader);
	gfx_delete_shader(g_voxel->device, &vertex_shader);
	return ret;
}

static bool build_particles(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{"vs_color", 1},
		{"vs_uv", 2},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{"texture", 0},
		{NULL, 0},
	};
	return load_shader_state(shader_state, "particles", attributes, constants, samplers);
}

static bool build_breaking(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{"vs_uv", 1},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{"texture", 0},
		{NULL, 0},
	};
	return load_shader_state(shader_state, "breaking", attributes, constants, samplers);
}

static bool build_dropped(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{"vs_color", 1},
		{"vs_uv", 2},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{"texture", 0},
		{NULL, 0},
	};
	return load_shader_state(shader_state, "dropped", attributes, constants, samplers);
}

static bool build_focused(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{NULL, 0},
	};
	return load_shader_state(shader_state, "focused", attributes, constants, samplers);
}

static bool build_sunmoon(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{"vs_color", 1},
		{"vs_uv", 2},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{"texture", 0},
		{NULL, 0},
	};
	return load_shader_state(shader_state, "sun_moon", attributes, constants, samplers);
}

static bool build_blocks(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{"vs_color", 1},
		{"vs_uv", 2},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{"texture", 0},
		{NULL, 0},
	};
	return load_shader_state(shader_state, "blocks", attributes, constants, samplers);
}

static bool build_clouds(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{"vs_color", 1},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{NULL, 0},
	};
	return load_shader_state(shader_state, "clouds", attributes, constants, samplers);
}

static bool build_entity(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{"vs_uv", 1},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{"texture", 0},
		{NULL, 0},
	};
	return load_shader_state(shader_state, "entity", attributes, constants, samplers);
}

static bool build_skybox(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{"vs_color", 1},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{NULL, 0},
	};
	return load_shader_state(shader_state, "skybox", attributes, constants, samplers);
}

static bool build_gui(gfx_shader_state_t *shader_state)
{
	static const struct gfx_shader_attribute attributes[] =
	{
		{"vs_position", 0},
		{"vs_color", 1},
		{"vs_uv", 2},
		{NULL, 0}
	};
	static const struct gfx_shader_constant constants[] =
	{
		{"model_block", 1},
		{NULL, 0},
	};
	static const struct gfx_shader_sampler samplers[] =
	{
		{"texture", 0},
		{NULL, 0},
	};
	return load_shader_state(shader_state, "gui", attributes, constants, samplers);
}

struct shaders *shaders_new(void)
{
	struct shaders *shaders = calloc(sizeof(*shaders), 1);
	if (!shaders)
	{
		LOG_ERROR("shaders allocation failed");
		return NULL;
	}

#define BUILD_SHADER(name) \
	do \
	{ \
		LOG_INFO("building shader " #name); \
		if (!build_##name(&shaders->name)) \
			goto err; \
	} while(0)

	BUILD_SHADER(particles);
	BUILD_SHADER(breaking);
	BUILD_SHADER(dropped);
	BUILD_SHADER(focused);
	BUILD_SHADER(sunmoon);
	BUILD_SHADER(blocks);
	BUILD_SHADER(clouds);
	BUILD_SHADER(entity);
	BUILD_SHADER(skybox);
	BUILD_SHADER(gui);
	return shaders;

#undef BUILD_SHADER

err:
	shaders_delete(shaders);
	return NULL;
}

void shaders_delete(struct shaders *shaders)
{
	if (!shaders)
		return;

#define CLEAN_SHADER(name) \
	do \
	{ \
		LOG_INFO("cleaning shader " #name); \
		gfx_delete_shader_state(g_voxel->device, &shaders->name); \
	} while (0)

	CLEAN_SHADER(particles);
	CLEAN_SHADER(breaking);
	CLEAN_SHADER(dropped);
	CLEAN_SHADER(focused);
	CLEAN_SHADER(sunmoon);
	CLEAN_SHADER(blocks);
	CLEAN_SHADER(clouds);
	CLEAN_SHADER(entity);
	CLEAN_SHADER(skybox);
	CLEAN_SHADER(gui);

#undef CLEAN_SHADER

	free(shaders);
}
