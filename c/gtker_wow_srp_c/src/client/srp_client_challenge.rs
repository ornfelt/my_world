use crate::client::srp_client::WowSrpClient;
use crate::util::read_public_key;
use crate::util::{
    char_ptr_to_string, free_box_ptr, is_null, read_array, retake_ownership, write_error,
};
use crate::WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH;
use std::ffi::c_char;
use wow_srp::client::SrpClientChallenge as SrpClientChallengeInner;

/// First part of the client side authentication.
///
/// Created through `wow_srp_client_challenge_create`.
pub struct WowSrpClientChallenge(SrpClientChallengeInner);

impl WowSrpClientChallenge {
    pub(crate) fn new(inner: SrpClientChallengeInner) -> Self {
        Self(inner)
    }
}

/// Create a `WowSrpClientChallenge`.
///
/// This should be called after receiving `CMD_AUTH_LOGON_CHALLENGE_Server`.
///
/// * `username` is a null terminated string no longer than 16 characters.
/// * `password` is a null terminated string no longer than 16 characters.
/// * `large_safe_prime` is a `WOW_SRP_KEY_LENGTH` array.
/// * `server_public_key` is a `WOW_SRP_KEY_LENGTH` array.
/// * `salt` is a `WOW_SRP_KEY_LENGTH` array.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
/// * `WOW_SRP_ERROR_UTF8` if the username/password contains disallowed characters.
/// * `WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME` if the username/password contains disallowed characters.
/// * `WOW_SRP_ERROR_INVALID_PUBLIC_KEY` if the public key is invalid.
/// * `WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH` if the client proof does not match the server proof.
#[no_mangle]
pub extern "C" fn wow_srp_client_challenge_create(
    username: *const c_char,
    password: *const c_char,
    generator: u8,
    large_safe_prime: *const u8,
    server_public_key: *const u8,
    salt: *const u8,
    out_error: *mut c_char,
) -> *mut WowSrpClientChallenge {
    let Some(username) = char_ptr_to_string(username, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(password) = char_ptr_to_string(password, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(large_safe_prime) = read_array(large_safe_prime, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(server_public_key) = read_public_key(server_public_key, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(salt) = read_array(salt, out_error) else {
        return std::ptr::null_mut();
    };

    let challenge = SrpClientChallengeInner::new(
        username,
        password,
        generator,
        large_safe_prime,
        server_public_key,
        salt,
    );

    let challenge = Box::new(WowSrpClientChallenge::new(challenge));

    Box::into_raw(challenge)
}

/// Returns the client proof as a `WOW_SRP_PROOF_LENGTH` sized array.
///
/// This should be passed to the client through `CMD_AUTH_LOGON_PROOF_Client`.
///
/// Will return null if `proof` is null.
#[no_mangle]
pub extern "C" fn wow_srp_client_challenge_client_proof(
    client_challenge: *mut WowSrpClientChallenge,
) -> *const u8 {
    let Some(server) = is_null(client_challenge, std::ptr::null_mut()) else {
        return std::ptr::null();
    };

    server.0.client_proof() as *const u8
}

/// Returns the client proof as a `WOW_SRP_KEY_LENGTH` sized array.
///
/// This should be passed to the client through `CMD_AUTH_LOGON_PROOF_Client`.
///
/// Will return null if `proof` is null.
#[no_mangle]
pub extern "C" fn wow_srp_client_challenge_client_public_key(
    client_challenge: *mut WowSrpClientChallenge,
) -> *const u8 {
    let Some(server) = is_null(client_challenge, std::ptr::null_mut()) else {
        return std::ptr::null();
    };

    server.0.client_public_key() as *const u8
}

/// Convert the `WowSrpClientChallenge` into a `WowSrpClient` and
/// verify that the server and client proofs match.
///
/// * `server_proof` is a `WOW_SRP_PROOF_LENGTH` array.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
/// * `WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH` if the client proof does not match the server proof.
#[no_mangle]
pub extern "C" fn wow_srp_client_challenge_verify_server_proof(
    client_challenge: *mut WowSrpClientChallenge,
    server_proof: *const u8,
    out_error: *mut c_char,
) -> *mut WowSrpClient {
    let Some(server_proof) = read_array(server_proof, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(client_challenge) = retake_ownership(client_challenge, out_error) else {
        return std::ptr::null_mut();
    };

    let Ok(client) = (*client_challenge).0.verify_server_proof(server_proof) else {
        write_error(out_error, WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH);
        return std::ptr::null_mut();
    };

    let client = WowSrpClient::new(client);
    let client = Box::new(client);

    Box::into_raw(client)
}

/// Frees a `WowSrpClientChallenge`.
///
/// This should not normally need to be called since `wow_srp_proof_into_server` will
/// free the proof.
#[no_mangle]
pub extern "C" fn wow_srp_client_challenge_free(client_challenge: *mut WowSrpClientChallenge) {
    free_box_ptr(client_challenge)
}
