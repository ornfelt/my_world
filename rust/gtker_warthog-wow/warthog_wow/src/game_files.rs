use std::future::Future;
use std::sync::Arc;
use warthog_lib::{CMD_AUTH_LOGON_CHALLENGE_Client, GameFileProvider};

#[derive(Clone, Debug)]
pub(crate) struct GameFileImpl {}

impl GameFileProvider for GameFileImpl {
    fn get_game_files(
        &mut self,
        _message: &CMD_AUTH_LOGON_CHALLENGE_Client,
    ) -> impl Future<Output = Option<Arc<[u8]>>> + Send {
        async move { None }
    }
}
