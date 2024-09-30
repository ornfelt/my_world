use std::net::{Ipv4Addr, SocketAddr, SocketAddrV4};
use std::str::FromStr;
use std::thread::sleep;
use std::time::Duration;
use wow_client::connect_and_authenticate;
use wow_login_messages::all::{
    CMD_AUTH_LOGON_CHALLENGE_Client, Locale, Os, Platform, ProtocolVersion, Version,
};

#[tokio::main]
async fn main() {
    for _ in 0..10_000 {
        tokio::spawn(async {
            connect_and_authenticate(
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
                    account_name: "A".to_string(),
                },
                SocketAddr::V4(SocketAddrV4::from_str("127.0.0.1:3724").unwrap()),
                "A",
                None,
            )
            .await
            .unwrap();
            tokio::time::sleep(Duration::new(10, 0)).await;
        });
    }

    sleep(Duration::new(10, 0));
}
