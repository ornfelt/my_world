#include "nbt.h"

#include <string.h>
#include <stdlib.h>

#define SANITIZE_NUMBER(type_name) \
static int sanitize_##type_name(const struct nbt_sanitize_entry *entry) \
{ \
	if (*entry->tag) \
		return 0; \
	*entry->tag = nbt_tag_new(entry->type, entry->name); \
	if (!*entry->tag) \
		return -1; \
	struct nbt_tag_##type_name *tag_##type_name = (struct nbt_tag_##type_name*)*entry->tag; \
	tag_##type_name->value  = entry->tag_##type_name.default_value; \
	return 1; \
}

SANITIZE_NUMBER(byte);
SANITIZE_NUMBER(short);
SANITIZE_NUMBER(int);
SANITIZE_NUMBER(long);
SANITIZE_NUMBER(float);
SANITIZE_NUMBER(double);

#define SANITIZE_ARRAY(type_name) \
static int sanitize_##type_name(const struct nbt_sanitize_entry *entry) \
{ \
	if (*entry->tag) \
	{ \
		struct nbt_tag_##type_name *tag_##type_name = (struct nbt_tag_##type_name*)*entry->tag; \
		if (tag_##type_name->count != entry->tag_##type_name.default_size) \
		{ \
			free(tag_##type_name->value); \
			tag_##type_name->count = entry->tag_##type_name.default_size; \
			tag_##type_name->value = calloc(sizeof(*tag_##type_name->value), tag_##type_name->count); \
			if (!tag_##type_name->value) \
				return -1; \
		} \
		return 0; \
	} \
	*entry->tag = nbt_tag_new(entry->type, entry->name); \
	if (!*entry->tag) \
		return -1; \
	struct nbt_tag_##type_name *tag_##type_name = (struct nbt_tag_##type_name*)*entry->tag; \
	tag_##type_name->count = entry->tag_##type_name.default_size; \
	tag_##type_name->value = calloc(sizeof(*tag_##type_name->value), tag_##type_name->count); \
	if (!tag_##type_name->value) \
		return -1; \
	return 1; \
}

SANITIZE_ARRAY(byte_array);
SANITIZE_ARRAY(int_array);
SANITIZE_ARRAY(long_array);

static int sanitize_string(const struct nbt_sanitize_entry *entry)
{
	if (*entry->tag)
		return 0;
	*entry->tag = nbt_tag_new(entry->type, entry->name);
	if (!*entry->tag)
		return -1;
	struct nbt_tag_string *tag_string = (struct nbt_tag_string*)*entry->tag;
	tag_string->value = strdup(entry->tag_string.default_value);
	if (!tag_string->value)
		return -1;
	tag_string->count = strlen(tag_string->value);
	return 1;
}

static int sanitize_list(const struct nbt_sanitize_entry *entry)
{
	if (*entry->tag)
	{
		struct nbt_tag_list *tag_list = (struct nbt_tag_list*)*entry->tag;
		if (tag_list->type != entry->tag_list.default_type)
		{
			free(tag_list->tags);
			tag_list->tags = NULL;
			tag_list->count = 0;
			tag_list->type = entry->tag_list.default_type;
		}
		return 0;
	}
	*entry->tag = nbt_tag_new(entry->type, entry->name);
	if (!*entry->tag)
		return -1;
	struct nbt_tag_list *tag_list = (struct nbt_tag_list*)*entry->tag;
	tag_list->type = entry->tag_list.default_type;
	return 1;
}

static int sanitize_compound(const struct nbt_sanitize_entry *entry)
{
	int created = 0;
	if (!*entry->tag)
	{
		*entry->tag = nbt_tag_new(NBT_TAG_COMPOUND, entry->name);
		if (!*entry->tag)
			return -1;
		created = 1;
	}
	if (!nbt_sanitize((struct nbt_tag_compound*)*entry->tag,
	                  entry->tag_compound.sanitize_entries))
		return created ? -1 : 0;
	return created ? 1 : 0;
}

int nbt_sanitize(struct nbt_tag_compound *compound,
                 const struct nbt_sanitize_entry *entries)
{
	for (int32_t i = 0; i < compound->count; ++i)
	{
		struct nbt_tag *tag = compound->tags[i];
		const struct nbt_sanitize_entry *entry;
		for (entry = entries; ; ++entry)
		{
			if (!entry->type)
			{
				entry = NULL;
				break;
			}
			if (strcmp(entry->name, tag->name))
				continue;
			if (entry->type != tag->type)
				entry = NULL;
			break;
		}
		if (entry)
		{
			*entry->tag = tag;
			continue;
		}
		if (!nbt_compound_remove_index(compound, i))
			return 0;
		nbt_tag_delete(tag);
		i--;
	}
	for (const struct nbt_sanitize_entry *entry = entries; entry->type; ++entry)
	{
		switch (entry->type)
		{
			default:
				return 0;

#define SANITIZE_TAG(type, name) \
	case NBT_TAG_##type: \
		switch (sanitize_##name(entry)) \
		{ \
			case -1: \
				nbt_tag_delete(*entry->tag); \
				*entry->tag = NULL; \
				return 0; \
			case 0: \
				break; \
			case 1: \
				if (!nbt_compound_add(compound, *entry->tag)) \
					return 0; \
		} \
		break

			SANITIZE_TAG(BYTE, byte);
			SANITIZE_TAG(SHORT, short);
			SANITIZE_TAG(INT, int);
			SANITIZE_TAG(LONG, long);
			SANITIZE_TAG(FLOAT, float);
			SANITIZE_TAG(DOUBLE, double);
			SANITIZE_TAG(BYTE_ARRAY, byte_array);
			SANITIZE_TAG(INT_ARRAY, int_array);
			SANITIZE_TAG(LONG_ARRAY, long_array);
			SANITIZE_TAG(STRING, string);
			SANITIZE_TAG(LIST, list);
			SANITIZE_TAG(COMPOUND, compound);

#undef SANITIZE_TAG
		}
	}
	return 1;
}
