use std::future::Future;
use warthog_lib::{CMD_AUTH_LOGON_CHALLENGE_Client, PatchFile, PatchProvider};

#[derive(Clone, Debug)]
pub(crate) struct PatchImpl {}

impl PatchProvider for PatchImpl {
    fn get_patch(
        &mut self,
        _message: &CMD_AUTH_LOGON_CHALLENGE_Client,
    ) -> impl Future<Output = Option<PatchFile>> + Send {
        async move { None }
    }
}
