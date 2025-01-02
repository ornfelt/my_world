#ifndef WOW_LOGIN_MESSAGES_CPP_VERSION7_HPP
#define WOW_LOGIN_MESSAGES_CPP_VERSION7_HPP

/* clang-format off */

#include "wow_login_messages_cpp/wow_login_messages.hpp"

#include "wow_login_messages_cpp/all.hpp" /* type include */
#include "wow_login_messages_cpp/version2.hpp" /* type include */
#include "wow_login_messages_cpp/version5.hpp" /* type include */
#include "wow_login_messages_cpp/version6.hpp" /* type include */
namespace wow_login_messages {
namespace version7 {

typedef all::Locale Locale;

typedef version2::LoginResult LoginResult;

typedef all::Os Os;

typedef all::Platform Platform;

typedef all::ProtocolVersion ProtocolVersion;

typedef version2::RealmCategory RealmCategory;

typedef version2::RealmType RealmType;

typedef version2::RealmFlag RealmFlag;

typedef version5::SecurityFlag SecurityFlag;

typedef version5::Realm Realm;

typedef all::Version Version;

typedef version2::TelemetryKey TelemetryKey;

typedef version5::CMD_AUTH_LOGON_CHALLENGE_Server CMD_AUTH_LOGON_CHALLENGE_Server;

typedef all::CMD_AUTH_LOGON_CHALLENGE_Client CMD_AUTH_LOGON_CHALLENGE_Client;

typedef version5::CMD_AUTH_LOGON_PROOF_Client CMD_AUTH_LOGON_PROOF_Client;

typedef version5::CMD_AUTH_LOGON_PROOF_Server CMD_AUTH_LOGON_PROOF_Server;

typedef version2::CMD_AUTH_RECONNECT_CHALLENGE_Server CMD_AUTH_RECONNECT_CHALLENGE_Server;

typedef all::CMD_AUTH_RECONNECT_CHALLENGE_Client CMD_AUTH_RECONNECT_CHALLENGE_Client;

typedef version2::CMD_AUTH_RECONNECT_PROOF_Client CMD_AUTH_RECONNECT_PROOF_Client;

typedef version5::CMD_AUTH_RECONNECT_PROOF_Server CMD_AUTH_RECONNECT_PROOF_Server;

struct CMD_REALM_LIST_Client {

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;
};

typedef version6::CMD_REALM_LIST_Server CMD_REALM_LIST_Server;

typedef version2::CMD_XFER_INITIATE CMD_XFER_INITIATE;

typedef version2::CMD_XFER_DATA CMD_XFER_DATA;

struct CMD_XFER_ACCEPT {

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;
};

typedef version2::CMD_XFER_RESUME CMD_XFER_RESUME;

struct CMD_XFER_CANCEL {

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;
};

class ClientOpcode {
    enum class Opcode {
        NONE = 0xFF,
        CMD_AUTH_LOGON_PROOF = 1,
        CMD_AUTH_RECONNECT_PROOF = 3,
        CMD_REALM_LIST = 16,
        CMD_XFER_ACCEPT = 50,
        CMD_XFER_RESUME = 51,
        CMD_XFER_CANCEL = 52,
    } opcode;

    union {
        version7::CMD_AUTH_LOGON_PROOF_Client CMD_AUTH_LOGON_PROOF;
        version7::CMD_AUTH_RECONNECT_PROOF_Client CMD_AUTH_RECONNECT_PROOF;
        version7::CMD_REALM_LIST_Client CMD_REALM_LIST;
        version7::CMD_XFER_ACCEPT CMD_XFER_ACCEPT;
        version7::CMD_XFER_RESUME CMD_XFER_RESUME;
        version7::CMD_XFER_CANCEL CMD_XFER_CANCEL;
    };

public:
    WOW_LOGIN_MESSAGES_CPP_EXPORT bool is_none() const noexcept {
        return opcode == Opcode::NONE;
    }
    WOW_LOGIN_MESSAGES_CPP_EXPORT static ClientOpcode read(Reader& reader);

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;


    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode() : opcode(Opcode::NONE), CMD_AUTH_LOGON_PROOF() {}

    /* 1 destructor */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ~ClientOpcode();

    /* 2 copy constructor */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(const ClientOpcode& other);
    /* 3 copy assignment */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode& operator=(const ClientOpcode& other);
    /* 4 move constructor */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(ClientOpcode&& other) noexcept;

    /* 5 move assignment */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode& operator=(ClientOpcode&& other) noexcept;

    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version7::CMD_AUTH_LOGON_PROOF_Client&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version7::CMD_AUTH_RECONNECT_PROOF_Client&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version7::CMD_REALM_LIST_Client&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version7::CMD_XFER_ACCEPT&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version7::CMD_XFER_RESUME&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version7::CMD_XFER_CANCEL&& obj);

    template<typename T>
    // NOLINTNEXTLINE
    T& get() {
        static_assert(std::is_same<T, version7::CMD_AUTH_LOGON_PROOF_Client>::value, "Invalid type. All possible types have been specialized below.");
    }

    template<typename T>
    // NOLINTNEXTLINE
    T* get_if() {
        static_assert(std::is_same<T, version7::CMD_AUTH_LOGON_PROOF_Client>::value, "Invalid type. All possible types have been specialized below.");
    }

    WOW_LOGIN_MESSAGES_CPP_EXPORT const char* to_string() const;
};

template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_LOGON_PROOF_Client* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_LOGON_PROOF_Client& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_RECONNECT_PROOF_Client* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_RECONNECT_PROOF_Client& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_REALM_LIST_Client* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_REALM_LIST_Client& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_ACCEPT* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_ACCEPT& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_RESUME* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_RESUME& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_CANCEL* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_CANCEL& ClientOpcode::get();

class ServerOpcode {
    enum class Opcode {
        NONE = 0xFF,
        CMD_AUTH_LOGON_CHALLENGE = 0,
        CMD_AUTH_LOGON_PROOF = 1,
        CMD_AUTH_RECONNECT_CHALLENGE = 2,
        CMD_AUTH_RECONNECT_PROOF = 3,
        CMD_REALM_LIST = 16,
        CMD_XFER_INITIATE = 48,
        CMD_XFER_DATA = 49,
    } opcode;

    union {
        version7::CMD_AUTH_LOGON_CHALLENGE_Server CMD_AUTH_LOGON_CHALLENGE;
        version7::CMD_AUTH_LOGON_PROOF_Server CMD_AUTH_LOGON_PROOF;
        version7::CMD_AUTH_RECONNECT_CHALLENGE_Server CMD_AUTH_RECONNECT_CHALLENGE;
        version7::CMD_AUTH_RECONNECT_PROOF_Server CMD_AUTH_RECONNECT_PROOF;
        version7::CMD_REALM_LIST_Server CMD_REALM_LIST;
        version7::CMD_XFER_INITIATE CMD_XFER_INITIATE;
        version7::CMD_XFER_DATA CMD_XFER_DATA;
    };

public:
    WOW_LOGIN_MESSAGES_CPP_EXPORT bool is_none() const noexcept {
        return opcode == Opcode::NONE;
    }
    WOW_LOGIN_MESSAGES_CPP_EXPORT static ServerOpcode read(Reader& reader);

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;


    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode() : opcode(Opcode::NONE), CMD_AUTH_LOGON_CHALLENGE() {}

    /* 1 destructor */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ~ServerOpcode();

    /* 2 copy constructor */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(const ServerOpcode& other);
    /* 3 copy assignment */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode& operator=(const ServerOpcode& other);
    /* 4 move constructor */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(ServerOpcode&& other) noexcept;

    /* 5 move assignment */
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode& operator=(ServerOpcode&& other) noexcept;

    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version7::CMD_AUTH_LOGON_CHALLENGE_Server&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version7::CMD_AUTH_LOGON_PROOF_Server&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version7::CMD_AUTH_RECONNECT_CHALLENGE_Server&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version7::CMD_AUTH_RECONNECT_PROOF_Server&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version7::CMD_REALM_LIST_Server&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version7::CMD_XFER_INITIATE&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version7::CMD_XFER_DATA&& obj);

    template<typename T>
    // NOLINTNEXTLINE
    T& get() {
        static_assert(std::is_same<T, version7::CMD_AUTH_LOGON_CHALLENGE_Server>::value, "Invalid type. All possible types have been specialized below.");
    }

    template<typename T>
    // NOLINTNEXTLINE
    T* get_if() {
        static_assert(std::is_same<T, version7::CMD_AUTH_LOGON_CHALLENGE_Server>::value, "Invalid type. All possible types have been specialized below.");
    }

    WOW_LOGIN_MESSAGES_CPP_EXPORT const char* to_string() const;
};

template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_LOGON_CHALLENGE_Server* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_LOGON_CHALLENGE_Server& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_LOGON_PROOF_Server* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_LOGON_PROOF_Server& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_RECONNECT_CHALLENGE_Server* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_RECONNECT_CHALLENGE_Server& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_RECONNECT_PROOF_Server* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_AUTH_RECONNECT_PROOF_Server& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_REALM_LIST_Server* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_REALM_LIST_Server& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_INITIATE* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_INITIATE& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_DATA* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version7::CMD_XFER_DATA& ServerOpcode::get();

} // namespace version7
} // namespace wow_login_messages
#endif /* WOW_LOGIN_MESSAGES_CPP_VERSION7_HPP */
