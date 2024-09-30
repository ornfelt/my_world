use std::collections::HashMap;
use std::future::Future;
use std::sync::{Arc, Mutex};
use warthog_lib::{KeyStorage, SrpServer};

#[derive(Clone, Debug)]
pub(crate) struct KeyImpl {
    inner: Arc<Mutex<HashMap<String, SrpServer>>>,
}

impl KeyImpl {
    pub(crate) fn new() -> Self {
        Self {
            inner: Arc::new(Mutex::new(Default::default())),
        }
    }
}

impl KeyStorage for KeyImpl {
    fn add_key(&mut self, username: String, server: SrpServer) -> impl Future<Output = ()> + Send {
        async move {
            self.inner.lock().unwrap().insert(username, server);
        }
    }

    fn get_key_for_user(
        &mut self,
        username: &str,
    ) -> impl Future<Output = Option<SrpServer>> + Send {
        async move { self.inner.lock().unwrap().get(username).cloned() }
    }
}
