#pragma once

#include <stdint.h>

#define WOW_SRP_CLIENT_HEADER_LENGTH 6


/**
 * Length of the password verifier returned by `wow_srp_verifier_password_verifier` in bytes.
 *
 * `wow_srp` does not support keys of a smaller size than size.
 */
#define WOW_SRP_KEY_LENGTH 32


/**
 * Length of the session keys produced in bytes.
 */
#define WOW_SRP_SESSION_KEY_LENGTH 40


/**
 * Length of the proofs produced and used in bytes.
 */
#define WOW_SRP_PROOF_LENGTH 20


/**
 * Length of the reconnect data used in bytes.
 */
#define WOW_SRP_RECONNECT_DATA_LENGTH 16


/**
 * Generator used by the server implementation.
 *
 * This should be passed to the client through `CMD_AUTH_LOGON_CHALLENGE_Server`.
 */
#define WOW_SRP_GENERATOR 7


/**
 * Used by `out_error` to signify that everything went well.
 * You should initialize your `out_error` variable to this since
 * the variable will not be set to explicit success.
 */
#define WOW_SRP_SUCCESS 0


/**
 * Used by `out_error` to signify that one of the required parameters was null.
 *
 * If `out_error` is null errors will not be written.
 */
#define WOW_SRP_ERROR_NULL_POINTER 1


/**
 * Used by `out_error` to signify that the input string was not valid UTF-8.
 */
#define WOW_SRP_ERROR_UTF8 2


/**
 * Used by `out_error` to signify that the username or password string contained disallowed values.
 */
#define WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME 3


/**
 * Used by `out_error` to signify that the public key was invalid.
 */
#define WOW_SRP_ERROR_INVALID_PUBLIC_KEY 4


/**
 * Used by `out_error` to signify that the client and server proofs did not match.
 */
#define WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH 5


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * Large safe prime used by the server implementation.
 *
 * This should be passed to the client through `CMD_AUTH_LOGON_CHALLENGE_Server`.
 */
extern const uint8_t WOW_SRP_LARGE_SAFE_PRIME_LITTLE_ENDIAN[32];


#ifdef __cplusplus
}  /* extern "C" */
#endif  /* __cplusplus */
