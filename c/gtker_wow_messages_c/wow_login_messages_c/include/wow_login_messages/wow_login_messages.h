#ifndef WOW_LOGIN_MESSAGES_H
#define WOW_LOGIN_MESSAGES_H

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

#include <stddef.h>
#include <stdint.h>

#include "wow_login_messages_c_export.h"

typedef struct
{
    const unsigned char* source;
    size_t length;
    size_t index;
} WowLoginReader;

typedef struct
{
    unsigned char* destination;
    size_t length;
    size_t index;
} WowLoginWriter;

typedef enum
{
    WLM_RESULT_NOT_ENOUGH_BYTES = 1,
    WLM_RESULT_SUCCESS = 0,
    WLM_RESULT_INVALID_PARAMETERS = -1,
    WLM_RESULT_MALLOC_FAIL = -2
} WowLoginResult;

typedef enum
{
    CMD_AUTH_LOGON_CHALLENGE = 0x00,
    CMD_AUTH_LOGON_PROOF = 0x01,
    CMD_AUTH_RECONNECT_CHALLENGE = 0x02,
    CMD_AUTH_RECONNECT_PROOF = 0x03,
    CMD_SURVEY_RESULT = 0x04,
    CMD_REALM_LIST = 0x10,
    CMD_XFER_INITIATE = 0x30,
    CMD_XFER_DATA = 0x31,
    CMD_XFER_ACCEPT = 0x32,
    CMD_XFER_RESUME = 0x33,
    CMD_XFER_CANCEL = 0x34
} WowLoginOpcode;

WOW_LOGIN_MESSAGES_C_EXPORT const char* wlm_error_code_to_string(WowLoginResult result);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginReader wlm_create_reader(const unsigned char* const source, const size_t length);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginWriter wlm_create_writer(unsigned char* const destination, const size_t length);

#endif /* WOW_LOGIN_MESSAGES_H */
