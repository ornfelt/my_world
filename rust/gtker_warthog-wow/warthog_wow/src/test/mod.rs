mod util;

use crate::test::util::{add_user, register_realm, start_server, vanilla_1_12};
use crate::ApplicationOptions;
use std::net::{IpAddr, Ipv4Addr, SocketAddr};
use std::sync::atomic::Ordering;
use tokio::net::TcpStream;
use warthog_lib::{Options, Population};
use wow_client::connect_and_authenticate;

#[tokio::test]
async fn register_realms() {
    const REPLY_PORT: u16 = 32657;
    const GAME_PORT: u16 = REPLY_PORT + 1;

    const REPLY_ADDRESS: SocketAddr = SocketAddr::new(IpAddr::V4(Ipv4Addr::LOCALHOST), REPLY_PORT);
    const GAME_ADDRESS: SocketAddr = SocketAddr::new(IpAddr::V4(Ipv4Addr::LOCALHOST), GAME_PORT);

    const APPLICATION_OPTIONS: ApplicationOptions = ApplicationOptions {
        reply_address: REPLY_ADDRESS,
        use_pin: false,
        use_matrix_card: false,
    };

    const OPTIONS: Options = Options {
        address: GAME_ADDRESS,
        randomize_pin_grid: false,
        max_concurrent_users: 10000,
    };

    let (should_run, main) = start_server(OPTIONS, APPLICATION_OPTIONS).await;

    let mut reply = TcpStream::connect(REPLY_ADDRESS).await.unwrap();
    add_user(&mut reply, "A".to_string(), "A".to_string()).await;

    {
        let (_, realms, _) =
            connect_and_authenticate(vanilla_1_12("A".to_string()), GAME_ADDRESS, "A", None)
                .await
                .unwrap();

        assert!(realms.is_empty());
    }

    const REALM_NAME: &str = "Test Realm";
    const REALM_ADDRESS: &str = "localhost:8085";

    let realm_id = register_realm(
        &mut reply,
        REALM_NAME.to_string(),
        REALM_ADDRESS.to_string(),
    )
    .await;

    {
        let (_, realms, _) =
            connect_and_authenticate(vanilla_1_12("A".to_string()), GAME_ADDRESS, "A", None)
                .await
                .unwrap();

        match realms.as_slice() {
            [realm] => {
                assert_eq!(realm.population, Population::from(200.0));
                assert_eq!(realm.locked, false);
                assert_eq!(realm.name, REALM_NAME);
                assert_eq!(realm.address, REALM_ADDRESS);
                assert_eq!(realm.realm_id, realm_id);
            }
            _ => panic!(),
        }
        assert!(!realms.is_empty());
    }

    const REALM2_NAME: &str = "Test Realm2";
    const REALM2_ADDRESS: &str = "localhost:8088";

    let mut reply2 = TcpStream::connect(REPLY_ADDRESS).await.unwrap();
    let realm_id2 = register_realm(
        &mut reply2,
        REALM2_NAME.to_string(),
        REALM2_ADDRESS.to_string(),
    )
    .await;

    {
        let (_, realms, _) =
            connect_and_authenticate(vanilla_1_12("A".to_string()), GAME_ADDRESS, "A", None)
                .await
                .unwrap();

        match realms.as_slice() {
            [realm, realm2] => {
                let (first, second) = if realm.realm_id == 0 {
                    (&realm, &realm2)
                } else {
                    (&realm2, &realm)
                };

                assert_eq!(first.population, Population::from(200.0));
                assert_eq!(first.locked, false);
                assert_eq!(first.name, REALM_NAME);
                assert_eq!(first.address, REALM_ADDRESS);
                assert_eq!(first.realm_id, realm_id);

                assert_eq!(second.population, Population::from(200.0));
                assert_eq!(second.locked, false);
                assert_eq!(second.name, REALM2_NAME);
                assert_eq!(second.address, REALM2_ADDRESS);
                assert_eq!(second.realm_id, realm_id2);
            }
            _ => panic!(),
        }
        assert!(!realms.is_empty());
    }

    should_run.store(false, Ordering::SeqCst);
    main.await.unwrap();
}
