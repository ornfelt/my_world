use std::collections::BTreeSet;
use std::future::Future;
use std::sync::{Arc, Mutex};
use tracing::{error, info};
use warthog_lib::{
    CMD_AUTH_LOGON_CHALLENGE_Client, Population, Realm, RealmCategory, RealmListProvider,
    RealmType, Realm_RealmFlag,
};

#[derive(Clone, Debug)]
pub(crate) struct RealmListImpl {
    realms: Arc<Mutex<Vec<Realm>>>,
}

impl RealmListImpl {
    pub fn new() -> Self {
        Self {
            realms: Arc::new(Mutex::new(vec![])),
        }
    }

    fn first_available_realm_id(&self) -> Option<u8> {
        let mut ids_in_use = BTreeSet::new();

        for realm in self.realms.lock().unwrap().as_slice() {
            ids_in_use.insert(realm.realm_id);
        }

        for i in 0..=u8::MAX {
            if ids_in_use.get(&i).is_none() {
                return Some(i);
            }
        }

        None
    }

    #[tracing::instrument]
    pub fn add_realm(
        &mut self,
        name: String,
        address: String,
        population: Population,
        locked: bool,
        flag: Realm_RealmFlag,
        category: RealmCategory,
        realm_type: RealmType,
        realm_id: Option<u8>,
    ) -> Option<u8> {
        if let Some(realm_id) = realm_id {
            if let Some(realm) = self
                .realms
                .lock()
                .unwrap()
                .iter_mut()
                .find(|a| a.realm_id == realm_id)
            {
                realm.name = name;
                realm.address = address;
                realm.population = population;
                realm.locked = locked;
                realm.flag = flag;
                realm.category = category;
                realm.realm_type = realm_type;

                Some(realm_id)
            } else {
                error!("Unable to realm with id {realm_id:?}");

                None
            }
        } else {
            if let Some(realm_id) = self.first_available_realm_id() {
                self.realms.lock().unwrap().push(Realm {
                    realm_type,
                    locked,
                    flag,
                    name,
                    address,
                    population,
                    number_of_characters_on_realm: 0,
                    category,
                    realm_id,
                });

                info!(realm_id, "adding realm");

                Some(realm_id)
            } else {
                error!("Unable to find available realm id");

                None
            }
        }
    }

    #[tracing::instrument]
    pub fn remove_realm(&mut self, realm_id: u8) {
        if let Some((i, _)) = self
            .realms
            .lock()
            .unwrap()
            .iter()
            .enumerate()
            .find(|(_, a)| a.realm_id == realm_id)
        {
            info!("removing realm");
            self.realms.lock().unwrap().remove(i);
        }
    }
}

impl RealmListProvider for RealmListImpl {
    fn get_realm_list(
        &mut self,
        _message: &CMD_AUTH_LOGON_CHALLENGE_Client,
    ) -> impl Future<Output = Vec<Realm>> + Send {
        async move { self.realms.lock().unwrap().clone() }
    }
}
