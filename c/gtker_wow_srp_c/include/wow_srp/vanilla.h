#pragma once

#include <stdint.h>

#define WOW_SRP_VANILLA_SERVER_HEADER_LENGTH 4


/**
 * Header crypto for Vanilla.
 *
 * Created through `wow_srp_vanilla_proof_seed_into_*_header_crypto`.
 *
 * This object must manually be freed.
 */
typedef struct WowSrpVanillaHeaderCrypto WowSrpVanillaHeaderCrypto;


/**
 * First step of header decryption for Vanilla.
 *
 * Created through `wow_srp_vanilla_proof_seed_new`.
 */
typedef struct WowSrpVanillaProofSeed WowSrpVanillaProofSeed;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * Creates a proof seed.
 *
 * Can not be null.
 */
WOW_SRP_EXPORT struct WowSrpVanillaProofSeed *wow_srp_vanilla_proof_seed_new(void);


/**
 * Returns the randomized seed value.
 *
 * Used in `CMD_AUTH_RECONNECT_CHALLENGE_Server`.
 */
WOW_SRP_EXPORT
uint32_t wow_srp_vanilla_proof_seed(const struct WowSrpVanillaProofSeed *seed,
                                    char *out_error);


/**
 * Converts the seed into a `WowSrpVanillaHeaderCrypto` for the client.
 *
 * * `username` is a null terminated string no longer than 16 characters.
 * * `session_key` is a `WOW_SRP_SESSION_KEY_LENGTH` array.
 * * `out_client_proof` is a `WOW_SRP_PROOF_LENGTH` array that will be written to.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 * * `WOW_SRP_ERROR_UTF8` if the username/password contains disallowed characters.
 * * `WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME` if the username/password contains disallowed characters.
 */
WOW_SRP_EXPORT
struct WowSrpVanillaHeaderCrypto *wow_srp_vanilla_proof_seed_into_client_header_crypto(struct WowSrpVanillaProofSeed *seed,
                                                                                       const char *username,
                                                                                       const uint8_t *session_key,
                                                                                       uint32_t server_seed,
                                                                                       uint8_t *out_client_proof,
                                                                                       char *out_error);


/**
 * Converts the seed into a `WowSrpVanillaHeaderCrypto` for the server.
 *
 * * `username` is a null terminated string no longer than 16 characters.
 * * `session_key` is a `WOW_SRP_SESSION_KEY_LENGTH` array.
 * * `client_proof` is a `WOW_SRP_PROOF_LENGTH` array.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 * * `WOW_SRP_ERROR_UTF8` if the username/password contains disallowed characters.
 * * `WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME` if the username/password contains disallowed characters.
 */
WOW_SRP_EXPORT
struct WowSrpVanillaHeaderCrypto *wow_srp_vanilla_proof_seed_into_server_header_crypto(struct WowSrpVanillaProofSeed *seed,
                                                                                       const char *username,
                                                                                       const uint8_t *session_key,
                                                                                       const uint8_t *client_proof,
                                                                                       uint32_t client_seed,
                                                                                       char *out_error);


/**
 * Frees the `WowSrpVanillaProofSeed`.
 *
 * This should not normally be called since `wow_srp_vanilla_proof_seed_into_vanilla_*` functions
 * free this object.
 */
WOW_SRP_EXPORT void wow_srp_vanilla_proof_seed_free(struct WowSrpVanillaProofSeed *seed);


/**
 * Encrypts the `data`.
 *
 * You must manually size the `data` to be the appropriate size.
 * For messages sent from the client this is `WOW_SRP_CLIENT_HEADER_LENGTH`,
 * and for messages sent from the server this is `WOW_SRP_VANILLA_SERVER_HEADER_LENGTH`.
 *
 * * `data` is a `length` sized array that will be written to.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 */
WOW_SRP_EXPORT
void wow_srp_vanilla_header_crypto_encrypt(struct WowSrpVanillaHeaderCrypto *header,
                                           uint8_t *data,
                                           uint16_t length,
                                           char *out_error);


/**
 * Decrypts the `data`.
 *
 * You must manually size the `data` to be the appropriate size.
 * For messages sent from the client this is `WOW_SRP_CLIENT_HEADER_LENGTH`,
 * and for messages sent from the server this is `WOW_SRP_VANILLA_SERVER_HEADER_LENGTH`.
 *
 * * `data` is a `length` sized array that will be written to.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 */
WOW_SRP_EXPORT
void wow_srp_vanilla_header_crypto_decrypt(struct WowSrpVanillaHeaderCrypto *header,
                                           uint8_t *data,
                                           uint16_t length,
                                           char *out_error);


/**
 * Free the `WowSrpVanillaHeaderCrypto`.
 *
 * This must manually be done.
 */
WOW_SRP_EXPORT void wow_srp_vanilla_header_crypto_free(struct WowSrpVanillaHeaderCrypto *header);


#ifdef __cplusplus
}  /* extern "C" */
#endif  /* __cplusplus */
