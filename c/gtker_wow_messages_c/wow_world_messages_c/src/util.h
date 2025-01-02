#ifndef WWM_UTIL_H
#define WWM_UTIL_H

#include "wow_world_messages/wow_world_messages.h"

#include <malloc.h>
#include <stdint.h>

#define WWM_CHECK_RETURN_CODE(action)            \
    do                                           \
    {                                            \
        _return_value = action;                  \
        if (_return_value != WWM_RESULT_SUCCESS) \
        {                                        \
            goto cleanup;                        \
        }                                        \
    }                                            \
    while (0)

#define SKIP_FORWARD_BYTES(type_size)                                                  \
    if (reader->index + (size_t)(type_size) > reader->length)                          \
    {                                                                                  \
        _return_value = (int)((reader->index + ((size_t)type_size)) - reader->length); \
        goto cleanup;                                                                  \
    }                                                                                  \
    reader->index += (size_t)(type_size)

WowWorldResult wwm_read_u8(WowWorldReader* stream, uint8_t* value);

#define READ_U8(variable) WWM_CHECK_RETURN_CODE(wwm_read_u8(reader, (uint8_t*)&variable))

WowWorldResult wwm_read_u16(WowWorldReader* stream, uint16_t* value);

#define READ_U16(variable) WWM_CHECK_RETURN_CODE(wwm_read_u16(reader, (uint16_t*)&variable))

WowWorldResult wwm_read_u16_be(WowWorldReader* stream, uint16_t* value);

#define READ_U16_BE(variable) WWM_CHECK_RETURN_CODE(wwm_read_u16_be(reader, (uint16_t*)&variable))

WowWorldResult wwm_read_u32(WowWorldReader* stream, uint32_t* value);

#define READ_U32(variable) WWM_CHECK_RETURN_CODE(wwm_read_u32(reader, (uint32_t*)&variable))

WowWorldResult wwm_read_u48(WowWorldReader* stream, uint64_t* value);

#define READ_U48(variable) WWM_CHECK_RETURN_CODE(wwm_read_u48(reader, (uint64_t*)&variable))

WowWorldResult wwm_read_u64(WowWorldReader* stream, uint64_t* value);

#define READ_U64(variable) WWM_CHECK_RETURN_CODE(wwm_read_u64(reader, (uint64_t*)&variable))

WowWorldResult wwm_read_i32(WowWorldReader* stream, int32_t* value);

#define READ_I32(variable) WWM_CHECK_RETURN_CODE(wwm_read_i32(reader, (int32_t*)&variable))

WowWorldResult wwm_write_u8(WowWorldWriter* stream, uint8_t value);

#define WRITE_U8(variable) WWM_CHECK_RETURN_CODE(wwm_write_u8(writer, variable))

WowWorldResult wwm_write_u16(WowWorldWriter* stream, uint16_t value);

#define WRITE_U16(variable) WWM_CHECK_RETURN_CODE(wwm_write_u16(writer, variable))

WowWorldResult wwm_write_u16_be(WowWorldWriter* stream, uint16_t value);

#define WRITE_U16_BE(variable) WWM_CHECK_RETURN_CODE(wwm_write_u16_be(writer, variable))

WowWorldResult wwm_write_u32(WowWorldWriter* stream, uint32_t value);

#define WRITE_U32(variable) WWM_CHECK_RETURN_CODE(wwm_write_u32(writer, variable))

WowWorldResult wwm_write_u48(WowWorldWriter* stream, uint64_t value);

#define WRITE_U48(variable) WWM_CHECK_RETURN_CODE(wwm_write_u48(writer, variable))

WowWorldResult wwm_write_u64(WowWorldWriter* stream, uint64_t value);

#define WRITE_U64(variable) WWM_CHECK_RETURN_CODE(wwm_write_u64(writer, variable))

WowWorldResult wwm_write_i32(WowWorldWriter* stream, int32_t value);

#define WRITE_I32(variable) WWM_CHECK_RETURN_CODE(wwm_write_i32(writer, variable))

WowWorldResult wwm_read_string(WowWorldReader* stream, char** string);

#define READ_STRING(variable) WWM_CHECK_RETURN_CODE(wwm_read_string(reader, &variable));

WowWorldResult wwm_write_string(WowWorldWriter* stream, const char* string);

#define WRITE_STRING(variable) WWM_CHECK_RETURN_CODE(wwm_write_string(writer, variable))

WowWorldResult wwm_read_cstring(WowWorldReader* stream, char** string);

#define READ_CSTRING(variable) WWM_CHECK_RETURN_CODE(wwm_read_cstring(reader, &variable));

WowWorldResult wwm_write_cstring(WowWorldWriter* stream, const char* string);

#define WRITE_CSTRING(variable) WWM_CHECK_RETURN_CODE(wwm_write_cstring(writer, variable))

WowWorldResult wwm_read_sized_cstring(WowWorldReader* stream, char** string);

#define READ_SIZED_CSTRING(variable) WWM_CHECK_RETURN_CODE(wwm_read_sized_cstring(reader, &variable));

WowWorldResult wwm_write_sized_cstring(WowWorldWriter* stream, const char* string);

#define WRITE_SIZED_CSTRING(variable) WWM_CHECK_RETURN_CODE(wwm_write_sized_cstring(writer, variable))

WowWorldResult wwm_read_float(WowWorldReader* stream, float* value);

#define READ_FLOAT(variable) WWM_CHECK_RETURN_CODE(wwm_read_float(reader, &variable))

WowWorldResult wwm_write_float(WowWorldWriter* stream, float value);

#define WRITE_FLOAT(variable) WWM_CHECK_RETURN_CODE(wwm_write_float(writer, variable))

WowWorldResult wwm_read_bool8(WowWorldReader* stream, bool* value);

#define READ_BOOL8(variable) WWM_CHECK_RETURN_CODE(wwm_read_bool8(reader, &variable))

WowWorldResult wwm_write_bool8(WowWorldWriter* stream, bool value);

#define WRITE_BOOL8(variable) WWM_CHECK_RETURN_CODE(wwm_write_bool8(writer, variable))

WowWorldResult wwm_read_bool32(WowWorldReader* stream, bool* value);

#define READ_BOOL32(variable) WWM_CHECK_RETURN_CODE(wwm_read_bool32(reader, &variable))

WowWorldResult wwm_write_bool32(WowWorldWriter* stream, bool value);

#define WRITE_BOOL32(variable) WWM_CHECK_RETURN_CODE(wwm_write_bool32(writer, variable))

void wwm_free_string(char** string);

#define FREE_STRING(s) wwm_free_string(&s)

#define STRING_SIZE(s) strlen(s)

size_t wwm_packed_guid_size(uint64_t);

WowWorldResult wwm_read_packed_guid(WowWorldReader* stream, uint64_t* value);

#define READ_PACKED_GUID(variable) WWM_CHECK_RETURN_CODE(wwm_read_packed_guid(reader, &variable))

WowWorldResult wwm_write_packed_guid(WowWorldWriter* stream, uint64_t value);

#define WRITE_PACKED_GUID(variable) WWM_CHECK_RETURN_CODE(wwm_write_packed_guid(writer, variable))

WowWorldResult wwm_read_monster_move_spline(WowWorldReader* stream, MonsterMoveSpline* value);

#define READ_MONSTER_MOVE_SPLINE(variable) WWM_CHECK_RETURN_CODE(wwm_read_monster_move_spline(reader, &variable))

WowWorldResult wwm_write_monster_move_spline(WowWorldWriter* stream, const MonsterMoveSpline* value);

#define WRITE_MONSTER_MOVE_SPLINE(variable) WWM_CHECK_RETURN_CODE(wwm_write_monster_move_spline(writer, &variable))

size_t wwm_monster_move_spline_size(const MonsterMoveSpline* value);

void wwm_monster_move_spline_free(const MonsterMoveSpline* value);

WowWorldResult wwm_read_named_guid(WowWorldReader* stream, NamedGuid* value);
#define READ_NAMED_GUID(variable) WWM_CHECK_RETURN_CODE(wwm_read_named_guid(reader, &variable))
WowWorldResult wwm_write_named_guid(WowWorldWriter* stream, const NamedGuid* value);
#define WRITE_NAMED_GUID(variable) WWM_CHECK_RETURN_CODE(wwm_write_named_guid(writer, &variable))
size_t wwm_named_guid_size(const NamedGuid* value);
void wwm_named_guid_free(NamedGuid* value);

WowWorldResult wwm_read_variable_item_random_property(WowWorldReader* reader, VariableItemRandomProperty* value);
#define READ_VARIABLE_ITEM_RANDOM_PROPERTY(variable) \
    WWM_CHECK_RETURN_CODE(wwm_read_variable_item_random_property(reader, &variable))
WowWorldResult wwm_write_variable_item_random_property(WowWorldWriter* writer, const VariableItemRandomProperty* value);
#define WRITE_VARIABLE_ITEM_RANDOM_PROPERTY(variable) \
    WWM_CHECK_RETURN_CODE(wwm_write_variable_item_random_property(writer, &variable))
size_t wwm_variable_item_random_property_size(const VariableItemRandomProperty* value);

#define READ_ARRAY(variable, arrayLength, readAction) \
    do                                                \
    {                                                 \
        int i;                                        \
        for (i = 0; i < (int)arrayLength; ++i)        \
        {                                             \
            readAction;                               \
        }                                             \
    }                                                 \
    while (0)

#define WRITE_ARRAY(variable, arrayLength, writeAction) \
    do                                                  \
    {                                                   \
        int i;                                          \
        for (i = 0; i < (int)arrayLength; ++i)          \
        {                                               \
            writeAction;                                \
        }                                               \
    }                                                   \
    while (0)

#define WWM_COMPRESS_EXTRA_LENGTH 11

size_t wwm_compress_data_size(const unsigned char* src, size_t src_length);
size_t wwm_compress_data(const unsigned char* src, size_t src_length, unsigned char* dst, size_t dst_length);
size_t wwm_decompress_data(const unsigned char* src, size_t src_length, unsigned char* dst, size_t dst_length);

void wwm_update_mask_set_u32(uint32_t* headers, uint32_t* values, uint32_t offset, uint32_t value);
uint32_t wwm_update_mask_get_u32(const uint32_t* headers, const uint32_t* values, uint32_t offset);

void wwm_update_mask_set_u64(uint32_t* headers, uint32_t* values, uint32_t offset, uint64_t value);
uint64_t wwm_update_mask_get_u64(const uint32_t* headers, const uint32_t* values, uint32_t offset);

void wwm_update_mask_set_float(uint32_t* headers, uint32_t* values, uint32_t offset, float value);
float wwm_update_mask_get_float(const uint32_t* headers, const uint32_t* values, uint32_t offset);

void wwm_update_mask_set_two_shorts(uint32_t* headers, uint32_t* values, uint32_t offset, WowTwoShorts value);
WowTwoShorts wwm_update_mask_get_two_shorts(const uint32_t* headers, const uint32_t* values, uint32_t offset);

void wwm_update_mask_set_bytes(uint32_t* headers, uint32_t* values, uint32_t offset, WowBytes value);
WowBytes wwm_update_mask_get_bytes(const uint32_t* headers, const uint32_t* values, uint32_t offset);

#endif
