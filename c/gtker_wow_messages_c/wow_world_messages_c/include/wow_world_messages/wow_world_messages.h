#ifndef WWM_WOW_WORLD_MESSAGES_H
#define WWM_WOW_WORLD_MESSAGES_H

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

#include <stddef.h>
#include <stdint.h>

#include "wow_world_messages/all.h"

#include "wow_world_messages_c_export.h"

typedef struct
{
    const unsigned char* source;
    size_t length;
    size_t index;
} WowWorldReader;

typedef struct
{
    unsigned char* destination;
    size_t length;
    size_t index;
} WowWorldWriter;

typedef enum
{
    WWM_RESULT_NOT_ENOUGH_BYTES = 1,
    WWM_RESULT_SUCCESS = 0,
    WWM_RESULT_INVALID_PARAMETERS = -1,
    WWM_RESULT_MALLOC_FAIL = -2,
    WWM_RESULT_COMPRESSION_ERROR = -3
} WowWorldResult;

typedef struct
{
    uint32_t amount_of_splines;
    all_Vector3d* splines;
} MonsterMoveSpline;

typedef struct
{
    uint64_t guid;
    char* name;
} NamedGuid;

typedef struct
{
    uint32_t item_random_property_id;
    uint32_t item_suffix_factor;
} VariableItemRandomProperty;

typedef struct
{
    uint16_t lower;
    uint16_t upper;
} WowTwoShorts;

typedef struct
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
} WowBytes;

WOW_WORLD_MESSAGES_C_EXPORT const char* wwm_error_code_to_string(WowWorldResult result);

WOW_WORLD_MESSAGES_C_EXPORT WowWorldReader wwm_create_reader(const unsigned char* const source, const size_t length);

WOW_WORLD_MESSAGES_C_EXPORT WowWorldWriter wwm_create_writer(unsigned char* const destination, const size_t length);

#endif /* WWM_WOW_WORLD_MESSAGES_H */
