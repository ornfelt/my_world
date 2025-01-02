#ifndef WOW_LOGIN_MESSAGES_VERSION7_H
#define WOW_LOGIN_MESSAGES_VERSION7_H

/* clang-format off */

#include "wow_login_messages/wow_login_messages.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "wow_login_messages/all.h" /* type include */
#include "wow_login_messages/version2.h" /* type include */
#include "wow_login_messages/version5.h" /* type include */
#include "wow_login_messages/version6.h" /* type include */

typedef all_Locale version7_Locale;

typedef version2_LoginResult version7_LoginResult;

typedef all_Os version7_Os;

typedef all_Platform version7_Platform;

typedef all_ProtocolVersion version7_ProtocolVersion;

typedef version2_RealmCategory version7_RealmCategory;

typedef version2_RealmType version7_RealmType;

typedef version2_RealmFlag version7_RealmFlag;

typedef version5_SecurityFlag version7_SecurityFlag;

typedef version5_Realm version7_Realm;

typedef all_Version version7_Version;

typedef version2_TelemetryKey version7_TelemetryKey;

typedef version5_CMD_AUTH_LOGON_CHALLENGE_Server version7_CMD_AUTH_LOGON_CHALLENGE_Server;

typedef all_CMD_AUTH_LOGON_CHALLENGE_Client version7_CMD_AUTH_LOGON_CHALLENGE_Client;

typedef version5_CMD_AUTH_LOGON_PROOF_Client version7_CMD_AUTH_LOGON_PROOF_Client;

typedef version5_CMD_AUTH_LOGON_PROOF_Server version7_CMD_AUTH_LOGON_PROOF_Server;

typedef version2_CMD_AUTH_RECONNECT_CHALLENGE_Server version7_CMD_AUTH_RECONNECT_CHALLENGE_Server;

typedef all_CMD_AUTH_RECONNECT_CHALLENGE_Client version7_CMD_AUTH_RECONNECT_CHALLENGE_Client;

typedef version2_CMD_AUTH_RECONNECT_PROOF_Client version7_CMD_AUTH_RECONNECT_PROOF_Client;

typedef version5_CMD_AUTH_RECONNECT_PROOF_Server version7_CMD_AUTH_RECONNECT_PROOF_Server;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version7_CMD_REALM_LIST_Client_write(WowLoginWriter* writer);

typedef version6_CMD_REALM_LIST_Server version7_CMD_REALM_LIST_Server;

typedef version2_CMD_XFER_INITIATE version7_CMD_XFER_INITIATE;

typedef version2_CMD_XFER_DATA version7_CMD_XFER_DATA;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version7_CMD_XFER_ACCEPT_write(WowLoginWriter* writer);

typedef version2_CMD_XFER_RESUME version7_CMD_XFER_RESUME;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version7_CMD_XFER_CANCEL_write(WowLoginWriter* writer);

typedef struct {
    WowLoginOpcode opcode;

    union {
        version5_CMD_AUTH_LOGON_PROOF_Client CMD_AUTH_LOGON_PROOF_Client;
        version2_CMD_AUTH_RECONNECT_PROOF_Client CMD_AUTH_RECONNECT_PROOF_Client;
        version2_CMD_XFER_RESUME CMD_XFER_RESUME;
    } body;
} Version7ClientOpcodeContainer;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version7_client_opcode_write(WowLoginWriter* writer, const Version7ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version7_client_opcode_read(WowLoginReader* reader, Version7ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT void version7_client_opcode_free(Version7ClientOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT char* version7_client_opcode_to_str(Version7ClientOpcodeContainer* opcodes);
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
} Version7ServerOpcodeContainer;

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version7_server_opcode_write(WowLoginWriter* writer, const Version7ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT WowLoginResult version7_server_opcode_read(WowLoginReader* reader, Version7ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT void version7_server_opcode_free(Version7ServerOpcodeContainer* opcodes);

WOW_LOGIN_MESSAGES_C_EXPORT char* version7_server_opcode_to_str(Version7ServerOpcodeContainer* opcodes);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* WOW_LOGIN_MESSAGES_VERSION7_H */
