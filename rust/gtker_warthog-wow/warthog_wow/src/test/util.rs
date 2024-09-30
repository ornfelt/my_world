use crate::{lib_main, ApplicationOptions};
use std::net::Ipv4Addr;
use std::sync::atomic::AtomicBool;
use std::sync::Arc;
use std::time::Duration;
use tokio::net::TcpStream;
use tokio::task::JoinHandle;
use warthog_lib::{CMD_AUTH_LOGON_CHALLENGE_Client, Options, Version};
use warthog_messages::ClientOpcodes;
use wow_client::{Locale, Os, Platform, ProtocolVersion};

pub fn vanilla_1_12(account_name: String) -> CMD_AUTH_LOGON_CHALLENGE_Client {
    CMD_AUTH_LOGON_CHALLENGE_Client {
        protocol_version: ProtocolVersion::Three,
        version: Version {
            major: 1,
            minor: 12,
            patch: 1,
            build: 5875,
        },
        platform: Platform::X86,
        os: Os::Windows,
        locale: Locale::EnGb,
        utc_timezone_offset: 60,
        client_ip_address: Ipv4Addr::new(127, 0, 0, 1),
        account_name,
    }
}

pub async fn register_realm(mut stream: &mut TcpStream, name: String, address: String) -> u8 {
    warthog_messages::ServerOpcodes::RegisterRealm {
        name,
        address,
        population: 200.0,
        locked: false,
        flags: 0,
        category: 0,
        realm_type: 0,
        version_major: 0,
        version_minor: 0,
        version_patch: 0,
        version_build: 0,
    }
    .tokio_write(&mut stream)
    .await
    .unwrap();

    match ClientOpcodes::tokio_read(&mut stream).await.unwrap() {
        ClientOpcodes::RegisterRealmReply { realm_id } => realm_id.unwrap(),
        _ => panic!(),
    }
}

pub async fn add_user(mut stream: &mut TcpStream, name: String, password: String) {
    let original_name = name.clone();

    warthog_messages::ServerOpcodes::AddUser { name, password }
        .tokio_write(&mut stream)
        .await
        .unwrap();

    match ClientOpcodes::tokio_read(&mut stream).await.unwrap() {
        ClientOpcodes::AddUserReply { name, success } => {
            assert_eq!(name, original_name);
            assert!(success);
        }
        _ => panic!(),
    }
}

pub async fn start_server(
    options: Options,
    application_options: ApplicationOptions,
) -> (Arc<AtomicBool>, JoinHandle<()>) {
    let game_address = options.address;

    let should_run = Arc::new(AtomicBool::new(true));
    let should_run_inner = should_run.clone();
    let main =
        tokio::spawn(async move { lib_main(options, application_options, should_run_inner).await });

    let mut i = 0;
    while TcpStream::connect(game_address).await.is_err() {
        assert_ne!(i, 20);

        tokio::time::sleep(Duration::new(0, 10)).await;
        i += 1;
    }

    (should_run, main)
}
