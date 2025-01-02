#ifndef WOW_LOGIN_MESSAGES_CPP_VERSION3_HPP
#define WOW_LOGIN_MESSAGES_CPP_VERSION3_HPP

/* clang-format off */

#include "wow_login_messages_cpp/wow_login_messages.hpp"

#include "wow_login_messages_cpp/all.hpp" /* type include */
#include "wow_login_messages_cpp/version2.hpp" /* type include */
namespace wow_login_messages {
namespace version3 {

typedef all::Locale Locale;

typedef version2::LoginResult LoginResult;

typedef all::Os Os;

typedef all::Platform Platform;

typedef all::ProtocolVersion ProtocolVersion;

typedef version2::RealmCategory RealmCategory;

typedef version2::RealmType RealmType;

enum class SecurityFlag : uint8_t {
    NONE = 0,
    PIN = 1,
};

typedef version2::RealmFlag RealmFlag;

typedef version2::Realm Realm;

typedef all::Version Version;

typedef version2::TelemetryKey TelemetryKey;

/* Reply to [CMD_AUTH_LOGON_CHALLENGE_Client]. */
struct CMD_AUTH_LOGON_CHALLENGE_Server {
    LoginResult result;
    std::array<uint8_t, 32> server_public_key;
    std::vector<uint8_t> generator;
    std::vector<uint8_t> large_safe_prime;
    std::array<uint8_t, 32> salt;
    /* Used for the `crc_hash` in [CMD_AUTH_LOGON_PROOF_Client]. */
    std::array<uint8_t, 16> crc_salt;
    SecurityFlag security_flag;
    /* Used to randomize the layout of the PIN keypad. */
    uint32_t pin_grid_seed;
    std::array<uint8_t, 16> pin_salt;

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;
};

typedef all::CMD_AUTH_LOGON_CHALLENGE_Client CMD_AUTH_LOGON_CHALLENGE_Client;

typedef version2::CMD_AUTH_LOGON_PROOF_Server CMD_AUTH_LOGON_PROOF_Server;

/* Reply after successful [CMD_AUTH_LOGON_CHALLENGE_Server]. */
struct CMD_AUTH_LOGON_PROOF_Client {
    std::array<uint8_t, 32> client_public_key;
    std::array<uint8_t, 20> client_proof;
    std::array<uint8_t, 20> crc_hash;
    std::vector<version2::TelemetryKey> telemetry_keys;
    SecurityFlag security_flag;
    std::array<uint8_t, 16> pin_salt;
    std::array<uint8_t, 20> pin_hash;

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;
};

typedef all::CMD_AUTH_RECONNECT_CHALLENGE_Client CMD_AUTH_RECONNECT_CHALLENGE_Client;

struct CMD_SURVEY_RESULT {
    uint32_t survey_id;
    uint8_t error;
    std::vector<uint8_t> data;

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;
};

typedef version2::CMD_REALM_LIST_Server CMD_REALM_LIST_Server;

struct CMD_REALM_LIST_Client {

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;
};

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
        CMD_SURVEY_RESULT = 4,
        CMD_REALM_LIST = 16,
        CMD_XFER_ACCEPT = 50,
        CMD_XFER_RESUME = 51,
        CMD_XFER_CANCEL = 52,
    } opcode;

    union {
        version3::CMD_AUTH_LOGON_PROOF_Client CMD_AUTH_LOGON_PROOF;
        version3::CMD_SURVEY_RESULT CMD_SURVEY_RESULT;
        version3::CMD_REALM_LIST_Client CMD_REALM_LIST;
        version3::CMD_XFER_ACCEPT CMD_XFER_ACCEPT;
        version3::CMD_XFER_RESUME CMD_XFER_RESUME;
        version3::CMD_XFER_CANCEL CMD_XFER_CANCEL;
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

    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version3::CMD_AUTH_LOGON_PROOF_Client&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version3::CMD_SURVEY_RESULT&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version3::CMD_REALM_LIST_Client&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version3::CMD_XFER_ACCEPT&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version3::CMD_XFER_RESUME&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(version3::CMD_XFER_CANCEL&& obj);

    template<typename T>
    // NOLINTNEXTLINE
    T& get() {
        static_assert(std::is_same<T, version3::CMD_AUTH_LOGON_PROOF_Client>::value, "Invalid type. All possible types have been specialized below.");
    }

    template<typename T>
    // NOLINTNEXTLINE
    T* get_if() {
        static_assert(std::is_same<T, version3::CMD_AUTH_LOGON_PROOF_Client>::value, "Invalid type. All possible types have been specialized below.");
    }

    WOW_LOGIN_MESSAGES_CPP_EXPORT const char* to_string() const;
};

template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_AUTH_LOGON_PROOF_Client* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_AUTH_LOGON_PROOF_Client& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_SURVEY_RESULT* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_SURVEY_RESULT& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_REALM_LIST_Client* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_REALM_LIST_Client& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_ACCEPT* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_ACCEPT& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_RESUME* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_RESUME& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_CANCEL* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_CANCEL& ClientOpcode::get();

class ServerOpcode {
    enum class Opcode {
        NONE = 0xFF,
        CMD_AUTH_LOGON_CHALLENGE = 0,
        CMD_AUTH_LOGON_PROOF = 1,
        CMD_REALM_LIST = 16,
        CMD_XFER_INITIATE = 48,
        CMD_XFER_DATA = 49,
    } opcode;

    union {
        version3::CMD_AUTH_LOGON_CHALLENGE_Server CMD_AUTH_LOGON_CHALLENGE;
        version3::CMD_AUTH_LOGON_PROOF_Server CMD_AUTH_LOGON_PROOF;
        version3::CMD_REALM_LIST_Server CMD_REALM_LIST;
        version3::CMD_XFER_INITIATE CMD_XFER_INITIATE;
        version3::CMD_XFER_DATA CMD_XFER_DATA;
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

    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version3::CMD_AUTH_LOGON_CHALLENGE_Server&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version3::CMD_AUTH_LOGON_PROOF_Server&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version3::CMD_REALM_LIST_Server&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version3::CMD_XFER_INITIATE&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ServerOpcode(version3::CMD_XFER_DATA&& obj);

    template<typename T>
    // NOLINTNEXTLINE
    T& get() {
        static_assert(std::is_same<T, version3::CMD_AUTH_LOGON_CHALLENGE_Server>::value, "Invalid type. All possible types have been specialized below.");
    }

    template<typename T>
    // NOLINTNEXTLINE
    T* get_if() {
        static_assert(std::is_same<T, version3::CMD_AUTH_LOGON_CHALLENGE_Server>::value, "Invalid type. All possible types have been specialized below.");
    }

    WOW_LOGIN_MESSAGES_CPP_EXPORT const char* to_string() const;
};

template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_AUTH_LOGON_CHALLENGE_Server* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_AUTH_LOGON_CHALLENGE_Server& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_AUTH_LOGON_PROOF_Server* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_AUTH_LOGON_PROOF_Server& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_REALM_LIST_Server* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_REALM_LIST_Server& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_INITIATE* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_INITIATE& ServerOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_DATA* ServerOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT version3::CMD_XFER_DATA& ServerOpcode::get();

} // namespace version3
} // namespace wow_login_messages
#endif /* WOW_LOGIN_MESSAGES_CPP_VERSION3_HPP */
