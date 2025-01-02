#ifndef WOW_LOGIN_MESSAGES_ALL_H
#define WOW_LOGIN_MESSAGES_ALL_H

/* clang-format off */

#include "wow_login_messages/wow_login_messages.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    ALL_LOCALE_EN_GB = 1701726018,
    ALL_LOCALE_EN_US = 1701729619,
    ALL_LOCALE_ES_MX = 1702055256,
    ALL_LOCALE_PT_BR = 1886667346,
    ALL_LOCALE_FR_FR = 1718765138,
    ALL_LOCALE_DE_DE = 1684358213,
    ALL_LOCALE_ES_ES = 1702053203,
    ALL_LOCALE_PT_PT = 1886670932,
    ALL_LOCALE_IT_IT = 1769228628,
    ALL_LOCALE_RU_RU = 1920291413,
    ALL_LOCALE_KO_KR = 1802455890,
    ALL_LOCALE_ZH_TW = 2053657687,
    ALL_LOCALE_EN_TW = 1701729367,
    ALL_LOCALE_EN_CN = 1701725006
} all_Locale;

typedef enum {
    ALL_OS_WINDOWS = 5728622,
    ALL_OS_MAC_OS_X = 5198680
} all_Os;

typedef enum {
    ALL_PLATFORM_X86 = 7878710,
    ALL_PLATFORM_POWER_PC = 5263427
} all_Platform;

typedef enum {
    ALL_PROTOCOL_VERSION_TWO = 2,
    ALL_PROTOCOL_VERSION_THREE = 3,
    ALL_PROTOCOL_VERSION_FIVE = 5,
    ALL_PROTOCOL_VERSION_SIX = 6,
    ALL_PROTOCOL_VERSION_SEVEN = 7,
    ALL_PROTOCOL_VERSION_EIGHT = 8
} all_ProtocolVersion;

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint16_t build;

} all_Version;

/* First message sent by the client when attempting to connect. The server will respond with [CMD_AUTH_LOGON_CHALLENGE_Server].
Has the exact same layout as [CMD_AUTH_RECONNECT_CHALLENGE_Client]. */
typedef struct {
    /* Determines which version of messages are used for further communication. */
    all_ProtocolVersion protocol_version;
    all_Version version;
    all_Platform platform;
    all_Os os;
    all_Locale locale;
    /* Offset in minutes from UTC time. 180 would be UTC+3 */
    int32_t utc_timezone_offset;
    uint32_t client_ip_address;
    /* Real clients can send a maximum of 16 UTF-8 characters. This is not necessarily 16 bytes since one character can be more than one byte.
Real clients will send a fully uppercased username, and will perform authentication calculations on the uppercased version.
Uppercasing in regards to non-ASCII values is little weird. See `https://docs.rs/wow_srp/latest/wow_srp/normalized_string/index.html` for more info. */
    char* account_name;

} all_CMD_AUTH_LOGON_CHALLENGE_Client;
WOW_LOGIN_MESSAGES_C_EXPORT void all_CMD_AUTH_LOGON_CHALLENGE_Client_free(all_CMD_AUTH_LOGON_CHALLENGE_Client* object);
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult all_CMD_AUTH_LOGON_CHALLENGE_Client_write(WowLoginWriter* writer, const all_CMD_AUTH_LOGON_CHALLENGE_Client* object);

/* First message sent by the client when attempting to reconnect. The server will respond with [CMD_AUTH_RECONNECT_CHALLENGE_Server].
Has the exact same layout as [CMD_AUTH_LOGON_CHALLENGE_Client]. */
typedef struct {
    /* Determines which version of messages are used for further communication. */
    all_ProtocolVersion protocol_version;
    all_Version version;
    all_Platform platform;
    all_Os os;
    all_Locale locale;
    /* Offset in minutes from UTC time. 180 would be UTC+3 */
    int32_t utc_timezone_offset;
    uint32_t client_ip_address;
    /* Real clients can send a maximum of 16 UTF-8 characters. This is not necessarily 16 bytes since one character can be more than one byte.
Real clients will send a fully uppercased username, and will perform authentication calculations on the uppercased version.
Uppercasing in regards to non-ASCII values is little weird. See `https://docs.rs/wow_srp/latest/wow_srp/normalized_string/index.html` for more info. */
    char* account_name;

} all_CMD_AUTH_RECONNECT_CHALLENGE_Client;
WOW_LOGIN_MESSAGES_C_EXPORT void all_CMD_AUTH_RECONNECT_CHALLENGE_Client_free(all_CMD_AUTH_RECONNECT_CHALLENGE_Client* object);
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult all_CMD_AUTH_RECONNECT_CHALLENGE_Client_write(WowLoginWriter* writer, const all_CMD_AUTH_RECONNECT_CHALLENGE_Client* object);

typedef struct {
    WowLoginOpcode opcode;

    union {
        all_CMD_AUTH_LOGON_CHALLENGE_Client CMD_AUTH_LOGON_CHALLENGE_Client;
        all_CMD_AUTH_RECONNECT_CHALLENGE_Client CMD_AUTH_RECONNECT_CHALLENGE_Client;
    } body;
} AllClientOpcodeContainer;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult all_client_opcode_write(WowLoginWriter* writer, const AllClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult all_client_opcode_read(WowLoginReader* reader, AllClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT void all_client_opcode_free(AllClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT char* all_client_opcode_to_str(AllClientOpcodeContainer* opcodes);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* WOW_LOGIN_MESSAGES_ALL_H */
