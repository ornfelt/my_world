#ifndef NBT_H
#define NBT_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif

enum nbt_tag_type
{
	NBT_TAG_END        = 0x0,
	NBT_TAG_BYTE       = 0x1,
	NBT_TAG_SHORT      = 0x2,
	NBT_TAG_INT        = 0x3,
	NBT_TAG_LONG       = 0x4,
	NBT_TAG_FLOAT      = 0x5,
	NBT_TAG_DOUBLE     = 0x6,
	NBT_TAG_BYTE_ARRAY = 0x7,
	NBT_TAG_STRING     = 0x8,
	NBT_TAG_LIST       = 0x9,
	NBT_TAG_COMPOUND   = 0xA,
	NBT_TAG_INT_ARRAY  = 0xB,
	NBT_TAG_LONG_ARRAY = 0xC,
};

struct nbt_tag_op;
struct nbt_stream;

struct nbt_tag
{
	const struct nbt_tag_op *op;
	enum nbt_tag_type type;
	char *name;
	int16_t name_len;
};

struct nbt_tag_end
{
	struct nbt_tag tag;
};

struct nbt_tag_byte
{
	struct nbt_tag tag;
	int8_t value;
};

struct nbt_tag_short
{
	struct nbt_tag tag;
	int16_t value;
};

struct nbt_tag_int
{
	struct nbt_tag tag;
	int32_t value;
};

struct nbt_tag_long
{
	struct nbt_tag tag;
	int64_t value;
};

struct nbt_tag_float
{
	struct nbt_tag tag;
	float value;
};

struct nbt_tag_double
{
	struct nbt_tag tag;
	double value;
};

struct nbt_tag_byte_array
{
	struct nbt_tag tag;
	int8_t *value;
	int32_t count;
};

struct nbt_tag_string
{
	struct nbt_tag tag;
	char *value;
	int16_t count;
};

struct nbt_tag_list
{
	struct nbt_tag tag;
	enum nbt_tag_type type;
	struct nbt_tag **tags;
	int32_t count;
};

struct nbt_tag_compound
{
	struct nbt_tag tag;
	struct nbt_tag **tags;
	int32_t count;
};

struct nbt_tag_int_array
{
	struct nbt_tag tag;
	int32_t *value;
	int32_t count;
};

struct nbt_tag_long_array
{
	struct nbt_tag tag;
	int64_t *value;
	int32_t count;
};

struct nbt_stream *nbt_stream_memory_new(const void *data, size_t size);
struct nbt_stream *nbt_stream_file_new(FILE *fp);
struct nbt_stream *nbt_stream_gzip_new(gzFile fp);
void nbt_stream_delete(struct nbt_stream *stream);
int nbt_stream_write(struct nbt_stream *stream, const void *data, size_t size);
int nbt_stream_read(struct nbt_stream *stream, void *data, size_t size);
int nbt_stream_w8(struct nbt_stream *stream, int8_t v);
int nbt_stream_w16(struct nbt_stream *stream, int16_t v);
int nbt_stream_w32(struct nbt_stream *stream, int32_t v);
int nbt_stream_w64(struct nbt_stream *stream, int64_t v);
int nbt_stream_r8(struct nbt_stream *stream, int8_t *v);
int nbt_stream_r16(struct nbt_stream *stream, int16_t *v);
int nbt_stream_r32(struct nbt_stream *stream, int32_t *v);
int nbt_stream_r64(struct nbt_stream *stream, int64_t *v);

struct nbt_tag *nbt_tag_new(enum nbt_tag_type type, const char *name);
struct nbt_tag *nbt_tag_read(struct nbt_stream *stream);
void nbt_tag_delete(struct nbt_tag *tag);
int nbt_tag_write(struct nbt_tag *tag, struct nbt_stream *stream);
int nbt_tag_write_data(struct nbt_tag *tag, struct nbt_stream *stream);
int nbt_tag_read_data(struct nbt_tag *tag, struct nbt_stream *stream);
size_t nbt_tag_size(struct nbt_tag *tag);
size_t nbt_tag_data_size(struct nbt_tag *tag);
void nbt_tag_print(struct nbt_tag *tag);

int nbt_list_remove(struct nbt_tag_list *list, int32_t index);
int nbt_list_add(struct nbt_tag_list *list, struct nbt_tag *tag);

struct nbt_tag *nbt_compound_get(struct nbt_tag_compound *compound,
                                 const char *name);
int nbt_compound_remove_index(struct nbt_tag_compound *compound, int32_t index);
int nbt_compound_remove(struct nbt_tag_compound *compound, const char *name);
int nbt_compound_add(struct nbt_tag_compound *compound, struct nbt_tag *tag);

struct nbt_sanitize_entry
{
	struct nbt_tag **tag;
	const char *name;
	enum nbt_tag_type type;
	union
	{
		struct
		{
			int8_t default_value;
		} tag_byte;
		struct
		{
			int16_t default_value;
		} tag_short;
		struct
		{
			int32_t default_value;
		} tag_int;
		struct
		{
			int64_t default_value;
		} tag_long;
		struct
		{
			float default_value;
		} tag_float;
		struct
		{
			double default_value;
		} tag_double;
		struct
		{
			int32_t default_size;
		} tag_byte_array;
		struct
		{
			int32_t default_size;
		} tag_int_array;
		struct
		{
			int32_t default_size;
		} tag_long_array;
		struct
		{
			const char *default_value;
		} tag_string;
		struct
		{
			enum nbt_tag_type default_type;
		} tag_list;
		struct
		{
			const struct nbt_sanitize_entry *sanitize_entries;
		} tag_compound;
		struct
		{
			int placeholder;
		} tag_end;
	};
};

#define NBT_SANITIZE(tag, name, type, type_name, default_name, default_value) \
	{ \
		(struct nbt_tag**)tag, \
		name, \
		type, \
		{ \
			.tag_##type_name = \
			{ \
				.default_name = default_value, \
			}, \
		}, \
	}

#define NBT_SANITIZE_END NBT_SANITIZE(NULL, NULL, NBT_TAG_END, end, placeholder, 0)
#define NBT_SANITIZE_BYTE(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_BYTE, byte, default_value, val)
#define NBT_SANITIZE_SHORT(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_SHORT, byte, default_value, val)
#define NBT_SANITIZE_INT(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_INT, int, default_value, val)
#define NBT_SANITIZE_LONG(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_LONG, long, default_value, val)
#define NBT_SANITIZE_FLOAT(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_FLOAT, float, default_value, val)
#define NBT_SANITIZE_DOUBLE(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_DOUBLE, double, default_value, val)
#define NBT_SANITIZE_BYTE_ARRAY(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_BYTE_ARRAY, byte_array, default_size, val)
#define NBT_SANITIZE_INT_ARRAY(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_INT_ARRAY, int_array, default_size, val)
#define NBT_SANITIZE_LONG_ARRAY(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_LONG_ARRAY, long_array, default_size, val)
#define NBT_SANITIZE_STRING(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_STRING, string, default_value, val)
#define NBT_SANITIZE_LIST(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_LIST, list, default_type, val)
#define NBT_SANITIZE_COMPOUND(tag, name, val) \
	NBT_SANITIZE(tag, name, NBT_TAG_COMPOUND, compound, sanitize_entries, val)

int nbt_sanitize(struct nbt_tag_compound *compound,
                 const struct nbt_sanitize_entry *entries);

#ifdef __cplusplus
}
#endif

#endif
