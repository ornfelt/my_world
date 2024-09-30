use std::error::Error;
use std::fmt::{Display, Formatter};
use std::string::FromUtf8Error;
use wow_world_base::EnumError;

/// Main error enum. Returned from [`crate::DbcTable::read`].
#[derive(Debug)]
pub enum DbcError {
    /// IO errors.
    Io(std::io::Error),
    /// Errors from invalid enum values.
    InvalidEnum(EnumError),
    /// Errors from converting bytes to strings.
    String(FromUtf8Error),
    /// Errors related to headers.
    InvalidHeader(InvalidHeaderError),
}

impl Display for DbcError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            DbcError::Io(i) => i.fmt(f),
            DbcError::InvalidEnum(i) => i.fmt(f),
            DbcError::String(i) => i.fmt(f),
            DbcError::InvalidHeader(i) => i.fmt(f),
        }
    }
}

impl Error for DbcError {}

impl From<std::io::Error> for DbcError {
    fn from(i: std::io::Error) -> Self {
        Self::Io(i)
    }
}

impl From<FromUtf8Error> for DbcError {
    fn from(e: FromUtf8Error) -> Self {
        Self::String(e)
    }
}

impl From<InvalidHeaderError> for DbcError {
    fn from(e: InvalidHeaderError) -> Self {
        Self::InvalidHeader(e)
    }
}

/// Errors from reading the header of the DBC file.
#[derive(Debug)]
pub enum InvalidHeaderError {
    /// The magic value was not `0x43424457`, but was instead [`InvalidHeaderError::MagicValue::actual`].
    MagicValue {
        /// Value gotten instead of magic header.
        actual: u32,
    },
    /// The reported `record_size` did not match the precomputed.
    RecordSize {
        /// Expected value.
        expected: u32,
        /// Actual value read.
        actual: u32,
    },
    /// The reported amount of fields did not match the precomputed.
    FieldCount {
        /// Expected value.
        expected: u32,
        /// Actual value read.
        actual: u32,
    },
}

impl Display for InvalidHeaderError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            InvalidHeaderError::RecordSize { expected, actual } => {
                write!(
                    f,
                    "invalid record size. Expected '{}', got '{}'",
                    expected, actual
                )
            }
            InvalidHeaderError::FieldCount { expected, actual } => write!(
                f,
                "invalid field count. Expected '{}', got '{}'",
                expected, actual
            ),
            InvalidHeaderError::MagicValue { actual } => {
                write!(f, "invalid header magic: '{}'", actual)
            }
        }
    }
}

impl Error for InvalidHeaderError {}

impl From<EnumError> for DbcError {
    fn from(i: EnumError) -> Self {
        Self::InvalidEnum(i)
    }
}
