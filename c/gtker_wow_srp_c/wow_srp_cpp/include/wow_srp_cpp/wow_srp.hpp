#pragma once

#include <cstdint>
#include <array>


namespace wow_srp {

constexpr int CLIENT_HEADER_LENGTH = 6;


/**
 * Length of the password verifier returned by `wow_srp_verifier_password_verifier` in bytes.
 *
 * `wow_srp` does not support keys of a smaller size than size.
 */
constexpr int KEY_LENGTH = 32;


/**
 * Length of the session keys produced in bytes.
 */
constexpr int SESSION_KEY_LENGTH = 40;


/**
 * Length of the proofs produced and used in bytes.
 */
constexpr int PROOF_LENGTH = 20;


/**
 * Length of the reconnect data used in bytes.
 */
constexpr int RECONNECT_DATA_LENGTH = 16;


/**
 * Generator used by the server implementation.
 *
 * This should be passed to the client through `CMD_AUTH_LOGON_CHALLENGE_Server`.
 */
constexpr int GENERATOR = 7;


/**
 * Used by `out_error` to signify that everything went well.
 * You should initialize your `out_error` variable to this since
 * the variable will not be set to explicit success.
 */
constexpr int SUCCESS = 0;


/**
 * Used by `out_error` to signify that one of the required parameters was null.
 *
 * If `out_error` is null errors will not be written.
 */
constexpr int ERROR_NULL_POINTER = 1;


/**
 * Used by `out_error` to signify that the input string was not valid UTF-8.
 */
constexpr int ERROR_UTF8 = 2;


/**
 * Used by `out_error` to signify that the username or password string contained disallowed values.
 */
constexpr int ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME = 3;


/**
 * Used by `out_error` to signify that the public key was invalid.
 */
constexpr int ERROR_INVALID_PUBLIC_KEY = 4;


/**
 * Used by `out_error` to signify that the client and server proofs did not match.
 */
constexpr int ERROR_PROOFS_DO_NOT_MATCH = 5;


/**
 * Large safe prime used by the server implementation.
 *
 * This should be passed to the client through `CMD_AUTH_LOGON_CHALLENGE_Server`.
 */
constexpr std::array<uint8_t, 32> LARGE_SAFE_PRIME {0xb7, 0x9b, 0x3e, 0x2a, 0x87, 0x82, 0x3c, 0xab, 0x8f, 0x5e, 0xbf, 0xbf, 0x8e, 0xb1, 0x1, 0x8, 0x53, 0x50, 0x6, 0x29, 0x8b, 0x5b, 0xad, 0xbd, 0x5b, 0x53, 0xe1, 0x89, 0x5e, 0x64, 0x4b, 0x89};




using ProofArray = std::array<uint8_t, PROOF_LENGTH>;

using SessionKeyArray = std::array<uint8_t, SESSION_KEY_LENGTH>;

using KeyArray = std::array<uint8_t, KEY_LENGTH>;

using ReconnectDataArray = std::array<uint8_t, RECONNECT_DATA_LENGTH>;

} // namespace wow_srp