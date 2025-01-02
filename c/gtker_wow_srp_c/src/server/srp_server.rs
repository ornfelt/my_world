use crate::util::{free_box_ptr, is_null, is_null_mut, read_array};
use std::ffi::c_char;
use wow_srp::server::SrpServer as SrpServerInner;

/// Last step of server side authentication.
///
/// Created through `wow_srp_proof_into_server`.
///
/// This object must be manually freed through `wow_srp_server_free`.
pub struct WowSrpServer(SrpServerInner);

impl WowSrpServer {
    pub fn new(inner: SrpServerInner) -> Self {
        Self(inner)
    }
}

/// Returns the session key as a `WOW_SRP_SESSION_KEY_LENGTH` sized array.
///
/// This should be passed to the client through `CMD_AUTH_LOGON_CHALLENGE_Server`.
///
/// Will return null if `proof` is null.
#[no_mangle]
pub extern "C" fn wow_srp_server_session_key(server: *const WowSrpServer) -> *const u8 {
    let Some(server) = is_null(server, std::ptr::null_mut()) else {
        return std::ptr::null();
    };

    server.0.session_key().as_ptr()
}

/// Returns the reconnect data as a `WOW_SRP_RECONNECT_DATA_LENGTH` sized array.
///
/// This should be passed to the client through `CMD_AUTH_RECONNECT_CHALLENGE_Server`.
///
/// Will return null if `proof` is null.
#[no_mangle]
pub extern "C" fn wow_srp_server_reconnect_challenge_data(
    server: *const WowSrpServer,
) -> *const u8 {
    let Some(server) = is_null(server, std::ptr::null_mut()) else {
        return std::ptr::null();
    };

    server.0.reconnect_challenge_data().as_ptr()
}

/// Returns true if the client proof matches the server proof.
///
/// * `client_data` is a `WOW_SRP_RECONNECT_DATA_LENGTH` length array.
/// * `client_proof` is a `WOW_SRP_PROOF_LENGTH` length array.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
/// * `WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH` if the client proof does not match the server proof.
#[no_mangle]
pub extern "C" fn wow_srp_server_verify_reconnection_attempt(
    server: *mut WowSrpServer,
    client_data: *const u8,
    client_proof: *const u8,
    out_error: *mut c_char,
) -> bool {
    let Some(server) = is_null_mut(server, out_error) else {
        return false;
    };

    let Some(client_data) = read_array(client_data, out_error) else {
        return false;
    };

    let Some(client_proof) = read_array(client_proof, out_error) else {
        return false;
    };

    server
        .0
        .verify_reconnection_attempt(client_data, client_proof)
}

/// Frees a `WowSrpServer`.
///
/// This must be called manually since no other function will free it.
#[no_mangle]
pub extern "C" fn wow_srp_server_free(server: *mut WowSrpServer) {
    free_box_ptr(server)
}
