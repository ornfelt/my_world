#include "nbt.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

static const char *tag_names[] =
{
	[NBT_TAG_END]        = "Tag_End",
	[NBT_TAG_BYTE]       = "Tag_Byte",
	[NBT_TAG_SHORT]      = "Tag_Short",
	[NBT_TAG_INT]        = "Tag_Int",
	[NBT_TAG_LONG]       = "Tag_Long",
	[NBT_TAG_FLOAT]      = "Tag_Float",
	[NBT_TAG_DOUBLE]     = "Tag_Double",
	[NBT_TAG_BYTE_ARRAY] = "Tag_ByteArray",
	[NBT_TAG_INT_ARRAY]  = "Tag_IntArray",
	[NBT_TAG_LONG_ARRAY] = "Tag_LongArray",
	[NBT_TAG_STRING]     = "Tag_String",
	[NBT_TAG_LIST]       = "Tag_List",
	[NBT_TAG_COMPOUND]   = "Tag_Compound",
};

struct nbt_tag_op
{
	void (*destroy)(struct nbt_tag *tag);
	int (*read_data)(struct nbt_tag *tag, struct nbt_stream *stream);
	int (*write_data)(struct nbt_tag *tag, struct nbt_stream *stream);
	size_t (*data_size)(struct nbt_tag *tag);
	void (*print)(struct nbt_tag *tag, size_t indent);
};

static int write_name(struct nbt_tag *tag, struct nbt_stream *stream)
{
	if (!nbt_stream_w16(stream, tag->name_len))
		return 0;
	return nbt_stream_write(stream, tag->name, tag->name_len);
}

static void print_indent(size_t indent)
{
	for (size_t i = 0; i < indent; ++i)
		putchar('\t');
}

static void end_print(struct nbt_tag *tag, size_t indent)
{
	(void)tag;
	print_indent(indent);
	printf("%s()\n", tag_names[tag->type]);
}

static const struct nbt_tag_op end_op =
{
	.print = end_print,
};

#define TAG_NUMBER_DEF(type_name, size) \
static int type_name##_read_data(struct nbt_tag *tag, struct nbt_stream *stream) \
{ \
	struct nbt_tag_##type_name *type_name##_tag = (struct nbt_tag_##type_name*)tag; \
	return nbt_stream_r##size(stream, (int##size##_t*)&type_name##_tag->value); \
} \
static int type_name##_write_data(struct nbt_tag *tag, struct nbt_stream *stream) \
{ \
	struct nbt_tag_##type_name *type_name##_tag = (struct nbt_tag_##type_name*)tag; \
	return nbt_stream_w##size(stream, *(int##size##_t*)&type_name##_tag->value); \
} \
static size_t type_name##_data_size(struct nbt_tag *tag) \
{ \
	struct nbt_tag_##type_name *type_name##_tag = (struct nbt_tag_##type_name*)tag; \
	return sizeof(type_name##_tag->value); \
} \
static void type_name##_print(struct nbt_tag *tag, size_t indent) \
{ \
	struct nbt_tag_##type_name *type_name##_tag = (struct nbt_tag_##type_name*)tag; \
	print_indent(indent); \
	printf("%s(%.*s): %lld\n", \
	       tag_names[tag->type], \
	       (int)tag->name_len, tag->name, \
	       (long long)type_name##_tag->value); \
} \
static const struct nbt_tag_op type_name##_op = \
{ \
	.read_data = type_name##_read_data, \
	.write_data = type_name##_write_data, \
	.data_size = type_name##_data_size, \
	.print = type_name##_print, \
};

TAG_NUMBER_DEF(byte, 8);
TAG_NUMBER_DEF(short, 16);
TAG_NUMBER_DEF(int, 32);
TAG_NUMBER_DEF(long, 64);
TAG_NUMBER_DEF(float, 32);
TAG_NUMBER_DEF(double, 64);

#define TAG_ARRAY_DEF(type_name, size) \
static void type_name##_destroy(struct nbt_tag *tag) \
{ \
	struct nbt_tag_##type_name *type_name##_tag = (struct nbt_tag_##type_name*)tag; \
	free(type_name##_tag->value); \
} \
static int type_name##_read_data(struct nbt_tag *tag, struct nbt_stream *stream) \
{ \
	struct nbt_tag_##type_name *type_name##_tag = (struct nbt_tag_##type_name*)tag; \
	if (!nbt_stream_r32(stream, &type_name##_tag->count)) \
		return 0; \
	type_name##_tag->value = malloc(sizeof(*type_name##_tag->value) * type_name##_tag->count); \
	if (!type_name##_tag->value) \
		return 0; \
	for (int32_t i = 0; i < type_name##_tag->count; ++i) \
	{ \
		if (!nbt_stream_r##size(stream, (int##size##_t*)&type_name##_tag->value[i])) \
			return 0; \
	} \
	return 1; \
} \
static int type_name##_write_data(struct nbt_tag *tag, struct nbt_stream *stream) \
{ \
	struct nbt_tag_##type_name *type_name##_tag = (struct nbt_tag_##type_name*)tag; \
	if (!nbt_stream_w16(stream, type_name##_tag->count)) \
		return 0; \
	for (int32_t i = 0; i < type_name##_tag->count; ++i) \
	{ \
		if (!nbt_stream_w##size(stream, *(int##size##_t*)&type_name##_tag->value[i])) \
			return 0; \
	} \
	return 1; \
} \
static size_t type_name##_data_size(struct nbt_tag *tag) \
{ \
	struct nbt_tag_##type_name *type_name##_tag = (struct nbt_tag_##type_name*)tag; \
	return sizeof(*type_name##_tag->value) * type_name##_tag->count; \
} \
static void type_name##_print(struct nbt_tag *tag, size_t indent) \
{ \
	struct nbt_tag_##type_name *type_name##_tag = (struct nbt_tag_##type_name*)tag; \
	print_indent(indent); \
	printf("%s(%.*s): [%" PRId32 " values]\n", \
	       tag_names[tag->type], \
	       (int)tag->name_len, tag->name, \
	       type_name##_tag->count); \
} \
static const struct nbt_tag_op type_name##_op = \
{ \
	.destroy = type_name##_destroy, \
	.read_data = type_name##_read_data, \
	.write_data = type_name##_write_data, \
	.data_size = type_name##_data_size, \
	.print = type_name##_print, \
};

TAG_ARRAY_DEF(byte_array, 8);
TAG_ARRAY_DEF(int_array, 32);
TAG_ARRAY_DEF(long_array, 64);

static void string_destroy(struct nbt_tag *tag)
{
	struct nbt_tag_string *string_tag = (struct nbt_tag_string*)tag;
	free(string_tag->value);
}

static int string_read_data(struct nbt_tag *tag, struct nbt_stream *stream)
{
	struct nbt_tag_string *string_tag = (struct nbt_tag_string*)tag;
	if (!nbt_stream_r16(stream, &string_tag->count))
		return 0;
	string_tag->value = malloc(sizeof(*string_tag->value) * string_tag->count + 1);
	if (!string_tag->value)
		return 0;
	string_tag->value[string_tag->count] = 0;
	for (int32_t i = 0; i < string_tag->count; ++i)
	{
		if (!nbt_stream_r8(stream, (int8_t*)&string_tag->value[i]))
			return 0;
	}
	return 1;
}

static int string_write_data(struct nbt_tag *tag, struct nbt_stream *stream)
{
	struct nbt_tag_string *string_tag = (struct nbt_tag_string*)tag;
	if (!nbt_stream_w16(stream, string_tag->count))
		return 0;
	for (int32_t i = 0; i < string_tag->count; ++i)
	{
		if (!nbt_stream_w8(stream, string_tag->value[i]))
			return 0;
	}
	return 1;
}

static size_t string_data_size(struct nbt_tag *tag)
{
	struct nbt_tag_string *string_tag = (struct nbt_tag_string*)tag;
	return sizeof(*string_tag->value) * string_tag->count;
}

static void string_print(struct nbt_tag *tag, size_t indent)
{
	struct nbt_tag_string *string_tag = (struct nbt_tag_string*)tag;
	print_indent(indent);
	printf("%s(%.*s): \"%.*s\"\n",
	       tag_names[tag->type],
	       (int)tag->name_len, tag->name,
	       (int)string_tag->count, string_tag->value);
}

static const struct nbt_tag_op string_op =
{
	.destroy = string_destroy,
	.read_data = string_read_data,
	.write_data = string_write_data,
	.data_size = string_data_size,
	.print = string_print,
};

static void list_destroy(struct nbt_tag *tag)
{
	struct nbt_tag_list *list_tag = (struct nbt_tag_list*)tag;
	for (int32_t i = 0; i < list_tag->count; ++i)
		nbt_tag_delete(list_tag->tags[i]);
	free(list_tag->tags);
}

static int list_read_data(struct nbt_tag *tag, struct nbt_stream *stream)
{
	struct nbt_tag_list *list_tag = (struct nbt_tag_list*)tag;
	int8_t type;
	if (!nbt_stream_r8(stream, &type))
		return 0;
	if (type < 0 || type > NBT_TAG_LONG_ARRAY)
		return 0;
	list_tag->type = type;
	if (!nbt_stream_r32(stream, &list_tag->count))
		return 0;
	if (list_tag->count < 0)
		return 0;
	list_tag->tags = calloc(sizeof(*list_tag->tags), list_tag->count);
	if (!list_tag->tags)
		return 0;
	for (int32_t i = 0; i < list_tag->count; ++i)
	{
		struct nbt_tag *child = nbt_tag_new(type, NULL);
		if (!child)
			return 0;
		if (!nbt_tag_read_data(child, stream))
			return 0;
		list_tag->tags[i] = child;
	}
	return 1;
}

static int list_write_data(struct nbt_tag *tag, struct nbt_stream *stream)
{
	struct nbt_tag_list *list_tag = (struct nbt_tag_list*)tag;
	if (!nbt_stream_w8(stream, list_tag->type))
		return 0;
	if (!nbt_stream_w32(stream, list_tag->count))
		return 0;
	for (int32_t i = 0; i < list_tag->count; ++i)
	{
		if (!nbt_tag_write_data(list_tag->tags[i], stream))
			return 0;
	}
	return 1;
}

static size_t list_data_size(struct nbt_tag *tag)
{
	struct nbt_tag_list *list_tag = (struct nbt_tag_list*)tag;
	size_t size = 5;
	for (int32_t i = 0; i < list_tag->count; ++i)
		size += nbt_tag_data_size(list_tag->tags[i]);
	return size;
}

static void list_print(struct nbt_tag *tag, size_t indent)
{
	struct nbt_tag_list *list_tag = (struct nbt_tag_list*)tag;
	print_indent(indent);
	printf("%s(%.*s): [%" PRId32 " %s entries]\n",
	       tag_names[tag->type],
	       (int)tag->name_len, tag->name,
	       list_tag->count,
	       tag_names[list_tag->type]);
}

static const struct nbt_tag_op list_op =
{
	.destroy = list_destroy,
	.read_data = list_read_data,
	.write_data = list_write_data,
	.data_size = list_data_size,
	.print = list_print,
};

static void compound_destroy(struct nbt_tag *tag)
{
	struct nbt_tag_compound *compound_tag = (struct nbt_tag_compound*)tag;
	for (int32_t i = 0; i < compound_tag->count; ++i)
		nbt_tag_delete(compound_tag->tags[i]);
	free(compound_tag->tags);
}

static int compound_read_data(struct nbt_tag *tag, struct nbt_stream *stream)
{
	struct nbt_tag_compound *compound_tag = (struct nbt_tag_compound*)tag;
	while (1)
	{
		struct nbt_tag *child = nbt_tag_read(stream);
		if (!child)
			return 0;
		if (child->type == NBT_TAG_END)
		{
			nbt_tag_delete(child);
			break;
		}
		struct nbt_tag **tags = realloc(compound_tag->tags,
		                                sizeof(*tags)
		                              * (compound_tag->count + 1));
		if (!tags)
		{
			nbt_tag_delete(child);
			return 0;
		}
		compound_tag->tags = tags;
		tags[compound_tag->count++] = child;
	}
	return 1;
}

static int compound_write_data(struct nbt_tag *tag, struct nbt_stream *stream)
{
	struct nbt_tag_compound *compound_tag = (struct nbt_tag_compound*)tag;
	for (int32_t i = 0; i < compound_tag->count; ++i)
	{
		if (!nbt_tag_write(compound_tag->tags[i], stream))
			return 0;
	}
	return nbt_stream_w8(stream, 0);
}

static size_t compound_data_size(struct nbt_tag *tag)
{
	struct nbt_tag_compound *compound_tag = (struct nbt_tag_compound*)tag;
	size_t size = 1;
	for (int32_t i = 0; i < compound_tag->count; ++i)
		size += nbt_tag_size(compound_tag->tags[i]);
	return size;
}

static void compound_print(struct nbt_tag *tag, size_t indent)
{
	struct nbt_tag_compound *compound_tag = (struct nbt_tag_compound*)tag;
	print_indent(indent);
	printf("%s(%.*s): [%" PRId32 " entries]\n",
	       tag_names[tag->type],
	       (int)tag->name_len, tag->name,
	       compound_tag->count);
	print_indent(indent);
	printf("{\n");
	for (int32_t i = 0; i < compound_tag->count; ++i)
		compound_tag->tags[i]->op->print(compound_tag->tags[i], indent + 1);
	print_indent(indent);
	printf("}\n");
}

static const struct nbt_tag_op compound_op =
{
	.destroy = compound_destroy,
	.read_data = compound_read_data,
	.write_data = compound_write_data,
	.data_size = compound_data_size,
	.print = compound_print,
};

static const struct
{
	const struct nbt_tag_op *op;
	size_t struct_size;
} tags_def [] =
{
	[NBT_TAG_END]        = {&end_op       , sizeof(struct nbt_tag_end)},
	[NBT_TAG_BYTE]       = {&byte_op      , sizeof(struct nbt_tag_byte)},
	[NBT_TAG_SHORT]      = {&short_op     , sizeof(struct nbt_tag_short)},
	[NBT_TAG_INT]        = {&int_op       , sizeof(struct nbt_tag_int)},
	[NBT_TAG_LONG]       = {&long_op      , sizeof(struct nbt_tag_long)},
	[NBT_TAG_FLOAT]      = {&float_op     , sizeof(struct nbt_tag_float)},
	[NBT_TAG_DOUBLE]     = {&double_op    , sizeof(struct nbt_tag_double)},
	[NBT_TAG_BYTE_ARRAY] = {&byte_array_op, sizeof(struct nbt_tag_byte_array)},
	[NBT_TAG_STRING]     = {&string_op    , sizeof(struct nbt_tag_string)},
	[NBT_TAG_LIST]       = {&list_op      , sizeof(struct nbt_tag_list)},
	[NBT_TAG_COMPOUND]   = {&compound_op  , sizeof(struct nbt_tag_compound)},
	[NBT_TAG_INT_ARRAY]  = {&int_array_op , sizeof(struct nbt_tag_int_array)},
	[NBT_TAG_LONG_ARRAY] = {&long_array_op, sizeof(struct nbt_tag_long_array)},
};

struct nbt_tag *tag_alloc(enum nbt_tag_type type, char *name, int16_t name_len)
{
	struct nbt_tag *tag = calloc(tags_def[type].struct_size, 1);
	if (!tag)
		return NULL;
	tag->type = type;
	tag->name = name;
	tag->name_len = name_len;
	tag->op = tags_def[type].op;
	return tag;
}

struct nbt_tag *nbt_tag_new(enum nbt_tag_type type, const char *name)
{
	char *name_dup;
	size_t name_len;
	if (name)
	{
		name_dup = strdup(name);
		name_len = strlen(name);
	}
	else
	{
		name_dup = NULL;
		name_len = 0;
	}
	struct nbt_tag *tag = tag_alloc(type, name_dup, name_len);
	if (!tag)
		free(name_dup);
	return tag;
}

struct nbt_tag *nbt_tag_read(struct nbt_stream *stream)
{
	int8_t type;
	if (!nbt_stream_r8(stream, &type))
		return NULL;
	if (type < 0 || type > NBT_TAG_LONG_ARRAY)
		return NULL;
	int16_t name_len;
	char *name;
	if (type)
	{
		if (!nbt_stream_r16(stream, &name_len))
			return NULL;
		if (name_len < 0)
			return NULL;
		name = malloc(name_len + 1);
		if (!name)
			return NULL;
		if (!nbt_stream_read(stream, name, name_len))
		{
			free(name);
			return NULL;
		}
		name[name_len] = 0;
	}
	else
	{
		name_len = 0;
		name = NULL;
	}
	struct nbt_tag *tag = tag_alloc(type, name, name_len);
	if (!tag)
	{
		free(name);
		return NULL;
	}
	if (!nbt_tag_read_data(tag, stream))
	{
		nbt_tag_delete(tag);
		return NULL;
	}
	return tag;
}

void nbt_tag_delete(struct nbt_tag *tag)
{
	if (!tag)
		return;
	if (tag->op->destroy)
		tag->op->destroy(tag);
	free(tag->name);
	free(tag);
}

int nbt_tag_write(struct nbt_tag *tag, struct nbt_stream *stream)
{
	if (!nbt_stream_w8(stream, tag->type))
		return 0;
	if (tag->name && !write_name(tag, stream))
		return 0;
	if (tag->op->write_data && !tag->op->write_data(tag, stream))
		return 0;
	return 1;
}

int nbt_tag_write_data(struct nbt_tag *tag, struct nbt_stream *stream)
{
	if (tag->op->write_data && !tag->op->write_data(tag, stream))
		return 0;
	return 1;
}

int nbt_tag_read_data(struct nbt_tag *tag, struct nbt_stream *stream)
{
	if (tag->op->read_data && !tag->op->read_data(tag, stream))
		return 0;
	return 1;
}

size_t nbt_tag_size(struct nbt_tag *tag)
{
	size_t size = 1;
	if (tag->name)
		size += 2 + tag->name_len;
	if (tag->op->data_size)
		size += tag->op->data_size(tag);
	return size;
}

size_t nbt_tag_data_size(struct nbt_tag *tag)
{
	if (tag->op->data_size)
		return tag->op->data_size(tag);
	return 0;
}

void nbt_tag_print(struct nbt_tag *tag)
{
	tag->op->print(tag, 0);
}

int nbt_list_remove(struct nbt_tag_list *list, int32_t index)
{
	if (index < 0 || index >= list->count)
		return 0;
	memmove(&list->tags[index], &list->tags[index + 1],
	        sizeof(*list->tags) * (list->count - index - 1));
	list->count--;
	return 1;
}

int nbt_list_add(struct nbt_tag_list *list, struct nbt_tag *tag)
{
	if (tag->type != list->type)
		return 0;
	struct nbt_tag **tags = realloc(list->tags, sizeof(*tags) * (list->count + 1));
	if (!tags)
		return 0;
	tags[list->count++] = tag;
	list->tags = tags;
	return 1;
}

struct nbt_tag *nbt_compound_get(struct nbt_tag_compound *compound,
                                 const char *name)
{
	for (int32_t i = 0; i < compound->count; ++i)
	{
		if (!strcmp(name, compound->tags[i]->name))
			return compound->tags[i];
	}
	return NULL;
}

int nbt_compound_remove_index(struct nbt_tag_compound *compound, int32_t index)
{
	if (index < 0 || index >= compound->count)
		return 0;
	memmove(&compound->tags[index], &compound->tags[index + 1],
	        sizeof(*compound->tags) * (compound->count - index - 1));
	compound->count--;
	return 1;
}

int nbt_compound_remove(struct nbt_tag_compound *compound, const char *name)
{
	for (int32_t i = 0; i < compound->count; ++i)
	{
		if (!strcmp(compound->tags[i]->name, name))
			return nbt_compound_remove_index(compound, i);
	}
	return 0;
}

int nbt_compound_add(struct nbt_tag_compound *compound, struct nbt_tag *tag)
{
	struct nbt_tag **tags = realloc(compound->tags, sizeof(*tags) * (compound->count + 1));
	if (!tags)
		return 0;
	tags[compound->count++] = tag;
	compound->tags = tags;
	return 1;
}
