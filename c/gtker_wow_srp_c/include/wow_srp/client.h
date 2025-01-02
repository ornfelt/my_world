#pragma once

#include <stdint.h>

/**
 * Final step of client side authentication.
 *
 * This must be manually freed with `wow_srp_client_free`.
 */
typedef struct WowSrpClient WowSrpClient;


/**
 * First part of the client side authentication.
 *
 * Created through `wow_srp_client_challenge_create`.
 */
typedef struct WowSrpClientChallenge WowSrpClientChallenge;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * Returns the session key as a `WOW_SRP_SESSION_KEY_LENGTH` sized array.
 *
 * This should be used for header decryption.
 *
 * Will return null if `proof` is null.
 */
WOW_SRP_EXPORT const uint8_t *wow_srp_client_session_key(struct WowSrpClient *client);


/**
 * Calculates the client proof for reconnection.
 *
 * * `server_challenge_data` is a `WOW_SRP_RECONNECT_DATA_LENGTH` array.
 * * `out_client_challenge_data` is a `WOW_SRP_RECONNECT_DATA_LENGTH` array that will be written to.
 * * `out_client_proof` is a `WOW_SRP_PROOF_LENGTH` array that will be written to.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 * * `WOW_SRP_ERROR_INVALID_PUBLIC_KEY` if the public key is invalid.
 * * `WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH` if the client proof does not match the server proof.
 */
WOW_SRP_EXPORT
void wow_srp_client_calculate_reconnect_values(struct WowSrpClient *client,
                                               const uint8_t *server_challenge_data,
                                               uint8_t *out_client_challenge_data,
                                               uint8_t *out_client_proof,
                                               char *out_error);


/**
 * Frees a `WowSrpClient`.
 */
WOW_SRP_EXPORT void wow_srp_client_free(struct WowSrpClient *client);


/**
 * Create a `WowSrpClientChallenge`.
 *
 * This should be called after receiving `CMD_AUTH_LOGON_CHALLENGE_Server`.
 *
 * * `username` is a null terminated string no longer than 16 characters.
 * * `password` is a null terminated string no longer than 16 characters.
 * * `large_safe_prime` is a `WOW_SRP_KEY_LENGTH` array.
 * * `server_public_key` is a `WOW_SRP_KEY_LENGTH` array.
 * * `salt` is a `WOW_SRP_KEY_LENGTH` array.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 * * `WOW_SRP_ERROR_UTF8` if the username/password contains disallowed characters.
 * * `WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME` if the username/password contains disallowed characters.
 * * `WOW_SRP_ERROR_INVALID_PUBLIC_KEY` if the public key is invalid.
 * * `WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH` if the client proof does not match the server proof.
 */
WOW_SRP_EXPORT
struct WowSrpClientChallenge *wow_srp_client_challenge_create(const char *username,
                                                              const char *password,
                                                              uint8_t generator,
                                                              const uint8_t *large_safe_prime,
                                                              const uint8_t *server_public_key,
                                                              const uint8_t *salt,
                                                              char *out_error);


/**
 * Returns the client proof as a `WOW_SRP_PROOF_LENGTH` sized array.
 *
 * This should be passed to the client through `CMD_AUTH_LOGON_PROOF_Client`.
 *
 * Will return null if `proof` is null.
 */
WOW_SRP_EXPORT
const uint8_t *wow_srp_client_challenge_client_proof(struct WowSrpClientChallenge *client_challenge);


/**
 * Returns the client proof as a `WOW_SRP_KEY_LENGTH` sized array.
 *
 * This should be passed to the client through `CMD_AUTH_LOGON_PROOF_Client`.
 *
 * Will return null if `proof` is null.
 */
WOW_SRP_EXPORT
const uint8_t *wow_srp_client_challenge_client_public_key(struct WowSrpClientChallenge *client_challenge);


/**
 * Convert the `WowSrpClientChallenge` into a `WowSrpClient` and
 * verify that the server and client proofs match.
 *
 * * `server_proof` is a `WOW_SRP_PROOF_LENGTH` array.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 * * `WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH` if the client proof does not match the server proof.
 */
WOW_SRP_EXPORT
struct WowSrpClient *wow_srp_client_challenge_verify_server_proof(struct WowSrpClientChallenge *client_challenge,
                                                                  const uint8_t *server_proof,
                                                                  char *out_error);


/**
 * Frees a `WowSrpClientChallenge`.
 *
 * This should not normally need to be called since `wow_srp_proof_into_server` will
 * free the proof.
 */
WOW_SRP_EXPORT void wow_srp_client_challenge_free(struct WowSrpClientChallenge *client_challenge);


#ifdef __cplusplus
}  /* extern "C" */
#endif  /* __cplusplus */
