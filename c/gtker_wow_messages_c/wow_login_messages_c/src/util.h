#ifndef WLM_UTIL_H
#define WLM_UTIL_H
/*
 * Manually written utility functions for login.c
 */

#include "wow_login_messages/wow_login_messages.h"

#include <malloc.h>
#include <memory.h>
#include <stdint.h>
#include <string.h> /* strlen */

#define WLM_CHECK_RETURN_CODE(action)            \
    do                                           \
    {                                            \
        _return_value = action;                  \
        if (_return_value != WLM_RESULT_SUCCESS) \
        {                                        \
            goto cleanup;                        \
        }                                        \
    }                                            \
    while (0)

#define WLM_CHECK_LENGTH(type_size)                                                        \
    stream->index;                                                                         \
    do                                                                                     \
    {                                                                                      \
        if (stream->index + (size_t)(type_size) > stream->length)                          \
        {                                                                                  \
            _return_value = (int)((stream->index + (size_t)(type_size)) - stream->length); \
            goto cleanup;                                                                  \
        }                                                                                  \
        stream->index += (size_t)(type_size);                                              \
    }                                                                                      \
    while (0)

#define SKIP_FORWARD_BYTES(type_size)                                                      \
    do                                                                                     \
    {                                                                                      \
        if (reader->index + (size_t)(type_size) > reader->length)                          \
        {                                                                                  \
            _return_value = (int)((reader->index + (size_t)(type_size)) - reader->length); \
            goto cleanup;                                                                  \
        }                                                                                  \
        reader->index += (size_t)(type_size);                                              \
    }                                                                                      \
    while (0)

static WowLoginResult wlm_read_u8(WowLoginReader* stream, uint8_t* value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(1);

    *value = stream->source[index];

    return WLM_RESULT_SUCCESS;

cleanup:
    return _return_value;
}

#define READ_U8(variable) WLM_CHECK_RETURN_CODE(wlm_read_u8(reader, (uint8_t*)&variable))

static WowLoginResult wlm_read_u16(WowLoginReader* stream, uint16_t* value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(2);

    *value = (uint16_t)stream->source[index] | (uint16_t)stream->source[index + 1] << 8;

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define READ_U16(variable) WLM_CHECK_RETURN_CODE(wlm_read_u16(reader, (uint16_t*)&variable))

static WowLoginResult wlm_read_u32(WowLoginReader* stream, uint32_t* value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(4);

    *value = (uint32_t)stream->source[index] | (uint32_t)stream->source[index + 1] << 8 |
        (uint32_t)stream->source[index + 2] << 16 | (uint32_t)stream->source[index + 3] << 24;

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define READ_U32(variable) WLM_CHECK_RETURN_CODE(wlm_read_u32(reader, (uint32_t*)&variable))

static WowLoginResult wlm_read_u64(WowLoginReader* stream, uint64_t* value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(8);

    *value = stream->source[index] | (uint64_t)stream->source[index + 1] << 8 |
        (uint64_t)stream->source[index + 2] << 16 | (uint64_t)stream->source[index + 3] << 24 |
        (uint64_t)stream->source[index + 4] << 32 | (uint64_t)stream->source[index + 5] << 40 |
        (uint64_t)stream->source[index + 6] << 48 | (uint64_t)stream->source[index + 7] << 56;

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define READ_U64(variable) WLM_CHECK_RETURN_CODE(wlm_read_u64(reader, (uint64_t*)&variable))

static WowLoginResult wlm_read_i32(WowLoginReader* stream, int32_t* value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(4);

    *value = (int32_t)stream->source[index] | (int32_t)stream->source[index + 1] << 8 |
        (int32_t)stream->source[index + 2] << 16 | (int32_t)stream->source[index + 3] << 24;

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define READ_I32(variable) WLM_CHECK_RETURN_CODE(wlm_read_i32(reader, (int32_t*)&variable))

static WowLoginResult wlm_write_u8(WowLoginWriter* stream, const uint8_t value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(1);

    stream->destination[index] = (char)value;

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define WRITE_U8(variable) WLM_CHECK_RETURN_CODE(wlm_write_u8(writer, variable))

static WowLoginResult wlm_write_u16(WowLoginWriter* stream, const uint16_t value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(2);

    stream->destination[index] = (char)value;
    stream->destination[index + 1] = (char)(value >> 8);

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define WRITE_U16(variable) WLM_CHECK_RETURN_CODE(wlm_write_u16(writer, variable))

static WowLoginResult wlm_write_u32(WowLoginWriter* stream, const uint32_t value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(4);

    stream->destination[index] = (char)value;
    stream->destination[index + 1] = (char)(value >> 8);
    stream->destination[index + 2] = (char)(value >> 16);
    stream->destination[index + 3] = (char)(value >> 24);

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define WRITE_U32(variable) WLM_CHECK_RETURN_CODE(wlm_write_u32(writer, variable))

static WowLoginResult wlm_write_u64(WowLoginWriter* stream, const uint64_t value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(8);

    stream->destination[index] = (char)value;
    stream->destination[index + 1] = (char)(value >> 8);
    stream->destination[index + 2] = (char)(value >> 16);
    stream->destination[index + 3] = (char)(value >> 24);
    stream->destination[index + 4] = (char)(value >> 32);
    stream->destination[index + 5] = (char)(value >> 40);
    stream->destination[index + 6] = (char)(value >> 48);
    stream->destination[index + 7] = (char)(value >> 56);

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define WRITE_U64(variable) WLM_CHECK_RETURN_CODE(wlm_write_u64(writer, variable))

static WowLoginResult wlm_write_i32(WowLoginWriter* stream, const int32_t value)
{
    int _return_value;
    const size_t index = WLM_CHECK_LENGTH(4);

    stream->destination[index] = (char)value;
    stream->destination[index + 1] = (char)(value >> 8);
    stream->destination[index + 2] = (char)(value >> 16);
    stream->destination[index + 3] = (char)(value >> 24);

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define WRITE_I32(variable) WLM_CHECK_RETURN_CODE(wlm_write_i32(writer, variable))

static WowLoginResult wlm_read_string(WowLoginReader* stream, char** string)
{
    int _return_value;
    uint8_t length;

    size_t index = WLM_CHECK_LENGTH(1);

    length = stream->source[index];

    index = WLM_CHECK_LENGTH(length);

    *string = malloc(length + 1);
    if (*string == NULL)
    {
        return WLM_RESULT_MALLOC_FAIL;
    }

    memcpy(*string, &stream->source[index], length);
    (*string)[length] = '\0';

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define READ_STRING(variable) WLM_CHECK_RETURN_CODE(wlm_read_string(reader, &variable))

static WowLoginResult wlm_write_string(WowLoginWriter* stream, const char* string)
{
    int _return_value;
    const size_t length = strlen(string);
    const size_t index = WLM_CHECK_LENGTH(1 + length);

    stream->destination[index] = (uint8_t)length;
    memcpy(&stream->destination[index + 1], string, length);

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define WRITE_STRING(variable) WLM_CHECK_RETURN_CODE(wlm_write_string(writer, variable))

static WowLoginResult wlm_read_cstring(WowLoginReader* stream, char** string)
{
    int _return_value;
    const unsigned char* const start = &stream->source[stream->index];
    size_t length = 1;
    size_t index = WLM_CHECK_LENGTH(1);

    while (stream->source[index] != '\0')
    {
        index = WLM_CHECK_LENGTH(1);
        length++;
    }

    *string = malloc(length);
    if (*string == NULL)
    {
        return WLM_RESULT_MALLOC_FAIL;
    }

    memcpy(*string, start, length);

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define READ_CSTRING(variable) WLM_CHECK_RETURN_CODE(wlm_read_cstring(reader, &variable))

static WowLoginResult wlm_write_cstring(WowLoginWriter* stream, const char* string)
{
    int _return_value;
    const size_t length = strlen(string);
    const size_t index = WLM_CHECK_LENGTH(length + 1);

    memcpy(&stream->destination[index], string, length + 1);

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define WRITE_CSTRING(variable) WLM_CHECK_RETURN_CODE(wlm_write_cstring(writer, variable))

static WowLoginResult wlm_read_float(WowLoginReader* stream, float* value)
{
    uint32_t v;
    const WowLoginResult result = wlm_read_u32(stream, &v);

    memcpy(value, &v, sizeof(uint32_t));

    return result;
}

#define READ_FLOAT(variable) WLM_CHECK_RETURN_CODE(wlm_read_float(reader, &variable))

static WowLoginResult wlm_write_float(WowLoginWriter* stream, const float value)
{
    uint32_t v;
    memcpy(&v, &value, sizeof(uint32_t));

    return wlm_write_u32(stream, v);
}

#define WRITE_FLOAT(variable) WLM_CHECK_RETURN_CODE(wlm_write_float(writer, variable))

static WowLoginResult wlm_read_bool8(WowLoginReader* stream, bool* value)
{
    int _return_value = 1;
    uint8_t v = 0;

    WLM_CHECK_RETURN_CODE(wlm_read_u8(stream, &v));

    *value = v == 1 ? true : false;

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define READ_BOOL8(variable) WLM_CHECK_RETURN_CODE(wlm_read_bool8(reader, &variable))

static WowLoginResult wlm_write_bool8(WowLoginWriter* stream, const bool value)
{
    int _return_value = 1;
    const uint8_t v = value ? 1 : 0;

    WLM_CHECK_RETURN_CODE(wlm_write_u8(stream, v));

    return WLM_RESULT_SUCCESS;
cleanup:
    return _return_value;
}

#define WRITE_BOOL8(variable) WLM_CHECK_RETURN_CODE(wlm_write_bool8(writer, variable))

static void wlm_free_string(char** string)
{
    free(*string);

    *string = NULL;
}

#define FREE_STRING(s) wlm_free_string(&s)

#define STRING_SIZE(s) strlen(s)

#define READ_ARRAY(variable, arrayLength, readAction) \
    do                                                \
    {                                                 \
        int i;                                        \
        for (i = 0; i < arrayLength; ++i)             \
        {                                             \
            readAction;                               \
        }                                             \
    }                                                 \
    while (0)

#define WRITE_ARRAY(variable, arrayLength, writeAction) \
    do                                                  \
    {                                                   \
        int i;                                          \
        for (i = 0; i < arrayLength; ++i)               \
        {                                               \
            writeAction;                                \
        }                                               \
    }                                                   \
    while (0)

#undef WLM_CHECK_LENGTH


WOW_LOGIN_MESSAGES_C_EXPORT WowLoginReader wlm_create_reader(const unsigned char* const source, const size_t length)
{
    WowLoginReader reader;
    reader.source = source;
    reader.length = length;
    reader.index = 0;

    return reader;
}

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginWriter wlm_create_writer(unsigned char* destination, const size_t length)
{
    WowLoginWriter writer;
    writer.destination = destination;
    writer.length = length;
    writer.index = 0;

    return writer;
}

WOW_LOGIN_MESSAGES_C_EXPORT const char* wlm_error_code_to_string(const WowLoginResult result)
{
    switch (result)
    {
        case WLM_RESULT_SUCCESS:
            return "Success";
        case WLM_RESULT_INVALID_PARAMETERS:
            return "Invalid parameters";
        case WLM_RESULT_NOT_ENOUGH_BYTES:
            return "Not enough bytes";
        case WLM_RESULT_MALLOC_FAIL:
            return "Malloc fail";
    }

    return "";
}

#endif
