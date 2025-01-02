use crate::util::is_null_mut;
use crate::util::{
    char_ptr_to_string, free_box_ptr, is_null, read_array, retake_ownership, write_array,
    write_error,
};
use crate::WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH;
use std::ffi::c_char;
use wow_srp::vanilla_header::HeaderCrypto as HeaderCryptoInner;
use wow_srp::vanilla_header::ProofSeed as ProofSeedInner;

pub const WOW_SRP_VANILLA_SERVER_HEADER_LENGTH: u8 = 4;

/// First step of header decryption for Vanilla.
///
/// Created through `wow_srp_vanilla_proof_seed_new`.
pub struct WowSrpVanillaProofSeed(ProofSeedInner);

/// Creates a proof seed.
///
/// Can not be null.
#[no_mangle]
pub extern "C" fn wow_srp_vanilla_proof_seed_new() -> *mut WowSrpVanillaProofSeed {
    let seed = Box::new(WowSrpVanillaProofSeed(ProofSeedInner::new()));

    Box::into_raw(seed)
}

/// Returns the randomized seed value.
///
/// Used in `CMD_AUTH_RECONNECT_CHALLENGE_Server`.
#[no_mangle]
pub extern "C" fn wow_srp_vanilla_proof_seed(
    seed: *const WowSrpVanillaProofSeed,
    out_error: *mut c_char,
) -> u32 {
    let Some(seed) = is_null(seed, out_error) else {
        return 0;
    };

    seed.0.seed()
}

/// Converts the seed into a `WowSrpVanillaHeaderCrypto` for the client.
///
/// * `username` is a null terminated string no longer than 16 characters.
/// * `session_key` is a `WOW_SRP_SESSION_KEY_LENGTH` array.
/// * `out_client_proof` is a `WOW_SRP_PROOF_LENGTH` array that will be written to.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
/// * `WOW_SRP_ERROR_UTF8` if the username/password contains disallowed characters.
/// * `WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME` if the username/password contains disallowed characters.
#[no_mangle]
pub extern "C" fn wow_srp_vanilla_proof_seed_into_client_header_crypto(
    seed: *mut WowSrpVanillaProofSeed,
    username: *const c_char,
    session_key: *const u8,
    server_seed: u32,
    out_client_proof: *mut u8,
    out_error: *mut c_char,
) -> *mut WowSrpVanillaHeaderCrypto {
    let Some(seed) = retake_ownership(seed, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(username) = char_ptr_to_string(username, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(session_key) = read_array(session_key, out_error) else {
        return std::ptr::null_mut();
    };

    let (client_proof, header_crypto) =
        (*seed)
            .0
            .into_client_header_crypto(&username, session_key, server_seed);

    write_array(out_client_proof, client_proof.as_slice());

    let header_crypto = Box::new(WowSrpVanillaHeaderCrypto::new(header_crypto));

    Box::into_raw(header_crypto)
}

/// Converts the seed into a `WowSrpVanillaHeaderCrypto` for the server.
///
/// * `username` is a null terminated string no longer than 16 characters.
/// * `session_key` is a `WOW_SRP_SESSION_KEY_LENGTH` array.
/// * `client_proof` is a `WOW_SRP_PROOF_LENGTH` array.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
/// * `WOW_SRP_ERROR_UTF8` if the username/password contains disallowed characters.
/// * `WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME` if the username/password contains disallowed characters.
#[no_mangle]
pub extern "C" fn wow_srp_vanilla_proof_seed_into_server_header_crypto(
    seed: *mut WowSrpVanillaProofSeed,
    username: *const c_char,
    session_key: *const u8,
    client_proof: *const u8,
    client_seed: u32,
    out_error: *mut c_char,
) -> *mut WowSrpVanillaHeaderCrypto {
    let Some(seed) = retake_ownership(seed, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(username) = char_ptr_to_string(username, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(session_key) = read_array(session_key, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(client_proof) = read_array(client_proof, out_error) else {
        return std::ptr::null_mut();
    };

    let Ok(header) =
        (*seed)
            .0
            .into_server_header_crypto(&username, session_key, client_proof, client_seed)
    else {
        write_error(out_error, WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH);
        return std::ptr::null_mut();
    };

    let header_crypto = Box::new(WowSrpVanillaHeaderCrypto::new(header));

    Box::into_raw(header_crypto)
}

/// Frees the `WowSrpVanillaProofSeed`.
///
/// This should not normally be called since `wow_srp_vanilla_proof_seed_into_vanilla_*` functions
/// free this object.
#[no_mangle]
pub extern "C" fn wow_srp_vanilla_proof_seed_free(seed: *mut WowSrpVanillaProofSeed) {
    free_box_ptr(seed)
}

/// Header crypto for Vanilla.
///
/// Created through `wow_srp_vanilla_proof_seed_into_*_header_crypto`.
///
/// This object must manually be freed.
pub struct WowSrpVanillaHeaderCrypto(HeaderCryptoInner);

impl WowSrpVanillaHeaderCrypto {
    pub fn new(inner: HeaderCryptoInner) -> Self {
        Self(inner)
    }
}

/// Encrypts the `data`.
///
/// You must manually size the `data` to be the appropriate size.
/// For messages sent from the client this is `WOW_SRP_CLIENT_HEADER_LENGTH`,
/// and for messages sent from the server this is `WOW_SRP_VANILLA_SERVER_HEADER_LENGTH`.
///
/// * `data` is a `length` sized array that will be written to.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
#[no_mangle]
pub extern "C" fn wow_srp_vanilla_header_crypto_encrypt(
    header: *mut WowSrpVanillaHeaderCrypto,
    data: *mut u8,
    length: u16,
    out_error: *mut c_char,
) {
    if length == 0 {
        return;
    }

    let Some(header) = is_null_mut(header, out_error) else {
        return;
    };

    let data = unsafe { std::slice::from_raw_parts_mut(data, length.into()) };

    header.0.encrypt(data);
}

/// Decrypts the `data`.
///
/// You must manually size the `data` to be the appropriate size.
/// For messages sent from the client this is `WOW_SRP_CLIENT_HEADER_LENGTH`,
/// and for messages sent from the server this is `WOW_SRP_VANILLA_SERVER_HEADER_LENGTH`.
///
/// * `data` is a `length` sized array that will be written to.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
#[no_mangle]
pub extern "C" fn wow_srp_vanilla_header_crypto_decrypt(
    header: *mut WowSrpVanillaHeaderCrypto,
    data: *mut u8,
    length: u16,
    out_error: *mut c_char,
) {
    if length == 0 {
        return;
    }

    let Some(header) = is_null_mut(header, out_error) else {
        return;
    };

    let data = unsafe { std::slice::from_raw_parts_mut(data, length.into()) };

    header.0.decrypt(data);
}

/// Free the `WowSrpVanillaHeaderCrypto`.
///
/// This must manually be done.
#[no_mangle]
pub extern "C" fn wow_srp_vanilla_header_crypto_free(header: *mut WowSrpVanillaHeaderCrypto) {
    free_box_ptr(header)
}
