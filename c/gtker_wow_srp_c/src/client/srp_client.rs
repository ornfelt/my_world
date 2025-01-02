use crate::util::{free_box_ptr, is_null, is_null_mut, read_array};
use std::ffi::c_char;
use wow_srp::client::SrpClient as SrpClientInner;

/// Final step of client side authentication.
///
/// This must be manually freed with `wow_srp_client_free`.
pub struct WowSrpClient(SrpClientInner);

impl WowSrpClient {
    pub(crate) fn new(inner: SrpClientInner) -> Self {
        Self(inner)
    }
}

/// Returns the session key as a `WOW_SRP_SESSION_KEY_LENGTH` sized array.
///
/// This should be used for header decryption.
///
/// Will return null if `proof` is null.
#[no_mangle]
pub extern "C" fn wow_srp_client_session_key(client: *mut WowSrpClient) -> *const u8 {
    let Some(client) = is_null_mut(client, std::ptr::null_mut()) else {
        return std::ptr::null();
    };

    client.0.session_key().as_ptr()
}

/// Calculates the client proof for reconnection.
///
/// * `server_challenge_data` is a `WOW_SRP_RECONNECT_DATA_LENGTH` array.
/// * `out_client_challenge_data` is a `WOW_SRP_RECONNECT_DATA_LENGTH` array that will be written to.
/// * `out_client_proof` is a `WOW_SRP_PROOF_LENGTH` array that will be written to.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
/// * `WOW_SRP_ERROR_INVALID_PUBLIC_KEY` if the public key is invalid.
/// * `WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH` if the client proof does not match the server proof.
#[no_mangle]
pub extern "C" fn wow_srp_client_calculate_reconnect_values(
    client: *mut WowSrpClient,
    server_challenge_data: *const u8,
    out_client_challenge_data: *mut u8,
    out_client_proof: *mut u8,
    out_error: *mut c_char,
) {
    let Some(client) = is_null(client, out_error) else {
        return;
    };

    let Some(server_challenge_data) = read_array(server_challenge_data, out_error) else {
        return;
    };

    let reconnect_data = (*client)
        .0
        .calculate_reconnect_values(server_challenge_data);

    for (i, d) in reconnect_data.challenge_data.iter().enumerate() {
        unsafe { out_client_challenge_data.offset(i as isize).write(*d) };
    }

    for (i, d) in reconnect_data.proof.iter().enumerate() {
        unsafe { out_client_proof.offset(i as isize).write(*d) };
    }
}

/// Frees a `WowSrpClient`.
#[no_mangle]
pub extern "C" fn wow_srp_client_free(client: *mut WowSrpClient) {
    free_box_ptr(client)
}
