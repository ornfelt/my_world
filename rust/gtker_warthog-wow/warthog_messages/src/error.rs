use std::fmt::{Display, Formatter};
use std::string::FromUtf8Error;

#[derive(Debug)]
pub enum MessageError {
    Io(std::io::Error),
    InvalidOpcode(u8),
    Utf8(FromUtf8Error),
}

impl Display for MessageError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            MessageError::Io(e) => e.fmt(f),
            MessageError::InvalidOpcode(e) => write!(f, "invalid opcode received: {e}"),
            MessageError::Utf8(e) => e.fmt(f),
        }
    }
}

impl std::error::Error for MessageError {}

impl From<std::io::Error> for MessageError {
    fn from(value: std::io::Error) -> Self {
        Self::Io(value)
    }
}

impl From<FromUtf8Error> for MessageError {
    fn from(value: FromUtf8Error) -> Self {
        Self::Utf8(value)
    }
}
