mod errors;

use crate::errors::ClientError;
use std::net::SocketAddr;
use tokio::net::TcpStream;
use wow_login_messages::helper::tokio_expect_server_message_protocol;
use wow_login_messages::CollectiveMessage;
use wow_srp::client::SrpClient;
use wow_srp::normalized_string::NormalizedString;
use wow_srp::{PublicKey, LARGE_SAFE_PRIME_LENGTH};

pub use wow_login_messages::all::{
    CMD_AUTH_LOGON_CHALLENGE_Client, Locale, Os, Platform, ProtocolVersion, Version,
};
pub use wow_login_messages::version_8::Realm;
use wow_srp::pin::PinCode;

pub async fn connect_and_authenticate(
    message: CMD_AUTH_LOGON_CHALLENGE_Client,
    address: SocketAddr,
    password: &str,
    client_pin: Option<PinCode>,
) -> Result<(SrpClient, Vec<Realm>, TcpStream), ClientError> {
    let username = NormalizedString::new(&message.account_name)?;
    let password = NormalizedString::new(&password)?;

    let protocol_version = message.protocol_version;

    let mut stream = TcpStream::connect(address).await?;
    type LResult = LoginResult;
    use wow_login_messages::version_8::*;

    message
        .tokio_write_protocol(&mut stream, protocol_version)
        .await?;

    let s = tokio_expect_server_message_protocol::<CMD_AUTH_LOGON_CHALLENGE_Server, _>(
        &mut stream,
        protocol_version,
    )
    .await?;

    let (client, pin) = match s {
        CMD_AUTH_LOGON_CHALLENGE_Server::Success {
            generator,
            large_safe_prime,
            salt,
            security_flag,
            server_public_key,
            ..
        } => {
            if generator.len() > 1 {
                return Err(ClientError::InvalidGenerator(generator));
            }
            let Some(&generator) = generator.get(0) else {
                return Err(ClientError::InvalidGenerator(generator));
            };

            let Ok(large_safe_prime): Result<[u8; LARGE_SAFE_PRIME_LENGTH as usize], _> =
                large_safe_prime.clone().try_into()
            else {
                return Err(ClientError::InvalidLargeSafePrime(large_safe_prime));
            };

            let Ok(server_public_key) = PublicKey::from_le_bytes(server_public_key) else {
                return Err(ClientError::InvalidPublicKey(server_public_key));
            };

            let pin = match (client_pin, security_flag.get_pin()) {
                (None, Some(_)) => {
                    return Err(ClientError::InvalidSecurityFlag);
                }
                (Some(client_pin), Some(pin)) => {
                    let server_salt = pin.pin_salt;
                    let pin_grid_seed = pin.pin_grid_seed;
                    let client_salt = wow_srp::pin::get_pin_salt();
                    let pin_hash = client_pin.into_hash(pin_grid_seed, &server_salt, &client_salt);

                    Some(CMD_AUTH_LOGON_PROOF_Client_SecurityFlag_Pin {
                        pin_hash,
                        pin_salt: client_salt,
                    })
                }
                (_, _) => None,
            };

            if security_flag.get_matrix_card().is_some()
                || security_flag.get_authenticator().is_some()
            {
                return Err(ClientError::InvalidSecurityFlag);
            }

            (
                wow_srp::client::SrpClientChallenge::new(
                    username,
                    password,
                    generator,
                    large_safe_prime,
                    server_public_key,
                    salt,
                ),
                pin,
            )
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailUnknown0 => {
            return Err(ClientError::reply(LResult::FailUnknown0));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailUnknown1 => {
            return Err(ClientError::reply(LResult::FailUnknown1));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailBanned => {
            return Err(ClientError::reply(LResult::FailBanned));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailUnknownAccount => {
            return Err(ClientError::reply(LResult::FailUnknownAccount));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailIncorrectPassword => {
            return Err(ClientError::reply(LResult::FailIncorrectPassword));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailAlreadyOnline => {
            return Err(ClientError::reply(LResult::FailAlreadyOnline));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailNoTime => {
            return Err(ClientError::reply(LResult::FailNoTime));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailDbBusy => {
            return Err(ClientError::reply(LResult::FailDbBusy));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailVersionInvalid => {
            return Err(ClientError::reply(LResult::FailVersionInvalid));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::LoginDownloadFile => {
            return Err(ClientError::reply(LResult::LoginDownloadFile));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailInvalidServer => {
            return Err(ClientError::reply(LResult::FailInvalidServer));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailSuspended => {
            return Err(ClientError::reply(LResult::FailSuspended));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailNoAccess => {
            return Err(ClientError::reply(LResult::FailNoAccess));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::SuccessSurvey => {
            return Err(ClientError::reply(LResult::SuccessSurvey));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailParentalcontrol => {
            return Err(ClientError::reply(LResult::FailParentalcontrol));
        }
        CMD_AUTH_LOGON_CHALLENGE_Server::FailLockedEnforced => {
            return Err(ClientError::reply(LResult::FailLockedEnforced));
        }
    };

    let security_flag = {
        let matrix_card = None;
        let authenticator = None;

        CMD_AUTH_LOGON_PROOF_Client_SecurityFlag::new(
            SecurityFlag::PIN | SecurityFlag::MATRIX_CARD | SecurityFlag::AUTHENTICATOR,
            pin,
            matrix_card,
            authenticator,
        )
    };

    CMD_AUTH_LOGON_PROOF_Client {
        client_public_key: *client.client_public_key(),
        client_proof: *client.client_proof(),
        crc_hash: [0; 20],
        telemetry_keys: vec![],
        security_flag,
    }
    .tokio_write_protocol(&mut stream, protocol_version)
    .await?;

    let s = tokio_expect_server_message_protocol::<CMD_AUTH_LOGON_PROOF_Server, _>(
        &mut stream,
        protocol_version,
    )
    .await?;
    let client = match s {
        CMD_AUTH_LOGON_PROOF_Server::Success { server_proof, .. } => {
            client.verify_server_proof(server_proof)?
        }
        CMD_AUTH_LOGON_PROOF_Server::FailUnknown0 => {
            return Err(ClientError::reply(LResult::FailUnknown0));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailUnknown1 => {
            return Err(ClientError::reply(LResult::FailUnknown1));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailBanned => {
            return Err(ClientError::reply(LResult::FailBanned));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailUnknownAccount => {
            return Err(ClientError::reply(LResult::FailUnknownAccount));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailIncorrectPassword => {
            return Err(ClientError::reply(LResult::FailIncorrectPassword));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailAlreadyOnline => {
            return Err(ClientError::reply(LResult::FailAlreadyOnline));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailNoTime => {
            return Err(ClientError::reply(LResult::FailNoTime));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailDbBusy => {
            return Err(ClientError::reply(LResult::FailDbBusy));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailVersionInvalid => {
            return Err(ClientError::reply(LResult::FailVersionInvalid));
        }
        CMD_AUTH_LOGON_PROOF_Server::LoginDownloadFile => {
            return Err(ClientError::reply(LResult::LoginDownloadFile));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailInvalidServer => {
            return Err(ClientError::reply(LResult::FailInvalidServer));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailSuspended => {
            return Err(ClientError::reply(LResult::FailSuspended));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailNoAccess => {
            return Err(ClientError::reply(LResult::FailNoAccess));
        }
        CMD_AUTH_LOGON_PROOF_Server::SuccessSurvey => {
            return Err(ClientError::reply(LResult::SuccessSurvey));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailParentalcontrol => {
            return Err(ClientError::reply(LResult::FailParentalcontrol));
        }
        CMD_AUTH_LOGON_PROOF_Server::FailLockedEnforced => {
            return Err(ClientError::reply(LResult::FailLockedEnforced));
        }
    };

    CMD_REALM_LIST_Client {}
        .tokio_write_protocol(&mut stream, protocol_version)
        .await?;

    let s = tokio_expect_server_message_protocol::<CMD_REALM_LIST_Server, _>(
        &mut stream,
        protocol_version,
    )
    .await?;

    Ok((client, s.realms, stream))
}
