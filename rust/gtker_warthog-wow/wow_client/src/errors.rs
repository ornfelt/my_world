use std::fmt::{Display, Formatter};
use std::io;
use wow_login_messages::errors::{ExpectedOpcodeError, ParseError};
use wow_login_messages::version_8::LoginResult;
use wow_srp::error::{MatchProofsError, NormalizedStringError};
use wow_srp::PUBLIC_KEY_LENGTH;

#[derive(Debug)]
pub enum ClientError {
    Io(io::Error),
    Parse(ParseError),
    InvalidMessageReceived(u32),
    ServerReply(LoginResult),
    NormalizedString(NormalizedStringError),
    InvalidGenerator(Vec<u8>),
    InvalidLargeSafePrime(Vec<u8>),
    InvalidPublicKey([u8; PUBLIC_KEY_LENGTH as usize]),
    InvalidSecurityFlag,
    MatchProofs(MatchProofsError),
    InvalidFieldSet,
}

impl ClientError {
    pub(crate) fn reply(result: LoginResult) -> Self {
        Self::ServerReply(result)
    }
}

impl Display for ClientError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            ClientError::Io(e) => e.fmt(f),
            ClientError::Parse(e) => e.fmt(f),
            ClientError::InvalidMessageReceived(e) => write!(f, "invalid opcode received: '{e}'"),
            ClientError::ServerReply(e) => write!(f, "received failure reply from server: '{}'", e),
            ClientError::NormalizedString(e) => e.fmt(f),
            ClientError::InvalidGenerator(e) => write!(f, "invalid generator: '{e:?}'"),
            ClientError::InvalidLargeSafePrime(e) => write!(f, "invalid large safe prime: '{e:?}'"),
            ClientError::InvalidPublicKey(e) => write!(f, "invalid server public key: '{e:?}'"),
            ClientError::InvalidSecurityFlag => write!(f, "invalid security flag"),
            ClientError::MatchProofs(e) => e.fmt(f),
            ClientError::InvalidFieldSet => write!(f, "invalid field set"),
        }
    }
}

impl std::error::Error for ClientError {}

impl From<io::Error> for ClientError {
    fn from(value: io::Error) -> Self {
        Self::Io(value)
    }
}

impl From<ExpectedOpcodeError> for ClientError {
    fn from(value: ExpectedOpcodeError) -> Self {
        match value {
            ExpectedOpcodeError::Opcode(e) => Self::InvalidMessageReceived(e),
            ExpectedOpcodeError::Parse(e) => Self::Parse(e),
            ExpectedOpcodeError::Io(e) => Self::Io(e),
        }
    }
}

impl From<ParseError> for ClientError {
    fn from(value: ParseError) -> Self {
        Self::Parse(value)
    }
}

impl From<NormalizedStringError> for ClientError {
    fn from(value: NormalizedStringError) -> Self {
        Self::NormalizedString(value)
    }
}

impl From<MatchProofsError> for ClientError {
    fn from(value: MatchProofsError) -> Self {
        Self::MatchProofs(value)
    }
}
