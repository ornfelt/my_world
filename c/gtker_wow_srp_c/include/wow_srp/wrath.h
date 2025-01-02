#pragma once

#include <stdint.h>

#define WRATH_SERVER_HEADER_MINIMUM_LENGTH 4


#define WRATH_SERVER_HEADER_MAXIMUM_LENGTH 5


/**
 * Header crypto for Wrath servers.
 *
 * Created through `wow_srp_wrath_proof_seed_into_client_header_crypto`.
 *
 * This object must manually be freed.
 */
typedef struct WowSrpWrathClientCrypto WowSrpWrathClientCrypto;


typedef struct WowSrpWrathProofSeed WowSrpWrathProofSeed;


/**
 * Header crypto for Wrath servers.
 *
 * Created through `wow_srp_wrath_proof_seed_into_server_header_crypto`.
 *
 * This object must manually be freed.
 */
typedef struct WowSrpWrathServerCrypto WowSrpWrathServerCrypto;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * Creates a proof seed.
 *
 * Can not be null.
 */
WOW_SRP_EXPORT struct WowSrpWrathProofSeed *wow_srp_wrath_proof_seed_new(void);


/**
 * Returns the randomized seed value.
 *
 * Used in `CMD_AUTH_RECONNECT_CHALLENGE_Server`.
 */
WOW_SRP_EXPORT
uint32_t wow_srp_wrath_proof_seed(const struct WowSrpWrathProofSeed *seed,
                                  char *out_error);


/**
 * Converts the seed into a `WowSrpWrathHeaderCrypto` for the client.
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
struct WowSrpWrathClientCrypto *wow_srp_proof_seed_into_wrath_client_crypto(struct WowSrpWrathProofSeed *seed,
                                                                            const char *username,
                                                                            const uint8_t *session_key,
                                                                            uint32_t server_seed,
                                                                            uint8_t *out_client_proof,
                                                                            char *out_error);


/**
 * Converts the seed into a `WowSrpWrathHeaderCrypto` for the server.
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
struct WowSrpWrathServerCrypto *wow_srp_proof_seed_into_wrath_server_crypto(struct WowSrpWrathProofSeed *seed,
                                                                            const char *username,
                                                                            const uint8_t *session_key,
                                                                            const uint8_t *client_proof,
                                                                            uint32_t client_seed,
                                                                            char *out_error);


/**
 * First step of header decryption for Wrath.
 *
 * Created through `wow_srp_wrath_proof_seed_new`.
 */
WOW_SRP_EXPORT void wow_srp_wrath_proof_seed_free(struct WowSrpWrathProofSeed *seed);


/**
 * Encrypts the `data`.
 *
 * You must manually size the `data` to be the appropriate size.
 * For messages sent from the server this is either
 * `WOW_SRP_WRATH_SERVER_MINIMUM_HEADER_LENGTH` or
 * `WOW_SRP_WRATH_SERVER_MAXIMUM_HEADER_LENGTH`, depending on if the first byte
 * has the `0x80` bit set.
 *
 * * `data` is a `length` sized array that will be written to.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 */
WOW_SRP_EXPORT
void wow_srp_wrath_server_crypto_encrypt(struct WowSrpWrathServerCrypto *header,
                                         uint8_t *data,
                                         uint16_t length,
                                         char *out_error);


/**
 * Decrypts the `data`.
 *
 * You must manually size the `data` to be the appropriate size.
 * For messages sent from the client this is `WOW_SRP_CLIENT_HEADER_LENGTH`.
 *
 * * `data` is a `length` sized array that will be written to.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 */
WOW_SRP_EXPORT
void wow_srp_wrath_server_crypto_decrypt(struct WowSrpWrathServerCrypto *header,
                                         uint8_t *data,
                                         uint16_t length,
                                         char *out_error);


/**
 * Free the `WowSrpWrathServerCrypto`.
 *
 * This must manually be done.
 */
WOW_SRP_EXPORT void wow_srp_wrath_server_crypto_free(struct WowSrpWrathServerCrypto *header);


/**
 * Encrypts the `data`.
 *
 * You must manually size the `data` to be the appropriate size.
 * For messages sent from the client this is `WOW_SRP_CLIENT_HEADER_LENGTH`.
 *
 * * `data` is a `length` sized array that will be written to.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 */
WOW_SRP_EXPORT
void wow_srp_wrath_client_crypto_encrypt(struct WowSrpWrathClientCrypto *header,
                                         uint8_t *data,
                                         uint16_t length,
                                         char *out_error);


/**
 * Decrypts the `data`.
 *
 * You must manually size the `data` to be the appropriate size.
 * For messages sent from the server this is either
 * `WOW_SRP_WRATH_SERVER_MINIMUM_HEADER_LENGTH` or
 * `WOW_SRP_WRATH_SERVER_MAXIMUM_HEADER_LENGTH`, depending on if the first byte
 * has the `0x80` bit set.
 *
 * * `data` is a `length` sized array that will be written to.
 * * `out_error` is a pointer to a single `uint8_t` that will be written to.
 *
 * This function can return a null pointer, in which case errors will be in `out_error`.
 * It can return:
 * * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
 */
WOW_SRP_EXPORT
void wow_srp_wrath_client_crypto_decrypt(struct WowSrpWrathClientCrypto *header,
                                         uint8_t *data,
                                         uint16_t length,
                                         char *out_error);


/**
 * Free the `WowSrpWrathClientCrypto`.
 *
 * This must manually be done.
 */
WOW_SRP_EXPORT void wow_srp_wrath_client_crypto_free(struct WowSrpWrathClientCrypto *header);


#ifdef __cplusplus
}  /* extern "C" */
#endif  /* __cplusplus */
