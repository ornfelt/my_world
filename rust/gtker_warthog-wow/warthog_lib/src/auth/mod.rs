mod logon;
mod reconnect;
mod transfer;

use crate::auth::logon::logon;
use crate::{
    CredentialProvider, GameFileProvider, KeyStorage, Options, PatchProvider, RealmListProvider,
};
use std::io;
use tokio::net::TcpStream;
use tracing::{error, trace};
use wow_login_messages::all::CMD_AUTH_LOGON_CHALLENGE_Client;
use wow_login_messages::helper::{
    tokio_expect_client_message_protocol, tokio_read_initial_message, InitialMessage,
};
use wow_login_messages::version_2::CMD_REALM_LIST_Client;
use wow_login_messages::version_8::CMD_REALM_LIST_Server;
use wow_login_messages::CollectiveMessage;

#[tracing::instrument(skip(
    provider,
    storage,
    patch_provider,
    game_file_provider,
    realm_list_provider,
    options
))]
pub(crate) async fn auth(
    mut stream: TcpStream,
    provider: impl CredentialProvider,
    storage: impl KeyStorage,
    mut patch_provider: impl PatchProvider,
    game_file_provider: impl GameFileProvider,
    realm_list_provider: impl RealmListProvider,
    options: &Options,
) {
    trace!("connected");
    let c = match tokio_read_initial_message(&mut stream).await {
        Ok(c) => c,
        Err(err) => {
            error!(?err, "incorrect opcode during initial connection");
            return;
        }
    };

    match c {
        InitialMessage::Logon(c) => {
            if let Some(data) = patch_provider.get_patch(&c).await {
                let size = data.data_size();
                if let Err(e) = transfer::transfer(stream, c, data.data(), size, *data.md5()).await
                {
                    error!(?e, "io error during transfer");
                }

                return;
            }

            if let Err(e) = logon(
                provider,
                storage,
                game_file_provider,
                realm_list_provider,
                stream,
                c,
                options,
            )
            .await
            {
                error!(?e, "io error during login")
            }
        }
        InitialMessage::Reconnect(c) => {
            if let Err(e) = reconnect::reconnect(storage, realm_list_provider, stream, c).await {
                error!(?e, "io error during reconnect")
            }
        }
    }
}

pub(crate) async fn send_realm_list(
    mut stream: &mut TcpStream,
    c: &CMD_AUTH_LOGON_CHALLENGE_Client,
    mut realm_list_provider: impl RealmListProvider,
) -> io::Result<()> {
    while tokio_expect_client_message_protocol::<CMD_REALM_LIST_Client, _>(
        &mut stream,
        c.protocol_version,
    )
    .await
    .is_ok()
    {
        let realms = realm_list_provider.get_realm_list(c).await;

        CMD_REALM_LIST_Server { realms }
            .tokio_write_protocol(&mut stream, c.protocol_version)
            .await?;
    }

    Ok(())
}
