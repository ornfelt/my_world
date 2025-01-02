#ifndef WOW_LOGIN_MESSAGES_VERSION6_H
#define WOW_LOGIN_MESSAGES_VERSION6_H

/* clang-format off */

#include "wow_login_messages/wow_login_messages.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "wow_login_messages/all.h" /* type include */
#include "wow_login_messages/version2.h" /* type include */
#include "wow_login_messages/version5.h" /* type include */

typedef all_Locale version6_Locale;

typedef version2_LoginResult version6_LoginResult;

typedef all_Os version6_Os;

typedef all_Platform version6_Platform;

typedef all_ProtocolVersion version6_ProtocolVersion;

typedef version2_RealmCategory version6_RealmCategory;

typedef version2_RealmType version6_RealmType;

typedef version2_RealmFlag version6_RealmFlag;

typedef version5_SecurityFlag version6_SecurityFlag;

typedef version5_Realm version6_Realm;

typedef all_Version version6_Version;

typedef version2_TelemetryKey version6_TelemetryKey;

typedef version5_CMD_AUTH_LOGON_CHALLENGE_Server version6_CMD_AUTH_LOGON_CHALLENGE_Server;

typedef all_CMD_AUTH_LOGON_CHALLENGE_Client version6_CMD_AUTH_LOGON_CHALLENGE_Client;

typedef version5_CMD_AUTH_LOGON_PROOF_Client version6_CMD_AUTH_LOGON_PROOF_Client;

typedef version5_CMD_AUTH_LOGON_PROOF_Server version6_CMD_AUTH_LOGON_PROOF_Server;

typedef version2_CMD_AUTH_RECONNECT_CHALLENGE_Server version6_CMD_AUTH_RECONNECT_CHALLENGE_Server;

typedef all_CMD_AUTH_RECONNECT_CHALLENGE_Client version6_CMD_AUTH_RECONNECT_CHALLENGE_Client;

typedef version2_CMD_AUTH_RECONNECT_PROOF_Client version6_CMD_AUTH_RECONNECT_PROOF_Client;

typedef version5_CMD_AUTH_RECONNECT_PROOF_Server version6_CMD_AUTH_RECONNECT_PROOF_Server;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version6_CMD_REALM_LIST_Client_write(WowLoginWriter* writer);

typedef struct {
    uint16_t number_of_realms;
    version5_Realm* realms;

} version6_CMD_REALM_LIST_Server;
WOW_LOGIN_MESSAGES_C_EXPORT void version6_CMD_REALM_LIST_Server_free(version6_CMD_REALM_LIST_Server* object);
WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version6_CMD_REALM_LIST_Server_write(WowLoginWriter* writer, const version6_CMD_REALM_LIST_Server* object);

typedef version2_CMD_XFER_INITIATE version6_CMD_XFER_INITIATE;

typedef version2_CMD_XFER_DATA version6_CMD_XFER_DATA;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version6_CMD_XFER_ACCEPT_write(WowLoginWriter* writer);

typedef version2_CMD_XFER_RESUME version6_CMD_XFER_RESUME;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version6_CMD_XFER_CANCEL_write(WowLoginWriter* writer);

typedef struct {
    WowLoginOpcode opcode;

    union {
        version5_CMD_AUTH_LOGON_PROOF_Client CMD_AUTH_LOGON_PROOF_Client;
        version2_CMD_AUTH_RECONNECT_PROOF_Client CMD_AUTH_RECONNECT_PROOF_Client;
        version2_CMD_XFER_RESUME CMD_XFER_RESUME;
    } body;
} Version6ClientOpcodeContainer;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version6_client_opcode_write(WowLoginWriter* writer, const Version6ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version6_client_opcode_read(WowLoginReader* reader, Version6ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT void version6_client_opcode_free(Version6ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT char* version6_client_opcode_to_str(Version6ClientOpcodeContainer* opcodes);
typedef struct {
    WowLoginOpcode opcode;

    union {
        version5_CMD_AUTH_LOGON_CHALLENGE_Server CMD_AUTH_LOGON_CHALLENGE_Server;
        version5_CMD_AUTH_LOGON_PROOF_Server CMD_AUTH_LOGON_PROOF_Server;
        version2_CMD_AUTH_RECONNECT_CHALLENGE_Server CMD_AUTH_RECONNECT_CHALLENGE_Server;
        version5_CMD_AUTH_RECONNECT_PROOF_Server CMD_AUTH_RECONNECT_PROOF_Server;
        version6_CMD_REALM_LIST_Server CMD_REALM_LIST_Server;
        version2_CMD_XFER_INITIATE CMD_XFER_INITIATE;
        version2_CMD_XFER_DATA CMD_XFER_DATA;
    } body;
} Version6ServerOpcodeContainer;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version6_server_opcode_write(WowLoginWriter* writer, const Version6ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version6_server_opcode_read(WowLoginReader* reader, Version6ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT void version6_server_opcode_free(Version6ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT char* version6_server_opcode_to_str(Version6ServerOpcodeContainer* opcodes);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* WOW_LOGIN_MESSAGES_VERSION6_H */
