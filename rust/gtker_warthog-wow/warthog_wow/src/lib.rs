mod credentials;
mod game_files;
mod keys;
mod patches;
mod realm_list;
mod reply;
#[cfg(test)]
mod test;

use crate::reply::start_reply_server;
use credentials::ProviderImpl;
use game_files::GameFileImpl;
use keys::KeyImpl;
use patches::PatchImpl;
use realm_list::RealmListImpl;
use std::net::SocketAddr;
use std::sync::atomic::AtomicBool;
use std::sync::Arc;
use tracing::info;
use warthog_lib::{start_auth_server, Options};

#[derive(Debug)]
pub struct ApplicationOptions {
    pub reply_address: SocketAddr,
    pub use_pin: bool,
    pub use_matrix_card: bool,
}

pub async fn lib_main(
    options: Options,
    application_options: ApplicationOptions,
    should_run: Arc<AtomicBool>,
) {
    let keys = KeyImpl::new();
    let realms = RealmListImpl::new();
    let provider = ProviderImpl::new(
        application_options.use_pin,
        application_options.use_matrix_card,
    );

    let keys_auth = keys.clone();
    let realms_auth = realms.clone();
    let provider_auth = provider.clone();
    let auth = tokio::spawn(async move {
        start_auth_server(
            provider_auth,
            keys_auth,
            PatchImpl {},
            GameFileImpl {},
            realms_auth,
            should_run,
            options,
        )
        .await
    });

    let reply = tokio::spawn(async move {
        start_reply_server(keys, realms, provider, application_options.reply_address).await
    });

    tokio::select! {
        auth = auth => {
            info!(?auth, "auth terminated");
        }
        reply = reply => {
            info!(?reply, "reply terminated");
        }
    }
}
