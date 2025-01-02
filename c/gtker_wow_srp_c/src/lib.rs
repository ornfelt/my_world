#[cfg(feature = "client")]
pub mod client;
pub mod header_crypto;
#[cfg(feature = "server")]
pub mod server;
mod util;

#[cfg(feature = "values")]
pub use values::*;

#[cfg(feature = "values")]
mod values {
    use std::ffi::c_char;
    use wow_srp::LARGE_SAFE_PRIME_LITTLE_ENDIAN;

    pub const WOW_SRP_CLIENT_HEADER_LENGTH: u8 = 6;

    /// Length of the password verifier returned by `wow_srp_verifier_password_verifier` in bytes.
    ///
    /// `wow_srp` does not support keys of a smaller size than size.
    pub const WOW_SRP_KEY_LENGTH: u8 = 32;

    /// Length of the session keys produced in bytes.
    pub const WOW_SRP_SESSION_KEY_LENGTH: u8 = 40;

    /// Length of the proofs produced and used in bytes.
    pub const WOW_SRP_PROOF_LENGTH: u8 = 20;

    /// Length of the reconnect data used in bytes.
    pub const WOW_SRP_RECONNECT_DATA_LENGTH: u8 = 16;

    /// Generator used by the server implementation.
    ///
    /// This should be passed to the client through `CMD_AUTH_LOGON_CHALLENGE_Server`.
    pub const WOW_SRP_GENERATOR: u8 = 7;

    /// Large safe prime used by the server implementation.
    ///
    /// This should be passed to the client through `CMD_AUTH_LOGON_CHALLENGE_Server`.
    #[no_mangle]
    pub static WOW_SRP_LARGE_SAFE_PRIME_LITTLE_ENDIAN: [u8; 32] = LARGE_SAFE_PRIME_LITTLE_ENDIAN;

    /// Used by `out_error` to signify that everything went well.
    /// You should initialize your `out_error` variable to this since
    /// the variable will not be set to explicit success.
    pub const WOW_SRP_SUCCESS: c_char = 0;
    /// Used by `out_error` to signify that one of the required parameters was null.
    ///
    /// If `out_error` is null errors will not be written.
    pub const WOW_SRP_ERROR_NULL_POINTER: c_char = 1;
    /// Used by `out_error` to signify that the input string was not valid UTF-8.
    pub const WOW_SRP_ERROR_UTF8: c_char = 2;
    /// Used by `out_error` to signify that the username or password string contained disallowed values.
    pub const WOW_SRP_ERROR_CHARACTERS_NOT_ALLOWED_IN_NAME: c_char = 3;
    /// Used by `out_error` to signify that the public key was invalid.
    pub const WOW_SRP_ERROR_INVALID_PUBLIC_KEY: c_char = 4;
    /// Used by `out_error` to signify that the client and server proofs did not match.
    pub const WOW_SRP_ERROR_PROOFS_DO_NOT_MATCH: c_char = 5;
}
