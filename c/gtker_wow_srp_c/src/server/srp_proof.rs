use crate::server::srp_server::WowSrpServer;
use crate::util::{free_box_ptr, is_null, read_array, read_public_key, write_array, write_error};
use crate::{WOW_SRP_ERROR_NULL_POINTER, WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH};
use std::ffi::c_char;
use wow_srp::server::SrpProof as SrpProofInner;

/// Second step of server side authentication.
///
/// Created through `wow_srp_verifier_into_proof`.
pub struct WowSrpProof(SrpProofInner);

impl WowSrpProof {
    pub(crate) fn new(inner: SrpProofInner) -> Self {
        Self(inner)
    }
}

/// Returns the server public key as a `WOW_SRP_KEY_LENGTH` sized array.
///
/// This should be passed to the client through `CMD_AUTH_LOGON_CHALLENGE_Server`.
///
/// Will return null if `proof` is null.
#[no_mangle]
pub extern "C" fn wow_srp_proof_server_public_key(proof: *const WowSrpProof) -> *const u8 {
    let Some(proof) = is_null(proof, std::ptr::null_mut()) else {
        return std::ptr::null();
    };

    proof.0.server_public_key() as *const u8
}

/// Returns the salt as a `WOW_SRP_KEY_LENGTH` sized array.
///
/// This should be passed to the client through `CMD_AUTH_LOGON_CHALLENGE_Server`.
///
/// Will return null if `proof` is null.
#[no_mangle]
pub extern "C" fn wow_srp_proof_salt(proof: *const WowSrpProof) -> *const u8 {
    let Some(proof) = is_null(proof, std::ptr::null_mut()) else {
        return std::ptr::null();
    };

    proof.0.salt() as *const u8
}

/// Convert the `WowSrpProof` into a `WowSrpServer`.
///
/// This should be called after receiving the client public key and proof from the client in
/// `CMD_AUTH_LOGON_PROOF_Client`.
///
/// * `client_public_key` is a `WOW_SRP_KEY_LENGTH` array.
/// * `client_proof` is a `WOW_SRP_PROOF_LENGTH` array.
/// * `out_server_proof` is a `WOW_SRP_PROOF_LENGTH` array that will be written to.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
/// * `WOW_SRP_ERROR_INVALID_PUBLIC_KEY` if the public key is invalid.
/// * `WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH` if the client proof does not match the server proof.
#[no_mangle]
pub extern "C" fn wow_srp_proof_into_server(
    proof: *mut WowSrpProof,
    client_public_key: *const u8,
    client_proof: *const u8,
    out_server_proof: *mut u8,
    out_error: *mut c_char,
) -> *mut WowSrpServer {
    if proof.is_null() {
        return std::ptr::null_mut();
    }

    let proof = unsafe { Box::from_raw(proof) };

    let Some(client_proof) = read_array(client_proof, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(client_public_key) = read_public_key(client_public_key, out_error) else {
        return std::ptr::null_mut();
    };

    let Ok((server, server_proof)) = (*proof).0.into_server(client_public_key, client_proof) else {
        write_error(out_error, WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH);
        return std::ptr::null_mut();
    };

    if out_server_proof.is_null() {
        write_error(out_error, WOW_SRP_ERROR_NULL_POINTER);
        return std::ptr::null_mut();
    } else {
        write_array(out_server_proof, server_proof.as_slice());
    }

    let server = Box::new(WowSrpServer::new(server));

    Box::into_raw(server)
}

/// Frees a `WowSrpProof`.
///
/// This should not normally need to be called since `wow_srp_proof_into_server` will
/// free the proof.
#[no_mangle]
pub extern "C" fn wow_srp_proof_free(proof: *mut WowSrpProof) {
    free_box_ptr(proof)
}
