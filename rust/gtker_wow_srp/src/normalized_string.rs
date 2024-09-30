//! Functionality for keeping strings in a format the client expects.
//!
//! # Background
//!
//! The client uppercases both the username and password before hashing them. The username sent to
//! the server is also an uppercased version. This means that to the client, there's no difference
//! between logging in as `alice`, `ALICE`, or anything in between. This is no problem for ASCII
//! characters as they have well defined upper- and lowercase letters.
//!
//! The client is able to enter up to 16 characters in the client, which will be sent over the network
//! as one or more UTF-8 bytes each.
//!
//! Unicode characters, however, act differently and without any real pattern.
//!
//! The letter `ń`, Unicode code point `U+0144`, name `LATIN SMALL LETTER N WITH ACUTE` for example,
//! appears as a capital `N` in the client, and sends the byte `0x4E` which is ASCII N. This is
//! despite the letter `Ń`, Unicode code point `U+0144`, name `LATIN CAPITAL LETTER N WITH ACUTE`
//! existing.
//!
//! The letter `ž`, Unicode code point `U+017E`, name `LATIN SMALL LETTER Z WITH CARON` appears as
//! the literal letter `ž` and gets sent over the network as the bytes `0xC5 0xBE` which is UTF-8
//! for that same letter.
//!
//! The letter `Ž`, Unicode code point `U+017D`, name `LATIN CAPITAL LETTER Z WITH CARON` appears as
//! the literal letter `Ž` in the client and gets sent over the network as the bytes `0xC5 0xBD`
//! which is UTF-8 for that same letter.
//!
//! The letter `ƒ`, Unicode code point `U+0192`, name `LATIN SMALL LETTER F WITH HOOK`, appears as
//! the literal letter `ƒ` and gets sent over the network as the bytes `0xC6 0x92` which is UTF-8
//! for that same letter.
//!
//! The letter `Ƒ`, Unicode code point `U+0191`, name `LATIN CAPITAL LETTER F WITH HOOK` appears as
//! the lower case version in the client and gets sent over the network as the lowercase version.
//!
//! None of the Cyrillic letters show in the client and get transmitted as a question mark (byte `0x3F`).
//!
//! These wildly varying rules for transforming the username and password means that the only way to
//! really be sure how a specific character is represented on the client and gets sent over the network
//! is to test every single unicode character. The behavior is also not guaranteed to be the same across
//! different versions, or even different localizations of the same version.
//!
//! # Problems
//!
//! The user will need to register their account outside of the client. They might name their account
//! `Ƒast` and get through registration because the web service does not know that the letter `Ƒ` can not
//! be represented in the client and is instead shown and sent as `ƒ`. The user is unable to log in, instead
//! getting an "Account does not exist" message.
//!
//! Another user creates an account named `ńacho` and gets through registration. Since the letter `ń` is
//! represented as the letter `N` in the client, the sign up service
//! makes this transformation in order to stay in sync with the client.
//! This might allow the user to log into the account named `Nacho`, depending on which
//! verifier/salt pair is fetched from the database.
//!
//! Authentication relies on the signup service, server and client to have the exact
//! same behavior, otherwise vulnerabilities will appear or users might be unable to log in.
//!
//! # Solution
//!
//! The only manageable solution is to stick to only the ASCII character set and reject all other
//! characters.
//! This greatly reduces the complexity of every link in the chain and decreases possible vulnerabilities.
//!
//! This also provides the benefit of knowing exactly how large an account name can be.
//!

use crate::error::NormalizedStringError;
use core::convert::TryFrom;
use core::fmt;
use core::fmt::{Display, Formatter};

/// Represents usernames and passwords containing only allowed characters.
///
/// See module [`normalized_string`](`crate::normalized_string`) for more information.
///
/// The [`NormalizedString::new`] method will always take the format that requires the
/// least amount of translation to the underlying string format.
/// **This method can change interface completely without it being a breaking change.**
/// This is done deliberately in order to alert users of changes that could impact performance.
///
/// Iif you want stability Use the [`NormalizedString::from_string`] or [`NormalizedString::from_str`].
///
/// ```
/// # use core::convert::TryInto;
/// use wow_srp::error::NormalizedStringError;
/// use wow_srp::normalized_string::NormalizedString;
///
/// # fn main() -> Result<(), NormalizedStringError> {
/// // From &str
/// let username = "Alice";
/// let username: NormalizedString = username.try_into()?;
///
/// // From string
/// let password = String::from("hunter2");
/// let password: NormalizedString = password.try_into()?;
/// # Ok(())
/// # }
/// ```
///
#[derive(Debug, PartialEq, Eq, Hash, Ord, PartialOrd, Clone)]
pub struct NormalizedString {
    s: [u8; MAXIMUM_STRING_LENGTH_IN_BYTES as usize],
    length: u8,
}

/// The highest amount of letters that the client will allow in both the username
/// and password fields.
/// Always 16.
pub const MAXIMUM_STRING_LENGTH_IN_BYTES: u8 = 16;

impl NormalizedString {
    /// Checks for non-ASCII characters and too large of a string
    /// and correctly uppercases letters as needed.
    ///
    /// Allowed characters are all ASCII characters except for ASCII control characters.
    ///
    /// # Errors
    ///
    /// See the [module level docs](crate::normalized_string) for explanation of allowed values.
    pub fn new(s: impl AsRef<str>) -> Result<Self, NormalizedStringError> {
        fn inner(s: &str) -> Result<NormalizedString, NormalizedStringError> {
            if s.len() > MAXIMUM_STRING_LENGTH_IN_BYTES as usize || s.is_empty() {
                return Err(NormalizedStringError::StringTooLong);
            }

            let mut array = [0_u8; MAXIMUM_STRING_LENGTH_IN_BYTES as usize];

            for (i, c) in s.chars().enumerate() {
                if !c.is_ascii() || c.is_ascii_control() {
                    return Err(NormalizedStringError::CharacterNotAllowed(c));
                }

                array[i] = c.to_ascii_uppercase() as u8;
            }

            Ok(NormalizedString {
                s: array,
                length: s.len() as u8,
            })
        }

        // Compile time optimization
        // https://matklad.github.io/2021/09/04/fast-rust-builds.html#Keeping-Instantiations-In-Check
        inner(s.as_ref())
    }

    /// Checks for non-ASCII characters and too large of a string
    /// and correctly uppercases letters as needed.
    ///
    /// Allowed characters are all ASCII characters except for ASCII control characters.
    ///
    /// # Errors
    ///
    /// See the [module level docs](crate::normalized_string) for explanation of allowed values.
    #[allow(clippy::should_implement_trait)]
    pub fn from_str(s: impl AsRef<str>) -> Result<Self, NormalizedStringError> {
        Self::new(s)
    }

    /// Checks for non-ASCII characters and too large of a string
    /// and correctly uppercases letters as needed.
    ///
    /// Allowed characters are all ASCII characters except for ASCII control characters.
    ///
    /// # Errors
    ///
    /// See the [module level docs](crate::normalized_string) for explanation of allowed values.
    pub fn from_string(s: impl Into<String>) -> Result<Self, NormalizedStringError> {
        Self::new(s.into())
    }
}

impl TryFrom<&str> for NormalizedString {
    type Error = NormalizedStringError;

    fn try_from(s: &str) -> Result<Self, Self::Error> {
        Self::new(s)
    }
}

impl TryFrom<String> for NormalizedString {
    type Error = NormalizedStringError;

    fn try_from(s: String) -> Result<Self, Self::Error> {
        Self::new(s)
    }
}

impl Display for NormalizedString {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        f.write_str(self.as_ref())
    }
}

impl AsRef<str> for NormalizedString {
    fn as_ref(&self) -> &str {
        // This is checked in the constructor
        // So we can unwrap
        core::str::from_utf8(&self.s[..self.length as usize]).unwrap()
    }
}

#[cfg(test)]
mod test {
    use crate::error::NormalizedStringError;
    use crate::normalized_string::{NormalizedString, MAXIMUM_STRING_LENGTH_IN_BYTES};
    use core::convert::TryInto;

    #[test]
    fn allows_all_ascii_chars() {
        let allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_-,.<>/?[]{}'|=+~` ";

        for c in allowed.chars() {
            let s = NormalizedString::new(c.to_string()).unwrap();
            assert_eq!(s.as_ref(), c.to_ascii_uppercase().to_string());
        }
    }

    #[test]
    // Acts as a sentinel to ensure that the ascii check doesn't disappear
    fn only_allows_ascii_chars() {
        // Arbitrarily picked non-allowed ASCII control codes, Latin-1, Cyrillic and Greek letters
        let ascii_control_codes = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f";
        for c in ascii_control_codes.chars() {
            let s = NormalizedString::new(c.to_string());
            assert!(s.is_err(), "Char should be allowed: '{}'", c);
        }

        let not_allowed_chars = "¢£¤¦¥©¨«¹²³¬®±µ¼½¾¿ÇÐØÞßðüĎГДЕЖЗЙΨΩ";
        for c in not_allowed_chars.chars() {
            let s = NormalizedString::new(c.to_string());
            assert!(s.is_err(), "Char should be allowed: '{}'", c);
        }
    }

    #[test]
    fn not_too_long() {
        let acceptable_length_string = String::from("16bytelongstring");
        let acceptable_length_string = NormalizedString::new(acceptable_length_string).unwrap();
        assert_eq!(acceptable_length_string.as_ref(), "16BYTELONGSTRING");

        let too_long_string = String::from("17bytelongstringA");
        let too_long_string = NormalizedString::new(too_long_string);
        match too_long_string {
            Ok(_) => {
                panic!(
                    "Should not allow strings longer than: '{}'",
                    MAXIMUM_STRING_LENGTH_IN_BYTES
                );
            }
            Err(e) => match e {
                NormalizedStringError::CharacterNotAllowed(_) => {
                    panic!("should fail due to string being too long")
                }
                NormalizedStringError::StringTooLong => {}
            },
        }
    }

    #[test]
    fn no_empty_strings() {
        let empty_string = String::new();
        let empty = NormalizedString::new(empty_string);
        assert!(empty.is_err());
    }

    #[test]
    fn constructors() {
        let str_ref = "Alice";
        let alice = NormalizedString::from_str(str_ref).unwrap();
        assert_eq!(alice.as_ref(), "ALICE");

        let string = "Alice".to_string();
        let alice = NormalizedString::from_string(str_ref).unwrap();
        assert_eq!(alice.as_ref(), "ALICE");

        let alice: NormalizedString = string.try_into().unwrap();
        assert_eq!(alice.as_ref(), "ALICE");
    }
}
