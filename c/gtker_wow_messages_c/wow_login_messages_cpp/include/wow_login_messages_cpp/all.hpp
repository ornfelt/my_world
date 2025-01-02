#ifndef WOW_LOGIN_MESSAGES_CPP_ALL_HPP
#define WOW_LOGIN_MESSAGES_CPP_ALL_HPP

/* clang-format off */

#include "wow_login_messages_cpp/wow_login_messages.hpp"

namespace wow_login_messages {
namespace all {

enum class Locale : uint32_t {
    EN_GB = 1701726018,
    EN_US = 1701729619,
    ES_MX = 1702055256,
    PT_BR = 1886667346,
    FR_FR = 1718765138,
    DE_DE = 1684358213,
    ES_ES = 1702053203,
    PT_PT = 1886670932,
    IT_IT = 1769228628,
    RU_RU = 1920291413,
    KO_KR = 1802455890,
    ZH_TW = 2053657687,
    EN_TW = 1701729367,
    EN_CN = 1701725006,
};

enum class Os : uint32_t {
    WINDOWS = 5728622,
    MAC_OS_X = 5198680,
};

enum class Platform : uint32_t {
    X86 = 7878710,
    POWER_PC = 5263427,
};

enum class ProtocolVersion : uint8_t {
    TWO = 2,
    THREE = 3,
    FIVE = 5,
    SIX = 6,
    SEVEN = 7,
    EIGHT = 8,
};

struct Version {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint16_t build;
};

/* First message sent by the client when attempting to connect. The server will respond with [CMD_AUTH_LOGON_CHALLENGE_Server].
Has the exact same layout as [CMD_AUTH_RECONNECT_CHALLENGE_Client]. */
struct CMD_AUTH_LOGON_CHALLENGE_Client {
    /* Determines which version of messages are used for further communication. */
    ProtocolVersion protocol_version;
    all::Version version;
    Platform platform;
    Os os;
    Locale locale;
    /* Offset in minutes from UTC time. 180 would be UTC+3 */
    int32_t utc_timezone_offset;
    uint32_t client_ip_address;
    /* Real clients can send a maximum of 16 UTF-8 characters. This is not necessarily 16 bytes since one character can be more than one byte.
Real clients will send a fully uppercased username, and will perform authentication calculations on the uppercased version.
Uppercasing in regards to non-ASCII values is little weird. See `https://docs.rs/wow_srp/latest/wow_srp/normalized_string/index.html` for more info. */
    std::string account_name;

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;
};

/* First message sent by the client when attempting to reconnect. The server will respond with [CMD_AUTH_RECONNECT_CHALLENGE_Server].
Has the exact same layout as [CMD_AUTH_LOGON_CHALLENGE_Client]. */
struct CMD_AUTH_RECONNECT_CHALLENGE_Client {
    /* Determines which version of messages are used for further communication. */
    ProtocolVersion protocol_version;
    all::Version version;
    Platform platform;
    Os os;
    Locale locale;
    /* Offset in minutes from UTC time. 180 would be UTC+3 */
    int32_t utc_timezone_offset;
    uint32_t client_ip_address;
    /* Real clients can send a maximum of 16 UTF-8 characters. This is not necessarily 16 bytes since one character can be more than one byte.
Real clients will send a fully uppercased username, and will perform authentication calculations on the uppercased version.
Uppercasing in regards to non-ASCII values is little weird. See `https://docs.rs/wow_srp/latest/wow_srp/normalized_string/index.html` for more info. */
    std::string account_name;

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;
};

class ClientOpcode {
    enum class Opcode {
        NONE = 0xFF,
        CMD_AUTH_LOGON_CHALLENGE = 0,
        CMD_AUTH_RECONNECT_CHALLENGE = 2,
    } opcode;

    union {
        all::CMD_AUTH_LOGON_CHALLENGE_Client CMD_AUTH_LOGON_CHALLENGE;
        all::CMD_AUTH_RECONNECT_CHALLENGE_Client CMD_AUTH_RECONNECT_CHALLENGE;
    };

public:
    WOW_LOGIN_MESSAGES_CPP_EXPORT bool is_none() const noexcept {
        return opcode == Opcode::NONE;
    }
    WOW_LOGIN_MESSAGES_CPP_EXPORT static ClientOpcode read(Reader& reader);

    WOW_LOGIN_MESSAGES_CPP_EXPORT std::vector<unsigned char> write() const;


    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode() : opcode(Opcode::NONE), CMD_AUTH_LOGON_CHALLENGE() {}

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

    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(all::CMD_AUTH_LOGON_CHALLENGE_Client&& obj);
    WOW_LOGIN_MESSAGES_CPP_EXPORT ClientOpcode(all::CMD_AUTH_RECONNECT_CHALLENGE_Client&& obj);

    template<typename T>
    // NOLINTNEXTLINE
    T& get() {
        static_assert(std::is_same<T, all::CMD_AUTH_LOGON_CHALLENGE_Client>::value, "Invalid type. All possible types have been specialized below.");
    }

    template<typename T>
    // NOLINTNEXTLINE
    T* get_if() {
        static_assert(std::is_same<T, all::CMD_AUTH_LOGON_CHALLENGE_Client>::value, "Invalid type. All possible types have been specialized below.");
    }

    WOW_LOGIN_MESSAGES_CPP_EXPORT const char* to_string() const;
};

template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT all::CMD_AUTH_LOGON_CHALLENGE_Client* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT all::CMD_AUTH_LOGON_CHALLENGE_Client& ClientOpcode::get();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT all::CMD_AUTH_RECONNECT_CHALLENGE_Client* ClientOpcode::get_if();
template<>
WOW_LOGIN_MESSAGES_CPP_EXPORT all::CMD_AUTH_RECONNECT_CHALLENGE_Client& ClientOpcode::get();

} // namespace all
} // namespace wow_login_messages
#endif /* WOW_LOGIN_MESSAGES_CPP_ALL_HPP */
