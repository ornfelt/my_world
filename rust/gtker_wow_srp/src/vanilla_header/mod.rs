//! Functionality for encrypting/decrypting [World Packet] headers.
//!
//! For unknown reasons the session key obtained during the SRP6
//! exchange is used to "encrypt" packet headers.
//! Be aware that [Login Packets] are not encrypted in this way.
//!
//! The packet headers are different length depending on if they are
//! [client](CLIENT_HEADER_LENGTH) or [server](SERVER_HEADER_LENGTH) headers.
//!
//! The sending party will encrypt the packets they send using an [`EncrypterHalf`] and the receiving
//! party will decrypt with a [`DecrypterHalf`].
//! The [`HeaderCrypto`] struct contains both and can be split with [`HeaderCrypto::split`].
//!
//! The [Typestate](https://yoric.github.io/post/rust-typestate/) pattern is used
//! in order to prevent incorrect use.
//! This means that whenever the next step of computation takes place, you call a function
//! taking `self`, consuming the old object, and returning the new object.
//!
//! When a player connects to the world server, the server will need to send a seed value
//! in the [`SMSG_AUTH_CHALLENGE`] message before the username has been received in the
//! [`CMSG_AUTH_SESSION`] message.
//!
//! This means the following workflow has to be done:
//!
//! 1. Create a [`ProofSeed`] struct containing a randomly generated `u32` seed.
//! 2. Send the seed to the client in a [`SMSG_AUTH_CHALLENGE`] message.
//! 3. Receive the username, proof and seed in the [`CMSG_AUTH_SESSION`] message.
//! 4. Retrieve the session key from the login server.
//! 5. Create the [`HeaderCrypto`] struct through [`ProofSeed::into_server_header_crypto`].
//! 6. Optionally, split the [`HeaderCrypto`] into [`EncrypterHalf`] and [`DecrypterHalf`] through
//! [`HeaderCrypto::split`].
//! 7. Optionally, unsplit them through [`EncrypterHalf::unsplit`].
//!
//! In a diagram this would look like:
//! ```text
//!                         Optional
//!                            |
//!                            |   |-> EncrypterHalf -|
//! ProofSeed -> HeaderCrypto -|---|                  |--> HeaderCrypto
//!                            |   |-> DecrypterHalf -|
//!                            |
//! ```
//!
//! # Example
//!
//! After establishing a successful connection to the world server individual headers can be
//! encrypted or decrypted through a few different means:
//!
//! ```
//! use std::io::{Read, Error, Write};
//! use wow_srp::vanilla_header::{HeaderCrypto, ServerHeader, ProofSeed};
//! use core::convert::TryInto;
//! use wow_srp::{SESSION_KEY_LENGTH, PROOF_LENGTH};
//! use wow_srp::normalized_string::NormalizedString;
//!
//! fn establish_connection(username: NormalizedString,
//!                         session_key: [u8; SESSION_KEY_LENGTH as _],
//!                         client_proof: [u8; PROOF_LENGTH as _],
//!                         client_seed: u32) {
//!     let seed = ProofSeed::new();
//!     // Send seed to client
//!     seed.seed();
//!     // Get username from client, fetch session key from login server
//!     let encryption = seed.into_server_header_crypto(&username, session_key, client_proof, client_seed);
//!
//!     // Send the first server message
//! }
//!
//! fn decrypt_header<R: Read>(r: R, raw_data: &mut [u8], encryption: &mut HeaderCrypto) {
//!     let server_header = encryption.read_and_decrypt_server_header(r);
//!     match server_header {
//!         Ok(c) => {}
//!         Err(_) => {
//!             panic!("Reader error")
//!         }
//!     }
//!
//!     // OR
//!
//!     let header = raw_data[0..6].try_into().unwrap();
//!     let server_header = encryption.decrypt_server_header(header);
//!
//!     // OR
//!
//!     encryption.decrypt(raw_data);
//! }
//!
//! fn encrypt<W: Write>(w: W, raw_data: &mut [u8], encryption: &mut HeaderCrypto) {
//!     let result = encryption.write_encrypted_server_header(w, 4, 0xFF);
//!     match result {
//!         Ok(_) => {}
//!         Err(_) => {
//!             panic!("Reader error")
//!         }
//!     }
//!
//!     // OR
//!
//!     let server_header = encryption.encrypt_server_header(4, 0xFF);
//!     // Send server_header
//!
//!     // OR
//!
//!     encryption.encrypt(raw_data);
//! }
//!
//! ```
//!
//! [World Packet]: https://wowdev.wiki/World_Packet
//! [Login Packets]: https://wowdev.wiki/Login_Packet
//! [`SMSG_AUTH_CHALLENGE`]: https://wowdev.wiki/SMSG_AUTH_CHALLENGE
//! [`CMSG_AUTH_SESSION`]: https://wowdev.wiki/SMSG_AUTH_SESSION

use std::io::{Read, Write};

pub use decrypt::DecrypterHalf;
pub use encrypt::EncrypterHalf;

use crate::error::MatchProofsError;
use crate::key::{Proof, SessionKey};
use crate::normalized_string::NormalizedString;
use crate::{PROOF_LENGTH, SESSION_KEY_LENGTH};
use rand::{thread_rng, RngCore};

pub(crate) mod decrypt;
pub(crate) mod encrypt;
mod internal;

pub(crate) use internal::calculate_world_server_proof;

/// Size in bytes of the client [world packet] header.
///
/// Always 6 bytes because the size is 2 bytes and the opcode is 4 bytes.
///
/// [world packet]: https://wowdev.wiki/World_Packet
pub const CLIENT_HEADER_LENGTH: u8 =
    (core::mem::size_of::<u16>() + core::mem::size_of::<u32>()) as u8;
/// Size in bytes of the server [world packet] header.
///
/// Always 4 bytes because the size is 2 bytes and the opcode is 2 bytes.
///
/// [world packet]: https://wowdev.wiki/World_Packet
pub const SERVER_HEADER_LENGTH: u8 =
    (core::mem::size_of::<u16>() + core::mem::size_of::<u16>()) as u8;

/// Decrypted values from a server.
///
/// Gotten from either the various reader methods.
#[derive(Debug, Clone, Copy, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct ServerHeader {
    /// Size of the message in bytes.
    /// Includes the opcode field but not the size field
    pub size: u16,
    /// Opcode of the message. Note that the size is not the same as the [`ClientHeader`].
    pub opcode: u16,
}

impl ServerHeader {
    /// Construct a header from **unencrypted** data.
    pub const fn from_array(b: [u8; SERVER_HEADER_LENGTH as usize]) -> Self {
        let size = u16::from_be_bytes([b[0], b[1]]);
        let opcode = u16::from_le_bytes([b[2], b[3]]);

        Self { size, opcode }
    }
}

/// Decrypted values from a client.
///
/// Gotten from either the various reader methods.
#[derive(Debug, Clone, Copy, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct ClientHeader {
    /// Size of the message in bytes.
    /// Includes the opcode field but not the size field
    pub size: u16,
    /// Opcode of the message. Note that the size is not the same as the [`ServerHeader`].
    pub opcode: u32,
}

impl ClientHeader {
    /// Construct a header from **unencrypted** data.
    pub const fn from_array(b: [u8; CLIENT_HEADER_LENGTH as usize]) -> Self {
        let size: u16 = u16::from_be_bytes([b[0], b[1]]);
        let opcode: u32 = u32::from_le_bytes([b[2], b[3], b[4], b[5]]);

        Self { size, opcode }
    }
}

/// Main struct for encryption or decryption.
///
/// Created from [`ProofSeed::into_server_header_crypto`].
///
/// Handles both encryption and decryption of headers through the
/// [`EncrypterHalf`] and [`DecrypterHalf`] structs.
///
/// Can be split into a [`EncrypterHalf`] and [`DecrypterHalf`] through
/// the [`HeaderCrypto::split`] method. This is useful if you have this struct behind a
/// mutex and don't want to lock both reading and writing at the same time.
#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct HeaderCrypto {
    decrypt: DecrypterHalf,
    encrypt: EncrypterHalf,
}

impl HeaderCrypto {
    /// Direct access to the internal [`DecrypterHalf`].
    #[must_use]
    pub fn decrypter(&mut self) -> &mut DecrypterHalf {
        &mut self.decrypt
    }

    /// Direct access to the internal [`EncrypterHalf`].
    #[must_use]
    pub fn encrypter(&mut self) -> &mut EncrypterHalf {
        &mut self.encrypt
    }

    /// Use either [the client](Self::write_encrypted_client_header)
    /// or [the server](Self::write_encrypted_server_header)
    /// [`Write`] functions, or
    /// [the client](Self::encrypt_client_header)
    /// or [the server](Self::encrypt_server_header) array functions.
    pub fn encrypt(&mut self, data: &mut [u8]) {
        self.encrypt.encrypt(data);
    }

    /// Convenience wrapper for [`EncrypterHalf::write_encrypted_server_header`].
    ///
    /// # Errors
    ///
    /// Has the same errors as [`EncrypterHalf::write_encrypted_server_header`].
    pub fn write_encrypted_server_header<W: Write>(
        &mut self,
        write: W,
        size: u16,
        opcode: u16,
    ) -> std::io::Result<()> {
        self.encrypt
            .write_encrypted_server_header(write, size, opcode)
    }

    /// Convenience wrapper for [`EncrypterHalf::write_encrypted_client_header`].
    ///
    /// # Errors
    ///
    /// Has the same errors as [`EncrypterHalf::write_encrypted_client_header`].
    pub fn write_encrypted_client_header<W: Write>(
        &mut self,
        write: W,
        size: u16,
        opcode: u32,
    ) -> std::io::Result<()> {
        self.encrypt
            .write_encrypted_client_header(write, size, opcode)
    }

    /// Convenience wrapper for [`EncrypterHalf::encrypt_server_header`].
    #[must_use]
    pub fn encrypt_server_header(
        &mut self,
        size: u16,
        opcode: u16,
    ) -> [u8; SERVER_HEADER_LENGTH as usize] {
        self.encrypt.encrypt_server_header(size, opcode)
    }

    /// Convenience wrapper for [`EncrypterHalf::encrypt_client_header`].
    #[must_use]
    pub fn encrypt_client_header(
        &mut self,
        size: u16,
        opcode: u32,
    ) -> [u8; CLIENT_HEADER_LENGTH as usize] {
        self.encrypt.encrypt_client_header(size, opcode)
    }

    /// Use either [the client](Self::read_and_decrypt_client_header)
    /// or [the server](Self::read_and_decrypt_server_header)
    /// [`Read`] functions, or
    /// [the client](Self::decrypt_client_header)
    /// or [the server](Self::decrypt_server_header) array functions.
    pub fn decrypt(&mut self, data: &mut [u8]) {
        self.decrypt.decrypt(data);
    }

    /// Convenience wrapper for [`DecrypterHalf::read_and_decrypt_server_header`].
    ///
    /// # Errors
    ///
    /// Has the same errors as [`DecrypterHalf::read_and_decrypt_server_header`].
    pub fn read_and_decrypt_server_header<R: Read>(
        &mut self,
        reader: R,
    ) -> std::io::Result<ServerHeader> {
        self.decrypt.read_and_decrypt_server_header(reader)
    }

    /// Convenience wrapper for [`DecrypterHalf::read_and_decrypt_client_header`].
    ///
    /// # Errors
    ///
    /// Has the same errors as [`DecrypterHalf::read_and_decrypt_client_header`].
    pub fn read_and_decrypt_client_header<R: Read>(
        &mut self,
        reader: R,
    ) -> std::io::Result<ClientHeader> {
        self.decrypt.read_and_decrypt_client_header(reader)
    }

    /// Convenience wrapper for [`DecrypterHalf::decrypt_server_header`].
    ///
    /// Prefer this over directly using [`Self::decrypt`].
    #[must_use]
    pub fn decrypt_server_header(
        &mut self,
        data: [u8; SERVER_HEADER_LENGTH as usize],
    ) -> ServerHeader {
        self.decrypt.decrypt_server_header(data)
    }

    /// Convenience wrapper for [`DecrypterHalf::decrypt_client_header`].
    ///
    /// Prefer this over directly using [`Self::decrypt`].
    #[must_use]
    pub fn decrypt_client_header(
        &mut self,
        mut data: [u8; CLIENT_HEADER_LENGTH as usize],
    ) -> ClientHeader {
        self.decrypt(&mut data);

        let size: u16 = u16::from_be_bytes([data[0], data[1]]);
        let opcode: u32 = u32::from_le_bytes([data[2], data[3], data[4], data[5]]);

        ClientHeader { size, opcode }
    }

    /// Split the [`HeaderCrypto`] into two parts for use with split connections.
    ///
    /// It is intended for the [`EncrypterHalf`] to be stored with the write half of
    /// the connection and for the [`DecrypterHalf`] to be stored with the read half
    /// of the connection.
    ///
    /// This is not necessary to do unless you actually can split your connections into
    /// read and write halves, and you have some reason for not just keeping the crypto together
    /// like if you don't want locking encryption to also lock decryption in a mutex.
    #[allow(clippy::missing_const_for_fn)] // Clippy does not consider `self` arg
    #[must_use]
    pub fn split(self) -> (EncrypterHalf, DecrypterHalf) {
        (self.encrypt, self.decrypt)
    }

    pub(crate) const fn new(session_key: [u8; SESSION_KEY_LENGTH as usize]) -> Self {
        Self {
            decrypt: DecrypterHalf::new(session_key),
            encrypt: EncrypterHalf::new(session_key),
        }
    }
}

/// Random Seed part of the calculation needed to verify
/// that a client knows the session key.
///
/// The [`ProofSeed::into_server_header_crypto`] function is used by the server to verify
/// that a client knows the session key.
///
/// The [`ProofSeed::into_client_header_crypto`] function is used by the client to
/// prove to the server that the client knows the session key.
#[derive(Debug, Clone, Copy, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct ProofSeed {
    seed: u32,
}

impl ProofSeed {
    /// Creates a new, random, seed.
    pub fn new() -> Self {
        Self::default()
    }

    #[cfg(test)]
    const fn from_specific_seed(server_seed: u32) -> Self {
        Self { seed: server_seed }
    }

    /// Either the server seed used in [`SMSG_AUTH_CHALLENGE`] or the client
    /// seed used in [`CMSG_AUTH_SESSION`].
    ///
    /// [`SMSG_AUTH_CHALLENGE`]: https://wowdev.wiki/SMSG_AUTH_CHALLENGE
    /// [`CMSG_AUTH_SESSION`]: https://wowdev.wiki/CMSG_AUTH_SESSION
    #[must_use]
    pub const fn seed(&self) -> u32 {
        self.seed
    }

    /// Generates world server proof and [`HeaderCrypto`].
    ///
    /// This is not valid until the server has responded with a successful [`SMSG_AUTH_RESPONSE`].
    ///
    /// [`SMSG_AUTH_RESPONSE`]: https://wowdev.wiki/SMSG_AUTH_RESPONSE
    #[must_use]
    pub fn into_client_header_crypto(
        self,
        username: &NormalizedString,
        session_key: [u8; SESSION_KEY_LENGTH as _],
        server_seed: u32,
    ) -> ([u8; PROOF_LENGTH as _], HeaderCrypto) {
        let client_proof = calculate_world_server_proof(
            username,
            &SessionKey::from_le_bytes(session_key),
            server_seed,
            self.seed,
        );

        let crypto = HeaderCrypto::new(session_key);

        (*client_proof.as_le_bytes(), crypto)
    }

    /// Asserts that the client knows the session key.
    ///
    /// # Errors
    ///
    /// If the `client_proof` does not match the server generated proof.
    /// This should only happen if:
    ///
    /// * There's an error with the provided parameters.
    /// * The session key might be out of date.
    /// * The client is not well behaved and deliberately trying to get past the login server.
    ///
    pub fn into_server_header_crypto(
        self,
        username: &NormalizedString,
        session_key: [u8; SESSION_KEY_LENGTH as _],
        client_proof: [u8; PROOF_LENGTH as _],
        client_seed: u32,
    ) -> Result<HeaderCrypto, MatchProofsError> {
        let server_proof = calculate_world_server_proof(
            username,
            &SessionKey::from_le_bytes(session_key),
            self.seed,
            client_seed,
        );

        if server_proof != Proof::from_le_bytes(client_proof) {
            return Err(MatchProofsError {
                client_proof,
                server_proof: *server_proof.as_le_bytes(),
            });
        }

        Ok(HeaderCrypto::new(session_key))
    }
}

impl Default for ProofSeed {
    fn default() -> Self {
        Self {
            seed: thread_rng().next_u32(),
        }
    }
}

#[cfg(test)]
mod test {

    use crate::hex::*;
    use crate::key::SessionKey;
    use crate::normalized_string::NormalizedString;
    use crate::vanilla_header::{HeaderCrypto, ProofSeed};
    use core::convert::TryInto;

    const SESSION_KEY_REGRESSION: [u8; 40] = [
        0x2E, 0xFE, 0xE7, 0xB0, 0xC1, 0x77, 0xEB, 0xBD, 0xFF, 0x66, 0x76, 0xC5, 0x6E, 0xFC, 0x23,
        0x39, 0xBE, 0x9C, 0xAD, 0x14, 0xBF, 0x8B, 0x54, 0xBB, 0x5A, 0x86, 0xFB, 0xF8, 0x1F, 0x6D,
        0x42, 0x4A, 0xA2, 0x3C, 0xC9, 0xA3, 0x14, 0x9F, 0xB1, 0x75,
    ];
    const CLIENT_PROOF: [u8; 20] = [
        171, 16, 181, 52, 139, 193, 19, 213, 173, 100, 0, 37, 65, 184, 70, 148, 36, 169, 17, 228,
    ];

    #[test]
    fn verify_seed_proof() {
        const FILE: &str = include_str!("../../tests/encryption/calculate_world_server_proof.txt");
        for line in FILE.lines() {
            let mut line = line.split_whitespace();

            let username = line.next().unwrap();
            let session_key = SessionKey::from_be_hex_str(line.next().unwrap());
            let server_seed: u32 = line.next().unwrap().parse().unwrap();
            let client_seed = ProofSeed::from_specific_seed(line.next().unwrap().parse().unwrap());
            let expected: [u8; 20] = hex_decode_be(line.next().unwrap()).try_into().unwrap();

            let (proof, _) = client_seed.into_client_header_crypto(
                &username.try_into().unwrap(),
                *session_key.as_le_bytes(),
                server_seed,
            );

            assert_eq!(expected, proof);
        }
    }

    #[test]
    fn verify_client_and_server_agree() {
        let session_key = [
            239, 107, 150, 237, 174, 220, 162, 4, 138, 56, 166, 166, 138, 152, 188, 146, 96, 151,
            1, 201, 202, 137, 231, 87, 203, 23, 62, 17, 7, 169, 178, 1, 51, 208, 202, 223, 26, 216,
            250, 9,
        ];

        let username = NormalizedString::new("A").unwrap();

        let client_seed = ProofSeed::new();
        let client_seed_value = client_seed.seed();
        let server_seed = ProofSeed::new();

        let (client_proof, mut client_crypto) =
            client_seed.into_client_header_crypto(&username, session_key, server_seed.seed());

        let mut server_crypto = server_seed
            .into_server_header_crypto(&username, session_key, client_proof, client_seed_value)
            .unwrap();

        let original_data = hex_decode_be("ca81d335f3fbf3b2e9d79bab2e0f23fd32df279bdb67e10baabe2d1ccf53b658fe68fe95ddf4b9a89edfe45b4fef96e19a3d");
        let mut data = original_data.clone();

        client_crypto.encrypt(&mut data);
        server_crypto.decrypt(&mut data);

        assert_eq!(original_data, data);

        server_crypto.encrypt(&mut data);
        client_crypto.decrypt(&mut data);

        assert_eq!(original_data, data);
    }

    #[test]
    fn verify_server_header() {
        // Real capture with 1.12 client

        let session_key = [
            239, 107, 150, 237, 174, 220, 162, 4, 138, 56, 166, 166, 138, 152, 188, 146, 96, 151,
            1, 201, 202, 137, 231, 87, 203, 23, 62, 17, 7, 169, 178, 1, 51, 208, 202, 223, 26, 216,
            250, 9,
        ];
        let client_seed = 12589856;
        let client_proof = [
            26, 97, 90, 187, 176, 134, 53, 49, 75, 160, 129, 47, 67, 207, 231, 42, 234, 184, 227,
            124,
        ];
        let mut encryption = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof,
                client_seed,
            )
            .unwrap();
        let mut write_encryption = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof,
                client_seed,
            )
            .unwrap();

        let mut buf = [0_u8; 4];

        let header = encryption.encrypt_server_header(12, 494);
        let expected_header = [239, 86, 206, 186];
        assert_eq!(header, expected_header);
        write_encryption
            .write_encrypted_server_header(&mut &mut buf[..], 12, 494)
            .unwrap();
        assert_eq!(header, buf);

        let header = encryption.encrypt_server_header(170, 59);
        let expected_header = [104, 222, 119, 123];
        assert_eq!(header, expected_header);
        write_encryption
            .write_encrypted_server_header(&mut &mut buf[..], 170, 59)
            .unwrap();
        assert_eq!(header, buf);

        let header = encryption.encrypt_server_header(6, 477);
        let expected_header = [5, 67, 190, 101];
        assert_eq!(header, expected_header);
        write_encryption
            .write_encrypted_server_header(&mut &mut buf[..], 6, 477)
            .unwrap();
        assert_eq!(header, buf);

        let header = encryption.encrypt_server_header(6, 477);
        let expected_header = [239, 141, 238, 129];
        assert_eq!(header, expected_header);
        write_encryption
            .write_encrypted_server_header(&mut &mut buf[..], 6, 477)
            .unwrap();
        assert_eq!(header, buf);
    }

    #[test]
    fn verify_client_header() {
        // Real capture with 1.12 client

        let session_key = [
            9, 83, 75, 103, 5, 182, 16, 162, 170, 134, 230, 117, 11, 100, 136, 74, 88, 145, 175,
            126, 216, 48, 38, 40, 234, 116, 174, 149, 133, 20, 193, 51, 103, 223, 194, 141, 4, 191,
            161, 96,
        ];
        let client_proof = [
            102, 178, 126, 54, 19, 161, 151, 190, 103, 77, 100, 97, 155, 55, 161, 248, 99, 146,
            229, 128,
        ];
        let client_seed = 12589856;
        let mut encryption = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof,
                client_seed,
            )
            .unwrap();
        let mut read_encryption = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof,
                client_seed,
            )
            .unwrap();

        let header = [9, 96, 220, 67, 72, 254];
        let c = encryption.decrypt_client_header(header);
        let expected_size = 4;
        let expected_opcode = 55; // CMSG_CHAR_ENUM
        assert_eq!(c.opcode, expected_opcode);
        assert_eq!(c.size, expected_size);
        let read_c = read_encryption
            .read_and_decrypt_client_header(&mut &header[..])
            .unwrap();
        assert_eq!(read_c, c);

        let expected_size = 12;
        let expected_opcode = 476; // CMSG_PING

        // Must be run through in order because the session key index is changed
        let headers = [
            [14, 188, 50, 185, 159, 20],
            [31, 135, 219, 38, 126, 15],
            [190, 48, 52, 101, 139, 179],
        ];
        for header in headers.iter() {
            let c = encryption.decrypt_client_header(*header);
            let read_c = read_encryption
                .read_and_decrypt_client_header(&mut &header[..])
                .unwrap();
            assert_eq!(c.opcode, expected_opcode);
            assert_eq!(c.size, expected_size);
            assert_eq!(c, read_c);
        }
    }

    #[test]
    fn verify_login() {
        let session_key = [
            115, 0, 100, 222, 18, 15, 156, 194, 27, 1, 216, 229, 165, 207, 78, 233, 183, 241, 248,
            73, 190, 142, 14, 89, 44, 235, 153, 190, 103, 206, 34, 88, 45, 199, 104, 175, 79, 108,
            93, 48,
        ];
        let username = NormalizedString::new("A").unwrap();
        let server_seed = 0xDEADBEEF;
        let client_seed = 1266519981;
        let client_proof = [
            202, 54, 102, 180, 90, 87, 9, 107, 217, 97, 235, 56, 221, 203, 108, 19, 109, 141, 137,
            7,
        ];

        let seed = ProofSeed::from_specific_seed(server_seed);
        let encryption =
            seed.into_server_header_crypto(&username, session_key, client_proof, client_seed);
        assert!(encryption.is_ok());
    }

    #[test]
    fn verify_encrypt() {
        let contents = include_str!("../../tests/encryption/calculate_encrypt_values.txt");

        for line in contents.lines() {
            let mut line = line.split_whitespace();
            let session_key = SessionKey::from_be_hex_str(line.next().unwrap());
            let mut data = hex_decode_be(line.next().unwrap());
            let mut split_data = data.clone();
            let original_data = data.clone();
            let expected = hex_decode_be(line.next().unwrap());

            // Bypass checking seeds and proofs because they aren't there
            let mut encryption = HeaderCrypto::new(*session_key.as_le_bytes());

            encryption.encrypt(&mut data);

            assert_eq!(
                hex_encode_be(&expected),
                hex_encode_be(&data),
                "Session Key: {},
                 data: {},
                 Got data: {}",
                hex_encode_be(session_key.as_le_bytes()),
                hex_encode_be(&original_data),
                hex_encode_be(&data)
            );

            // Bypass checking seeds and proofs because they aren't there
            let full = HeaderCrypto::new(*session_key.as_le_bytes());
            let (mut enc, _dec) = full.split();

            enc.encrypt(&mut split_data);

            assert_eq!(
                hex_encode_be(&expected),
                hex_encode_be(&split_data),
                "Session Key: {},
                 data: {},
                 Got data: {}",
                hex_encode_be(session_key.as_le_bytes()),
                hex_encode_be(&original_data),
                hex_encode_be(&split_data)
            );
        }
    }

    #[test]
    fn verify_mixed_used() {
        // Verify that mixed use does not interfere with each other
        let original_data = hex_decode_be("ca81d335f3fbf3b2e9d79bab2e0f23fd32df279bdb67e10baabe2d1ccf53b658fe68fe95ddf4b9a89edfe45b4fef96e19a3d");
        let mut encrypt_data = original_data.clone();
        let mut decrypt_data = original_data.clone();

        let mut encryption = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                SESSION_KEY_REGRESSION,
                CLIENT_PROOF,
                0,
            )
            .unwrap();
        const STEP: usize = 10;
        for (i, _d) in original_data.iter().enumerate().step_by(STEP) {
            // Ensure that encrypting, then decrypting doesn't change how encryption works
            encryption.encrypt(&mut encrypt_data[i..(i) + STEP]);
            encryption.decrypt(&mut decrypt_data[i..(i) + STEP]);
        }

        let expected_decrypt = hex_decode_be("2f5123a98fc9f12eec1285cc80f885026f1ac68219992e9a6acbaa19f7224af70ad4509b1555d47cd90434e7179805a0a313");
        let expected_encrypt = hex_decode_be("1569eb7d9f1be19e4932394b318065e5b1a326b9e02ab299a97d9903bb778be9f492bcf53f1e84082a3212b909d1a37d7713");
        assert_eq!(
            expected_decrypt, decrypt_data,
            "Original data: {:?}, expected: {:?}, got: {:?}",
            original_data, expected_decrypt, decrypt_data
        );
        assert_eq!(
            expected_encrypt, encrypt_data,
            "Original data: {:?}, expected: {:?}, got: {:?}",
            original_data, expected_encrypt, encrypt_data
        );
    }
    #[test]
    fn verify_inner_access() {
        let original_data = hex_decode_be("ca81d335f3fbf3b2e9d79bab2e0f23fd32df279bdb67e10baabe2d1ccf53b658fe68fe95ddf4b9a89edfe45b4fef96e19a3d");
        let mut encrypt_data = original_data.clone();
        let mut decrypt_data = original_data.clone();

        let mut encryption = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                SESSION_KEY_REGRESSION,
                CLIENT_PROOF,
                0,
            )
            .unwrap();

        let enc = encryption.encrypter();
        enc.encrypt(&mut encrypt_data);
        let dec = encryption.decrypter();
        dec.decrypt(&mut decrypt_data);

        let expected_decrypt = hex_decode_be("2f5123a98fc9f12eec1285cc80f885026f1ac68219992e9a6acbaa19f7224af70ad4509b1555d47cd90434e7179805a0a313");
        let expected_encrypt = hex_decode_be("1569eb7d9f1be19e4932394b318065e5b1a326b9e02ab299a97d9903bb778be9f492bcf53f1e84082a3212b909d1a37d7713");
        assert_eq!(
            expected_decrypt, decrypt_data,
            "Original data: {:?}, expected: {:?}, got: {:?}",
            original_data, expected_decrypt, decrypt_data
        );
        assert_eq!(
            expected_encrypt, encrypt_data,
            "Original data: {:?}, expected: {:?}, got: {:?}",
            original_data, expected_encrypt, encrypt_data
        );
    }

    #[test]
    fn verify_splitting() {
        // Verify that splitting and combining again works

        let original_data = hex_decode_be("ca81d335f3fbf3b2e9d79bab2e0f23fd32df279bdb67e10baabe2d1ccf53b658fe68fe95ddf4b9a89edfe45b4fef96e19a3d");
        let mut encrypt_data = original_data.clone();
        let mut decrypt_data = original_data.clone();

        let mut encryption = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                SESSION_KEY_REGRESSION,
                CLIENT_PROOF,
                0,
            )
            .unwrap();

        const STEP: usize = 20;
        encryption.encrypt(&mut encrypt_data[0..STEP]);
        encryption.decrypt(&mut decrypt_data[0..STEP]);

        let (mut e, mut d) = encryption.split();
        e.encrypt(&mut encrypt_data[STEP..STEP * 2]);
        d.decrypt(&mut decrypt_data[STEP..STEP * 2]);

        let mut encryption = e.unsplit(d).unwrap();

        encryption.encrypt(&mut encrypt_data[STEP * 2..]);
        encryption.decrypt(&mut decrypt_data[STEP * 2..]);

        let expected_decrypt = hex_decode_be("2f5123a98fc9f12eec1285cc80f885026f1ac68219992e9a6acbaa19f7224af70ad4509b1555d47cd90434e7179805a0a313");
        let expected_encrypt = hex_decode_be("1569eb7d9f1be19e4932394b318065e5b1a326b9e02ab299a97d9903bb778be9f492bcf53f1e84082a3212b909d1a37d7713");
        assert_eq!(
            expected_decrypt, decrypt_data,
            "Original data: {:?}, expected: {:?}, got: {:?}",
            original_data, expected_decrypt, decrypt_data
        );
        assert_eq!(
            expected_encrypt, encrypt_data,
            "Original data: {:?}, expected: {:?}, got: {:?}",
            original_data, expected_encrypt, encrypt_data
        );
    }

    #[test]
    fn verify_unsplit_errors() {
        let session_key1 = SESSION_KEY_REGRESSION;
        let client_proof1 = CLIENT_PROOF;

        let session_key2 = [
            115, 0, 100, 222, 18, 15, 156, 194, 27, 1, 216, 229, 165, 207, 78, 233, 183, 241, 248,
            73, 190, 142, 14, 89, 44, 235, 153, 190, 103, 206, 34, 88, 45, 199, 104, 175, 79, 108,
            93, 48,
        ];
        let client_proof2 = [
            202, 54, 102, 180, 90, 87, 9, 107, 217, 97, 235, 56, 221, 203, 108, 19, 109, 141, 137,
            7,
        ];

        let (encryptions1, _) = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key1,
                client_proof1,
                0,
            )
            .unwrap()
            .split();
        let (_, decryptions2) = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key2,
                client_proof2,
                1266519981,
            )
            .unwrap()
            .split();

        assert!(!decryptions2.is_pair_of(&encryptions1));

        assert!(encryptions1.unsplit(decryptions2).is_err());
    }

    #[test]
    fn verify_errors() {
        let mut client_proof = [
            171, 16, 181, 52, 139, 193, 19, 213, 173, 100, 0, 37, 65, 184, 70, 148, 36, 169, 17,
            228,
        ];

        let mut session_key = SESSION_KEY_REGRESSION;

        assert!(ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof,
                1, // Should be 0
            )
            .is_err());

        client_proof[0] += 1;

        assert!(ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof, // [0] should be -1
                0,
            )
            .is_err());

        client_proof[0] -= 1;

        assert!(ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof,
                0,
            )
            .is_ok());

        session_key[0] += 1;

        assert!(ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key, // [0] should be -1
                client_proof,
                0,
            )
            .is_err());

        session_key[0] -= 1;

        assert!(ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof,
                0,
            )
            .is_ok());

        assert!(ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("B").unwrap(), // should be A
                session_key,
                client_proof,
                0,
            )
            .is_err());
    }

    #[test]
    fn server_helper() {
        let contents = include_str!("../../tests/encryption/vanilla_server.txt");

        for line in contents.lines() {
            let mut line = line.split_whitespace();

            let mut data = hex_decode_be(line.next().unwrap());
            let original_data = data.clone();
            let size = line.next().unwrap().parse().unwrap();
            let opcode = line.next().unwrap().parse().unwrap();

            let (client_proof, mut dec) = ProofSeed::from_specific_seed(0xDEADBEEF)
                .into_client_header_crypto(
                    &NormalizedString::new("A").unwrap(),
                    SESSION_KEY_REGRESSION,
                    0xDEADBEEF,
                );

            let mut enc = ProofSeed::from_specific_seed(0xDEADBEEF)
                .into_server_header_crypto(
                    &NormalizedString::new("A").unwrap(),
                    SESSION_KEY_REGRESSION,
                    client_proof,
                    0xDEADBEEF,
                )
                .unwrap();

            let header = dec
                .read_and_decrypt_server_header(&mut data.as_slice())
                .unwrap();

            assert_eq!(header.size, size);
            assert_eq!(header.opcode, opcode);

            enc.write_encrypted_server_header(data.as_mut_slice(), size, opcode)
                .unwrap();

            assert_eq!(data, original_data);
        }
    }

    #[test]
    fn client_helper() {
        let contents = include_str!("../../tests/encryption/vanilla_client.txt");

        for line in contents.lines() {
            let mut line = line.split_whitespace();
            let mut data = hex_decode_be(line.next().unwrap());
            let original_data = data.clone();
            let size = line.next().unwrap().parse().unwrap();
            let opcode = line.next().unwrap().parse().unwrap();

            let (client_proof, mut dec) = ProofSeed::from_specific_seed(0xDEADBEEF)
                .into_client_header_crypto(
                    &NormalizedString::new("A").unwrap(),
                    SESSION_KEY_REGRESSION,
                    0xDEADBEEF,
                );

            let mut enc = ProofSeed::from_specific_seed(0xDEADBEEF)
                .into_server_header_crypto(
                    &NormalizedString::new("A").unwrap(),
                    SESSION_KEY_REGRESSION,
                    client_proof,
                    0xDEADBEEF,
                )
                .unwrap();

            let header = dec.read_and_decrypt_client_header(data.as_slice()).unwrap();

            enc.write_encrypted_client_header(data.as_mut_slice(), size, opcode)
                .unwrap();

            assert_eq!(header.size, size);
            assert_eq!(header.opcode, opcode);
            assert_eq!(data, original_data);
        }
    }

    #[test]
    fn verify_trait_helpers() {
        // Verify that the trait helpers do the same thing as manually encrypting/decrypting
        let session_key = [
            0x2E, 0xFE, 0xE7, 0xB0, 0xC1, 0x77, 0xEB, 0xBD, 0xFF, 0x66, 0x76, 0xC5, 0x6E, 0xFC,
            0x23, 0x39, 0xBE, 0x9C, 0xAD, 0x14, 0xBF, 0x8B, 0x54, 0xBB, 0x5A, 0x86, 0xFB, 0xF8,
            0x1F, 0x6D, 0x42, 0x4A, 0xA2, 0x3C, 0xC9, 0xA3, 0x14, 0x9F, 0xB1, 0x75,
        ];

        let original_data = [
            0x3d, 0x9a, 0xe1, 0x96, 0xef, 0x4f, 0x3d, 0x9a, 0xe1, 0x96, 0x3d, 0x9a, 0xe1, 0x96,
            0xef, 0x4f, 0x3d, 0x9a, 0xe1, 0x96,
        ];
        let mut encrypt_data = original_data;
        let mut encrypted_data = Vec::new();
        let mut decrypt_data = original_data;
        let decrypted_data = original_data.clone().to_vec();

        let client_proof = [
            171, 16, 181, 52, 139, 193, 19, 213, 173, 100, 0, 37, 65, 184, 70, 148, 36, 169, 17,
            228,
        ];

        let mut encryption = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof,
                0,
            )
            .unwrap();
        let mut helper_encryption = ProofSeed::from_specific_seed(0xDEADBEEF)
            .into_server_header_crypto(
                &NormalizedString::new("A").unwrap(),
                session_key,
                client_proof,
                0,
            )
            .unwrap();

        encryption
            .write_encrypted_client_header(&mut encrypted_data, 0x3d9a, 0x4fef96e1)
            .unwrap();
        encryption
            .write_encrypted_server_header(&mut encrypted_data, 0x3d9a, 0x96e1)
            .unwrap();
        encrypted_data.append(
            &mut encryption
                .encrypt_client_header(0x3d9a, 0x4fef96e1)
                .to_vec(),
        );
        encrypted_data.append(&mut encryption.encrypt_server_header(0x3d9a, 0x96e1).to_vec());
        helper_encryption.encrypt(&mut encrypt_data);
        assert_eq!(encrypted_data, encrypt_data);

        encryption.decrypt(&mut decrypt_data);
        let c = helper_encryption
            .read_and_decrypt_client_header(&mut decrypted_data[0..6].to_vec().as_slice())
            .unwrap();
        assert_eq!(
            c.size,
            u16::from_be_bytes(decrypt_data[0..2].try_into().unwrap())
        );
        assert_eq!(
            c.opcode,
            u32::from_le_bytes(decrypt_data[2..6].try_into().unwrap())
        );
        let s = helper_encryption
            .read_and_decrypt_server_header(&mut decrypted_data[6..10].to_vec().as_slice())
            .unwrap();
        assert_eq!(
            s.size,
            u16::from_be_bytes(decrypt_data[6..8].try_into().unwrap())
        );
        assert_eq!(
            s.opcode,
            u16::from_le_bytes(decrypt_data[8..10].try_into().unwrap())
        );
        let c = helper_encryption.decrypt_client_header(decrypted_data[10..16].try_into().unwrap());
        assert_eq!(
            c.size,
            u16::from_be_bytes(decrypt_data[10..12].try_into().unwrap())
        );
        assert_eq!(
            c.opcode,
            u32::from_le_bytes(decrypt_data[12..16].try_into().unwrap())
        );
        let s = helper_encryption.decrypt_server_header(decrypted_data[16..20].try_into().unwrap());
        assert_eq!(
            s.size,
            u16::from_be_bytes(decrypt_data[16..18].try_into().unwrap())
        );
        assert_eq!(
            s.opcode,
            u16::from_le_bytes(decrypt_data[18..20].try_into().unwrap())
        );
    }

    #[test]
    fn verify_decrypt() {
        let contents = include_str!("../../tests/encryption/calculate_decrypt_values.txt");

        for line in contents.lines() {
            let mut line = line.split_whitespace();
            let session_key = SessionKey::from_be_hex_str(line.next().unwrap());
            let mut data = hex_decode_be(line.next().unwrap());
            let mut split_data = data.clone();
            let original_data = data.clone();
            let expected = hex_decode_be(line.next().unwrap());

            let mut encryption = HeaderCrypto::new(*session_key.as_le_bytes());

            encryption.decrypt(&mut data);

            assert_eq!(
                hex_encode_be(&expected),
                hex_encode_be(&data),
                "Session Key: {},
                 data: {},
                 Got data: {}",
                hex_encode_be(session_key.as_le_bytes()),
                hex_encode_be(&original_data),
                hex_encode_be(&data)
            );

            let full = HeaderCrypto::new(*session_key.as_le_bytes());
            let (_enc, mut dec) = full.split();

            dec.decrypt(&mut split_data);

            assert_eq!(
                hex_encode_be(&expected),
                hex_encode_be(&split_data),
                "Session Key: {},
                 data: {},
                 Got data: {}",
                hex_encode_be(session_key.as_le_bytes()),
                hex_encode_be(&original_data),
                hex_encode_be(&split_data),
            );
        }
    }
}
