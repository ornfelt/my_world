mod auth;

use std::fmt::Debug;
use std::future::Future;
use std::net::SocketAddr;
use std::sync::atomic::{AtomicBool, AtomicU32, Ordering};
use std::sync::Arc;
use std::time::Duration;
use tokio::net::TcpListener;
use tracing::info;

use crate::auth::auth;

pub use wow_login_messages::all::CMD_AUTH_LOGON_CHALLENGE_Client;
pub use wow_login_messages::all::CMD_AUTH_RECONNECT_CHALLENGE_Client;
pub use wow_login_messages::all::Population;
pub use wow_login_messages::all::Version;
pub use wow_login_messages::errors::ExpectedOpcodeError;
pub use wow_login_messages::version_8::opcodes::ClientOpcodeMessage;
pub use wow_login_messages::version_8::Realm;

pub use wow_login_messages::version_8::RealmCategory;
pub use wow_login_messages::version_8::RealmFlag;
pub use wow_login_messages::version_8::RealmType;
pub use wow_login_messages::version_8::Realm_RealmFlag;
pub use wow_login_messages::version_8::Realm_RealmFlag_SpecifyBuild;
pub use wow_srp::error::InvalidPublicKeyError;
pub use wow_srp::matrix_card::MatrixCard;
pub use wow_srp::matrix_card::MatrixCardVerifier;
pub use wow_srp::normalized_string::NormalizedString;
pub use wow_srp::pin::PinCode;
pub use wow_srp::server::SrpServer;
pub use wow_srp::server::SrpVerifier;
pub use wow_srp::PASSWORD_VERIFIER_LENGTH;
pub use wow_srp::SALT_LENGTH;

#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct Options {
    /// Address to host the auth server on.
    pub address: SocketAddr,
    /// Shift around numbers on the PIN grid.
    pub randomize_pin_grid: bool,
    /// Maximum amount of concurrent users.
    pub max_concurrent_users: u32,
}

#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct Credentials {
    pub password_verifier: [u8; PASSWORD_VERIFIER_LENGTH as usize],
    pub salt: [u8; SALT_LENGTH as usize],
    pub pin: Option<PinCode>,
    pub matrix_card: Option<MatrixCardOptions>,
}

#[derive(Debug, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct MatrixCardOptions {
    pub matrix_card: MatrixCard,
    pub challenge_count: u8,
}

#[derive(Copy, Clone, Debug, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub enum ExpectedOpcode {
    LoginOrReconnect,
    LogonProof,
    ReconnectProof,
    XferOrResume,
}

pub trait CredentialProvider: std::fmt::Debug + Clone + Send + Sync + 'static {
    fn get_user(
        &mut self,
        message: &CMD_AUTH_LOGON_CHALLENGE_Client,
    ) -> impl Future<Output = Option<Credentials>> + Send;

    fn add_user(
        &mut self,
        username: &str,
        password: &str,
    ) -> impl Future<Output = Option<()>> + Send;

    fn remove_user(&mut self, username: &str) -> impl Future<Output = bool> + Send;

    fn modify_user(&mut self, username: &str) -> impl Future<Output = bool> + Send;
}

pub trait KeyStorage: Debug + Clone + Send + Sync + 'static {
    fn add_key(&mut self, username: String, server: SrpServer) -> impl Future<Output = ()> + Send;

    fn get_key_for_user(
        &mut self,
        username: &str,
    ) -> impl Future<Output = Option<SrpServer>> + Send;
}

#[derive(Debug, Clone)]
pub struct PatchFile {
    data: Arc<[u8]>,
    md5: [u8; 16],
    size: u64,
}

impl PatchFile {
    fn verify_size(size: usize) -> Option<u64> {
        let s = size.try_into();
        s.ok()
    }

    pub fn data_size(&self) -> u64 {
        self.size
    }

    pub fn md5(&self) -> &[u8; 16] {
        &self.md5
    }

    pub fn data(&self) -> &[u8] {
        &self.data
    }

    pub fn new(data: Arc<[u8]>) -> Option<Self> {
        if let Some(size) = Self::verify_size(data.len()) {
            let md5 = md5::compute(&data).0;
            Some(Self { data, md5, size })
        } else {
            None
        }
    }
}

pub trait PatchProvider: Debug + Clone + Send + Sync + 'static {
    fn get_patch(
        &mut self,
        message: &CMD_AUTH_LOGON_CHALLENGE_Client,
    ) -> impl Future<Output = Option<PatchFile>> + Send;
}

pub trait GameFileProvider: Debug + Clone + Send + Sync + 'static {
    fn get_game_files(
        &mut self,
        message: &CMD_AUTH_LOGON_CHALLENGE_Client,
    ) -> impl Future<Output = Option<Arc<[u8]>>> + Send;
}

pub trait RealmListProvider: Debug + Clone + Send + Sync + 'static {
    fn get_realm_list(
        &mut self,
        message: &CMD_AUTH_LOGON_CHALLENGE_Client,
    ) -> impl Future<Output = Vec<Realm>> + Send;
}

#[tracing::instrument(skip(
    provider,
    storage,
    patch_provider,
    game_file_provider,
    realm_list_provider,
    should_run
))]
pub async fn start_auth_server(
    provider: impl CredentialProvider,
    storage: impl KeyStorage,
    patch_provider: impl PatchProvider,
    game_file_provider: impl GameFileProvider,
    realm_list_provider: impl RealmListProvider,
    should_run: Arc<AtomicBool>,
    options: Options,
) -> std::io::Result<()> {
    let options: &'static mut _ = Box::leak(Box::new(options));
    let listener = TcpListener::bind(options.address).await?;
    info!("auth server started");

    let concurrent_connections = Arc::new(AtomicU32::new(0));

    let should_run = tokio::spawn(async move {
        while should_run.load(Ordering::SeqCst) {
            tokio::time::sleep(Duration::new(0, 100)).await;
        }
    });

    let main_loop = tokio::spawn(async move {
        loop {
            let connections = concurrent_connections.clone();
            if connections.load(Ordering::SeqCst) > options.max_concurrent_users {
                continue;
            }

            if let Ok((stream, _)) = listener.accept().await {
                connections.fetch_add(1, Ordering::SeqCst);
                let provider = provider.clone();
                let storage = storage.clone();
                let patch_provider = patch_provider.clone();
                let game_file_provider = game_file_provider.clone();
                let realm_list_provider = realm_list_provider.clone();
                let options: &'static _ = &*options;

                tokio::spawn(async move {
                    auth(
                        stream,
                        provider,
                        storage,
                        patch_provider,
                        game_file_provider,
                        realm_list_provider,
                        options,
                    )
                    .await;

                    connections.fetch_sub(1, Ordering::SeqCst);
                });
            }
        }
    });

    tokio::select! {
        _ = should_run => {}
        _ = main_loop => {}
    }

    Ok(())
}
