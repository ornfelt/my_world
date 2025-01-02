#ifndef WOW_LOGIN_MESSAGES_VERSION5_H
#define WOW_LOGIN_MESSAGES_VERSION5_H

/* clang-format off */

#include "wow_login_messages/wow_login_messages.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "wow_login_messages/all.h" /* type include */
#include "wow_login_messages/version2.h" /* type include */

typedef all_Locale version5_Locale;

typedef version2_LoginResult version5_LoginResult;

typedef all_Os version5_Os;

typedef all_Platform version5_Platform;

typedef all_ProtocolVersion version5_ProtocolVersion;

typedef version2_RealmCategory version5_RealmCategory;

typedef version2_RealmType version5_RealmType;

typedef version2_RealmFlag version5_RealmFlag;

typedef enum {
    VERSION5_SECURITY_FLAG_NONE = 0,
    VERSION5_SECURITY_FLAG_PIN = 1,
    VERSION5_SECURITY_FLAG_MATRIX_CARD = 2
} version5_SecurityFlag;

typedef struct {
    version5_RealmType realm_type;
    bool locked;
    version5_RealmFlag flag;
    char* name;
    char* address;
    float population;
    uint8_t number_of_characters_on_realm;
    version5_RealmCategory category;
    uint8_t realm_id;

} version5_Realm;

typedef all_Version version5_Version;

typedef version2_TelemetryKey version5_TelemetryKey;

typedef struct {
    version5_LoginResult result;
    uint8_t server_public_key[32];
    /* The only realistic values for the generator are well below 255, so there's no reason for this to anything other than 1. */
    uint8_t generator_length;
    uint8_t* generator;
    /* Client can not handle arrays greater than 32. */
    uint8_t large_safe_prime_length;
    uint8_t* large_safe_prime;
    uint8_t salt[32];
    /* Used for the `crc_hash` in [CMD_AUTH_LOGON_PROOF_Client]. */
    uint8_t crc_salt[16];
    version5_SecurityFlag security_flag;
    /* Used to randomize the layout of the PIN keypad. */
    uint32_t pin_grid_seed;
    uint8_t pin_salt[16];
    /* Number of columns to display. */
    uint8_t width;
    /* Number of rows to display. */
    uint8_t height;
    /* Number of digits to be entered for each cell. */
    uint8_t digit_count;
    /* Number of cells to complete. */
    uint8_t challenge_count;
    /* Seed value used to randomize cell selection. */
    uint64_t seed;

} version5_CMD_AUTH_LOGON_CHALLENGE_Server;
WOW_LOGIN_MESSAGES_C_EXPORT void version5_CMD_AUTH_LOGON_CHALLENGE_Server_free(version5_CMD_AUTH_LOGON_CHALLENGE_Server* object);
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_CMD_AUTH_LOGON_CHALLENGE_Server_write(WowLoginWriter* writer, const version5_CMD_AUTH_LOGON_CHALLENGE_Server* object);

typedef all_CMD_AUTH_LOGON_CHALLENGE_Client version5_CMD_AUTH_LOGON_CHALLENGE_Client;

/* Reply after successful [CMD_AUTH_LOGON_CHALLENGE_Server]. */
typedef struct {
    uint8_t client_public_key[32];
    uint8_t client_proof[20];
    uint8_t crc_hash[20];
    uint8_t number_of_telemetry_keys;
    version2_TelemetryKey* telemetry_keys;
    version5_SecurityFlag security_flag;
    uint8_t pin_salt[16];
    uint8_t pin_hash[20];
    /* Client proof of matrix input.
Implementation details at `https://gist.github.com/barncastle/979c12a9c5e64d810a28ad1728e7e0f9`. */
    uint8_t matrix_card_proof[20];

} version5_CMD_AUTH_LOGON_PROOF_Client;
WOW_LOGIN_MESSAGES_C_EXPORT void version5_CMD_AUTH_LOGON_PROOF_Client_free(version5_CMD_AUTH_LOGON_PROOF_Client* object);
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_CMD_AUTH_LOGON_PROOF_Client_write(WowLoginWriter* writer, const version5_CMD_AUTH_LOGON_PROOF_Client* object);

/* Reply to [CMD_AUTH_LOGON_PROOF_Client]. */
typedef struct {
    version5_LoginResult result;
    uint8_t server_proof[20];
    uint32_t hardware_survey_id;
    uint16_t unknown;

} version5_CMD_AUTH_LOGON_PROOF_Server;
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_CMD_AUTH_LOGON_PROOF_Server_write(WowLoginWriter* writer, const version5_CMD_AUTH_LOGON_PROOF_Server* object);

typedef version2_CMD_AUTH_RECONNECT_CHALLENGE_Server version5_CMD_AUTH_RECONNECT_CHALLENGE_Server;

typedef all_CMD_AUTH_RECONNECT_CHALLENGE_Client version5_CMD_AUTH_RECONNECT_CHALLENGE_Client;

typedef version2_CMD_AUTH_RECONNECT_PROOF_Client version5_CMD_AUTH_RECONNECT_PROOF_Client;

/* Reply to [CMD_AUTH_RECONNECT_PROOF_Client]. */
typedef struct {
    version5_LoginResult result;

} version5_CMD_AUTH_RECONNECT_PROOF_Server;
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_CMD_AUTH_RECONNECT_PROOF_Server_write(WowLoginWriter* writer, const version5_CMD_AUTH_RECONNECT_PROOF_Server* object);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_CMD_REALM_LIST_Client_write(WowLoginWriter* writer);

typedef struct {
    uint8_t number_of_realms;
    version5_Realm* realms;

} version5_CMD_REALM_LIST_Server;
WOW_LOGIN_MESSAGES_C_EXPORT void version5_CMD_REALM_LIST_Server_free(version5_CMD_REALM_LIST_Server* object);
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_CMD_REALM_LIST_Server_write(WowLoginWriter* writer, const version5_CMD_REALM_LIST_Server* object);

typedef version2_CMD_XFER_INITIATE version5_CMD_XFER_INITIATE;

typedef version2_CMD_XFER_DATA version5_CMD_XFER_DATA;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_CMD_XFER_ACCEPT_write(WowLoginWriter* writer);

typedef version2_CMD_XFER_RESUME version5_CMD_XFER_RESUME;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_CMD_XFER_CANCEL_write(WowLoginWriter* writer);

typedef struct {
    WowLoginOpcode opcode;

    union {
        version5_CMD_AUTH_LOGON_PROOF_Client CMD_AUTH_LOGON_PROOF_Client;
        version2_CMD_AUTH_RECONNECT_PROOF_Client CMD_AUTH_RECONNECT_PROOF_Client;
        version2_CMD_XFER_RESUME CMD_XFER_RESUME;
    } body;
} Version5ClientOpcodeContainer;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_client_opcode_write(WowLoginWriter* writer, const Version5ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_client_opcode_read(WowLoginReader* reader, Version5ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT void version5_client_opcode_free(Version5ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT char* version5_client_opcode_to_str(Version5ClientOpcodeContainer* opcodes);
typedef struct {
    WowLoginOpcode opcode;

    union {
        version5_CMD_AUTH_LOGON_CHALLENGE_Server CMD_AUTH_LOGON_CHALLENGE_Server;
        version5_CMD_AUTH_LOGON_PROOF_Server CMD_AUTH_LOGON_PROOF_Server;
        version2_CMD_AUTH_RECONNECT_CHALLENGE_Server CMD_AUTH_RECONNECT_CHALLENGE_Server;
        version5_CMD_AUTH_RECONNECT_PROOF_Server CMD_AUTH_RECONNECT_PROOF_Server;
        version5_CMD_REALM_LIST_Server CMD_REALM_LIST_Server;
        version2_CMD_XFER_INITIATE CMD_XFER_INITIATE;
        version2_CMD_XFER_DATA CMD_XFER_DATA;
    } body;
} Version5ServerOpcodeContainer;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_server_opcode_write(WowLoginWriter* writer, const Version5ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version5_server_opcode_read(WowLoginReader* reader, Version5ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT void version5_server_opcode_free(Version5ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT char* version5_server_opcode_to_str(Version5ServerOpcodeContainer* opcodes);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* WOW_LOGIN_MESSAGES_VERSION5_H */
