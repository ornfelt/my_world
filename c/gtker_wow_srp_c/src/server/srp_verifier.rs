use crate::server::srp_proof::WowSrpProof;
use crate::util::{char_ptr_to_string, free_box_ptr, is_null, read_array, retake_ownership};
use std::ffi::c_char;
use wow_srp::server::SrpVerifier as SrpVerifierInner;

/// First step of Server authentication.
/// Converted into a `WowSrpProof` by calling `wow_srp_verifier_into_proof`.
pub struct WowSrpVerifier(SrpVerifierInner);

/// Creates a `WowSrpVerifier` from a username and password.
/// This should only be used the very first time that a client authenticates.
/// The username, salt, and password verifier should be stored in the database for future lookup,
/// and `wow_srp_verifier_from_database_values` should then be called.
///
/// * `username` is a null terminated string no longer than 16 characters.
/// * `password` is a null terminated string no longer than 16 characters.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
/// * `WOW_SRP_ERROR_UTF8` if the username/password contains disallowed characters.
/// * `WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME` if the username/password contains disallowed characters.
#[no_mangle]
pub extern "C" fn wow_srp_verifier_from_username_and_password(
    username: *const c_char,
    password: *const c_char,
    out_error: *mut c_char,
) -> *mut WowSrpVerifier {
    let Some(username) = char_ptr_to_string(username, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(password) = char_ptr_to_string(password, out_error) else {
        return std::ptr::null_mut();
    };

    let v = Box::new(WowSrpVerifier(
        SrpVerifierInner::from_username_and_password(username, password),
    ));

    Box::into_raw(v)
}

/// Creates a `WowSrpVerifier` from a username, password verifier, and salt
/// previously generated from `wow_srp_verifier_from_username_and_password`.
///
/// * `username` is a null terminated string no longer than 16 characters.
/// * `password_verifier` is a `WOW_SRP_KEY_LENGTH` array.
/// * `salt` is a `WOW_SRP_KEY_LENGTH` array.
/// * `out_error` is a pointer to a single `uint8_t` that will be written to.
///
/// This function can return a null pointer, in which case errors will be in `out_error`.
/// It can return:
/// * `WOW_SRP_ERROR_NULL_POINTER` if any pointer is null.
/// * `WOW_SRP_ERROR_UTF8` if the username/password contains disallowed characters.
/// * `WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME` if the username/password contains disallowed characters.
#[no_mangle]
pub extern "C" fn wow_srp_verifier_from_database_values(
    username: *const c_char,
    password_verifier: *const u8,
    salt: *const u8,
    out_error: *mut c_char,
) -> *mut WowSrpVerifier {
    let Some(username) = char_ptr_to_string(username, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(password_verifier) = read_array(password_verifier, out_error) else {
        return std::ptr::null_mut();
    };

    let Some(salt) = read_array(salt, out_error) else {
        return std::ptr::null_mut();
    };

    let b = Box::new(WowSrpVerifier(SrpVerifierInner::from_database_values(
        username,
        password_verifier,
        salt,
    )));

    Box::into_raw(b)
}

/// Converts the `WowSrpVerifier` into a `WowSrpProof`.
///
/// This ends the lifetime of the `WowSrpVerifier` and it must not be used again.
///
/// Will return null if `verifier` is null.
#[no_mangle]
pub extern "C" fn wow_srp_verifier_into_proof(verifier: *mut WowSrpVerifier) -> *mut WowSrpProof {
    let Some(v) = retake_ownership(verifier, std::ptr::null_mut()) else {
        return std::ptr::null_mut();
    };
    let v = Box::new(WowSrpProof::new(v.0.into_proof()));

    Box::into_raw(v)
}

/// Returns the salt as a `WOW_SRP_KEY_LENGTH` sized byte array.
///
/// This should be stored in the database for future lookup.
///
/// The lifetime of this is tied to the lifetime of the `WowSrpVerifier`.
///
/// Will return null if `verifier` is null.
#[no_mangle]
pub extern "C" fn wow_srp_verifier_salt(verifier: *const WowSrpVerifier) -> *const u8 {
    let Some(verifier) = is_null(verifier, std::ptr::null_mut()) else {
        return std::ptr::null();
    };

    verifier.0.salt() as *const u8
}

/// Returns the password verifier as a `WOW_SRP_KEY_LENGTH` sized byte array.
///
/// This should be stored in the database for future lookup.
///
/// The lifetime of this is tied to the lifetime of the `WowSrpVerifier`.
///
/// Will return null if `verifier` is null.
#[no_mangle]
pub extern "C" fn wow_srp_verifier_password_verifier(verifier: *const WowSrpVerifier) -> *const u8 {
    let Some(verifier) = is_null(verifier, std::ptr::null_mut()) else {
        return std::ptr::null();
    };

    verifier.0.password_verifier() as *const u8
}

/// Frees a `WowSrpVerifier`.
///
/// This should not normally need to be called since `wow_srp_verifier_into_proof` will
/// free the verifier.
#[no_mangle]
pub extern "C" fn wow_srp_verifier_free(verifier: *mut WowSrpVerifier) {
    free_box_ptr(verifier)
}
