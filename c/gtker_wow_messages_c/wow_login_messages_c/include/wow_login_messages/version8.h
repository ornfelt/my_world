#ifndef WOW_LOGIN_MESSAGES_VERSION8_H
#define WOW_LOGIN_MESSAGES_VERSION8_H

/* clang-format off */

#include "wow_login_messages/wow_login_messages.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "wow_login_messages/all.h" /* type include */
#include "wow_login_messages/version2.h" /* type include */

typedef all_Locale version8_Locale;

typedef enum {
    VERSION8_LOGIN_RESULT_SUCCESS = 0,
    VERSION8_LOGIN_RESULT_FAIL_UNKNOWN0 = 1,
    VERSION8_LOGIN_RESULT_FAIL_UNKNOWN1 = 2,
    VERSION8_LOGIN_RESULT_FAIL_BANNED = 3,
    VERSION8_LOGIN_RESULT_FAIL_UNKNOWN_ACCOUNT = 4,
    VERSION8_LOGIN_RESULT_FAIL_INCORRECT_PASSWORD = 5,
    VERSION8_LOGIN_RESULT_FAIL_ALREADY_ONLINE = 6,
    VERSION8_LOGIN_RESULT_FAIL_NO_TIME = 7,
    VERSION8_LOGIN_RESULT_FAIL_DB_BUSY = 8,
    VERSION8_LOGIN_RESULT_FAIL_VERSION_INVALID = 9,
    VERSION8_LOGIN_RESULT_LOGIN_DOWNLOAD_FILE = 10,
    VERSION8_LOGIN_RESULT_FAIL_INVALID_SERVER = 11,
    VERSION8_LOGIN_RESULT_FAIL_SUSPENDED = 12,
    VERSION8_LOGIN_RESULT_FAIL_NO_ACCESS = 13,
    VERSION8_LOGIN_RESULT_SUCCESS_SURVEY = 14,
    VERSION8_LOGIN_RESULT_FAIL_PARENTALCONTROL = 15,
    VERSION8_LOGIN_RESULT_FAIL_LOCKED_ENFORCED = 16
} version8_LoginResult;

typedef all_Os version8_Os;

typedef all_Platform version8_Platform;

typedef all_ProtocolVersion version8_ProtocolVersion;

typedef version2_RealmCategory version8_RealmCategory;

typedef version2_RealmType version8_RealmType;

typedef enum {
    VERSION8_ACCOUNT_FLAG_GM = 1,
    VERSION8_ACCOUNT_FLAG_TRIAL = 8,
    VERSION8_ACCOUNT_FLAG_PROPASS = 8388608
} version8_AccountFlag;

typedef enum {
    VERSION8_REALM_FLAG_NONE = 0,
    VERSION8_REALM_FLAG_INVALID = 1,
    VERSION8_REALM_FLAG_OFFLINE = 2,
    VERSION8_REALM_FLAG_SPECIFY_BUILD = 4,
    VERSION8_REALM_FLAG_FORCE_BLUE_RECOMMENDED = 32,
    VERSION8_REALM_FLAG_FORCE_GREEN_RECOMMENDED = 64,
    VERSION8_REALM_FLAG_FORCE_RED_FULL = 128
} version8_RealmFlag;

typedef enum {
    VERSION8_SECURITY_FLAG_NONE = 0,
    VERSION8_SECURITY_FLAG_PIN = 1,
    VERSION8_SECURITY_FLAG_MATRIX_CARD = 2,
    VERSION8_SECURITY_FLAG_AUTHENTICATOR = 4
} version8_SecurityFlag;

typedef all_Version version8_Version;

typedef struct {
    /* vmangos: this is the second column in `Cfg_Configs.dbc` */
    version8_RealmType realm_type;
    bool locked;
    version8_RealmFlag flag;
    char* name;
    char* address;
    float population;
    uint8_t number_of_characters_on_realm;
    version8_RealmCategory category;
    uint8_t realm_id;
    all_Version version;

} version8_Realm;

typedef version2_TelemetryKey version8_TelemetryKey;

typedef struct {
    version8_LoginResult result;
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
    version8_SecurityFlag security_flag;
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
    /* Dictates if the authenticator is in use and not just assigned to the account. */
    uint8_t required;

} version8_CMD_AUTH_LOGON_CHALLENGE_Server;
WOW_LOGIN_MESSAGES_C_EXPORT void version8_CMD_AUTH_LOGON_CHALLENGE_Server_free(version8_CMD_AUTH_LOGON_CHALLENGE_Server* object);
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_CMD_AUTH_LOGON_CHALLENGE_Server_write(WowLoginWriter* writer, const version8_CMD_AUTH_LOGON_CHALLENGE_Server* object);

typedef all_CMD_AUTH_LOGON_CHALLENGE_Client version8_CMD_AUTH_LOGON_CHALLENGE_Client;

/* Reply after successful [CMD_AUTH_LOGON_CHALLENGE_Server]. */
typedef struct {
    uint8_t client_public_key[32];
    uint8_t client_proof[20];
    uint8_t crc_hash[20];
    uint8_t number_of_telemetry_keys;
    version2_TelemetryKey* telemetry_keys;
    version8_SecurityFlag security_flag;
    uint8_t pin_salt[16];
    uint8_t pin_hash[20];
    /* Client proof of matrix input.
Implementation details at `https://gist.github.com/barncastle/979c12a9c5e64d810a28ad1728e7e0f9`. */
    uint8_t matrix_card_proof[20];
    /* String entered by the user in the "Authenticator" popup.
Can be empty and up to 16 characters.
Is not used by the client in any way but just sent directly, so this could in theory be used for anything. */
    char* authenticator;

} version8_CMD_AUTH_LOGON_PROOF_Client;
WOW_LOGIN_MESSAGES_C_EXPORT void version8_CMD_AUTH_LOGON_PROOF_Client_free(version8_CMD_AUTH_LOGON_PROOF_Client* object);
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_CMD_AUTH_LOGON_PROOF_Client_write(WowLoginWriter* writer, const version8_CMD_AUTH_LOGON_PROOF_Client* object);

typedef struct {
    version8_LoginResult result;
    uint8_t server_proof[20];
    version8_AccountFlag account_flag;
    uint32_t hardware_survey_id;
    uint16_t unknown;

} version8_CMD_AUTH_LOGON_PROOF_Server;
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_CMD_AUTH_LOGON_PROOF_Server_write(WowLoginWriter* writer, const version8_CMD_AUTH_LOGON_PROOF_Server* object);

/* Reply to [CMD_AUTH_RECONNECT_CHALLENGE_Client]. */
typedef struct {
    version8_LoginResult result;
    uint8_t challenge_data[16];
    uint8_t checksum_salt[16];

} version8_CMD_AUTH_RECONNECT_CHALLENGE_Server;
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_CMD_AUTH_RECONNECT_CHALLENGE_Server_write(WowLoginWriter* writer, const version8_CMD_AUTH_RECONNECT_CHALLENGE_Server* object);

typedef all_CMD_AUTH_RECONNECT_CHALLENGE_Client version8_CMD_AUTH_RECONNECT_CHALLENGE_Client;

typedef version2_CMD_AUTH_RECONNECT_PROOF_Client version8_CMD_AUTH_RECONNECT_PROOF_Client;

/* Reply to [CMD_AUTH_RECONNECT_PROOF_Client]. */
typedef struct {
    version8_LoginResult result;

} version8_CMD_AUTH_RECONNECT_PROOF_Server;
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_CMD_AUTH_RECONNECT_PROOF_Server_write(WowLoginWriter* writer, const version8_CMD_AUTH_RECONNECT_PROOF_Server* object);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_CMD_REALM_LIST_Client_write(WowLoginWriter* writer);

typedef struct {
    uint16_t number_of_realms;
    version8_Realm* realms;

} version8_CMD_REALM_LIST_Server;
WOW_LOGIN_MESSAGES_C_EXPORT void version8_CMD_REALM_LIST_Server_free(version8_CMD_REALM_LIST_Server* object);
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_CMD_REALM_LIST_Server_write(WowLoginWriter* writer, const version8_CMD_REALM_LIST_Server* object);

typedef version2_CMD_XFER_INITIATE version8_CMD_XFER_INITIATE;

typedef version2_CMD_XFER_DATA version8_CMD_XFER_DATA;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_CMD_XFER_ACCEPT_write(WowLoginWriter* writer);

typedef version2_CMD_XFER_RESUME version8_CMD_XFER_RESUME;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_CMD_XFER_CANCEL_write(WowLoginWriter* writer);

typedef struct {
    WowLoginOpcode opcode;

    union {
        version8_CMD_AUTH_LOGON_PROOF_Client CMD_AUTH_LOGON_PROOF_Client;
        version2_CMD_AUTH_RECONNECT_PROOF_Client CMD_AUTH_RECONNECT_PROOF_Client;
        version2_CMD_XFER_RESUME CMD_XFER_RESUME;
    } body;
} Version8ClientOpcodeContainer;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_client_opcode_write(WowLoginWriter* writer, const Version8ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_client_opcode_read(WowLoginReader* reader, Version8ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT void version8_client_opcode_free(Version8ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT char* version8_client_opcode_to_str(Version8ClientOpcodeContainer* opcodes);
typedef struct {
    WowLoginOpcode opcode;

    union {
        version8_CMD_AUTH_LOGON_CHALLENGE_Server CMD_AUTH_LOGON_CHALLENGE_Server;
        version8_CMD_AUTH_LOGON_PROOF_Server CMD_AUTH_LOGON_PROOF_Server;
        version8_CMD_AUTH_RECONNECT_CHALLENGE_Server CMD_AUTH_RECONNECT_CHALLENGE_Server;
        version8_CMD_AUTH_RECONNECT_PROOF_Server CMD_AUTH_RECONNECT_PROOF_Server;
        version8_CMD_REALM_LIST_Server CMD_REALM_LIST_Server;
        version2_CMD_XFER_INITIATE CMD_XFER_INITIATE;
        version2_CMD_XFER_DATA CMD_XFER_DATA;
    } body;
} Version8ServerOpcodeContainer;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_server_opcode_write(WowLoginWriter* writer, const Version8ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version8_server_opcode_read(WowLoginReader* reader, Version8ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT void version8_server_opcode_free(Version8ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT char* version8_server_opcode_to_str(Version8ServerOpcodeContainer* opcodes);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* WOW_LOGIN_MESSAGES_VERSION8_H */
