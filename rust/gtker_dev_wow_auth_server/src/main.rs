use io::{Read, Write};
use std::collections::HashMap;
use std::net::{SocketAddr, TcpListener, TcpStream};
use std::sync::{Arc, Mutex};
use std::thread::sleep;
use std::time::Duration;
use std::{io, thread};
use wow_login_messages::all::{
    CMD_AUTH_LOGON_CHALLENGE_Client, CMD_AUTH_RECONNECT_CHALLENGE_Client, ProtocolVersion,
};
use wow_login_messages::errors::ExpectedOpcodeError;
use wow_login_messages::helper::{
    expect_client_message_protocol, read_initial_message, InitialMessage,
};
use wow_srp::normalized_string::NormalizedString;
use wow_srp::server::{SrpProof, SrpServer, SrpVerifier};
use wow_srp::{PublicKey, GENERATOR, LARGE_SAFE_PRIME_LITTLE_ENDIAN};

use clap::Parser;
use log::{error, info};
use wow_login_messages::version_8::CMD_AUTH_LOGON_PROOF_Client;
use wow_login_messages::version_8::CMD_AUTH_RECONNECT_CHALLENGE_Server_LoginResult;
use wow_login_messages::version_8::{
    AccountFlag, CMD_AUTH_LOGON_CHALLENGE_Server_SecurityFlag, CMD_AUTH_LOGON_PROOF_Server,
    CMD_AUTH_LOGON_PROOF_Server_LoginResult, CMD_AUTH_RECONNECT_PROOF_Client,
    CMD_REALM_LIST_Client, Realm_RealmFlag,
};
use wow_login_messages::version_8::{
    CMD_AUTH_LOGON_CHALLENGE_Server, CMD_AUTH_LOGON_CHALLENGE_Server_LoginResult,
    CMD_AUTH_RECONNECT_CHALLENGE_Server,
};
use wow_login_messages::version_8::{CMD_AUTH_RECONNECT_PROOF_Server, LoginResult};
use wow_login_messages::version_8::{CMD_REALM_LIST_Server, Realm, RealmType};
use wow_login_messages::CollectiveMessage;

#[derive(Parser, Copy, Clone)]
#[command(author, version, about, long_about = None)]
struct Options {
    /// Socket of authentication/login server.
    #[arg(short, long, default_value = "127.0.0.1:3724")]
    auth: SocketAddr,
    /// Socket of world server (where the realm list points to).
    #[arg(short, long, default_value = "127.0.0.1:8085")]
    world: SocketAddr,
    /// Socket of reply server (where you query session keys).
    #[arg(short, long, default_value = "127.0.0.1:8086")]
    reply: SocketAddr,
}

fn main() {
    simple_logger::SimpleLogger::new().env().init().unwrap();

    let cli = Options::parse();

    let users = Arc::new(Mutex::new(HashMap::new()));

    let reply_users = users.clone();

    let auth_thread = thread::spawn(move || auth(users, &cli));

    let reply_thread = thread::spawn(move || reply(reply_users, &cli));

    reply_thread.join().unwrap();
    auth_thread.join().unwrap();
}

fn reply(users: Arc<Mutex<HashMap<String, SrpServer>>>, options: &Options) {
    let listener = TcpListener::bind(options.reply).unwrap();
    info!("[REPLY] Listening for queries on {}", options.reply);

    loop {
        let (stream, _) = listener.accept().unwrap();

        let users = users.clone();
        thread::spawn(move || {
            let peer_address = stream.peer_addr().unwrap();
            info!("[REPLY] Connected to {}", peer_address);

            match handle_reply(stream, users) {
                Ok(_) => {}
                Err(_) => info!("[REPLY] Lost connection to {}", peer_address),
            }
        });
    }
}

fn handle_reply(
    mut stream: TcpStream,
    users: Arc<Mutex<HashMap<String, SrpServer>>>,
) -> io::Result<()> {
    let mut buf = [0_u8; 1];
    loop {
        stream.read_exact(&mut buf)?;

        let name_length = buf[0];

        let mut v = Vec::with_capacity(name_length.into());

        for _ in 0..name_length {
            stream.read_exact(&mut buf)?;
            v.push(buf[0]);
        }

        let name = match String::from_utf8(v.clone()) {
            Ok(s) => s,
            Err(e) => {
                error!("[REPLY] Invalid UTF-8 string name: '{:?}'", v);
                return Err(io::Error::new(io::ErrorKind::Other, e));
            }
        };
        info!("[REPLY] '{}' Received query", name);

        let session_key = {
            let u = users.lock().unwrap();
            if let Some(u) = u.get(&name) {
                Some(*u.session_key())
            } else {
                None
            }
        };

        let vec_size = 2 + name.as_bytes().len() + if session_key.is_some() { 40 } else { 0 };
        let mut buffer = Vec::with_capacity(vec_size);

        buffer.push(name.len() as u8);

        for v in name.as_bytes() {
            buffer.push(*v);
        }

        if let Some(session_key) = session_key {
            info!("[REPLY] '{}': Session key found: {:?}", name, session_key);
            buffer.push(1);

            for v in session_key {
                buffer.push(v);
            }
        } else {
            info!("[REPLY] '{}': Session key not found", name);
            buffer.push(0);
        }

        stream.write_all(&buffer)?;
    }
}

fn auth(users: Arc<Mutex<HashMap<String, SrpServer>>>, options: &Options) {
    let listener = TcpListener::bind(options.auth).unwrap();

    let options = *options;

    info!("[AUTH] Listening for authentications on {}", options.auth);
    loop {
        let (stream, _) = listener.accept().unwrap();

        let users = users.clone();
        thread::spawn(move || {
            let peer_addr = stream.peer_addr().unwrap();
            info!("[REPLY] Connected to {}", peer_addr);
            match handle_auth(stream, users, &options) {
                Ok(_) => {}
                Err(_) => {
                    error!("[AUTH] Connection lost to {}", peer_addr)
                }
            }
        });
    }
}

fn handle_auth(
    mut stream: TcpStream,
    users: Arc<Mutex<HashMap<String, SrpServer>>>,
    options: &Options,
) -> io::Result<()> {
    let opcode = read_initial_message(&mut stream);
    let opcode = match opcode {
        Ok(o) => o,
        Err(e) => {
            match &e {
                ExpectedOpcodeError::Opcode(o) => {
                    error!("[AUTH] invalid opcode {}", o)
                }
                ExpectedOpcodeError::Parse(e) => {
                    error!("[AUTH] parse error {:#?}", e)
                }
                ExpectedOpcodeError::Io(e) => {
                    error!("[AUTH] io error {:#?}", e)
                }
            }
            return Err(io::Error::new(io::ErrorKind::Other, e));
        }
    };

    match opcode {
        InitialMessage::Logon(l) => match l.protocol_version {
            ProtocolVersion::Two => login(stream, l, users, options, ProtocolVersion::Two),
            ProtocolVersion::Three => login(stream, l, users, options, ProtocolVersion::Three),
            ProtocolVersion::Five => login(stream, l, users, options, ProtocolVersion::Five),
            ProtocolVersion::Six => login(stream, l, users, options, ProtocolVersion::Six),
            ProtocolVersion::Seven => login(stream, l, users, options, ProtocolVersion::Seven),
            ProtocolVersion::Eight => login(stream, l, users, options, ProtocolVersion::Eight),
        },
        InitialMessage::Reconnect(r) => match r.protocol_version {
            ProtocolVersion::Two => reconnect(stream, r, users, options, ProtocolVersion::Two),
            ProtocolVersion::Five => reconnect(stream, r, users, options, ProtocolVersion::Five),
            ProtocolVersion::Six => reconnect(stream, r, users, options, ProtocolVersion::Six),
            ProtocolVersion::Seven => reconnect(stream, r, users, options, ProtocolVersion::Seven),
            ProtocolVersion::Eight => reconnect(stream, r, users, options, ProtocolVersion::Eight),
            ProtocolVersion::Three => panic!("invalid reconnect flag: Three"),
        },
    }

    Ok(())
}

fn reconnect(
    mut stream: TcpStream,
    r: CMD_AUTH_RECONNECT_CHALLENGE_Client,
    users: Arc<Mutex<HashMap<String, SrpServer>>>,
    options: &Options,
    protocol_version: ProtocolVersion,
) {
    info!(
        "[AUTH] '{}' Reconnect version: {}",
        r.account_name, r.protocol_version
    );
    let username = r.account_name.clone();

    let server_reconnect_challenge_data = *users
        .lock()
        .unwrap()
        .get(&r.account_name)
        .unwrap()
        .reconnect_challenge_data();

    CMD_AUTH_RECONNECT_CHALLENGE_Server {
        result: CMD_AUTH_RECONNECT_CHALLENGE_Server_LoginResult::Success {
            challenge_data: server_reconnect_challenge_data,
            checksum_salt: [0; 16],
        },
    }
        .write_protocol(&mut stream, r.protocol_version)
        .unwrap();

    let p = expect_client_message_protocol::<CMD_AUTH_RECONNECT_PROOF_Client, _>(
        &mut stream,
        protocol_version,
    )
        .unwrap();
    let (proof_data, client_proof) = (p.proof_data, p.client_proof);

    let success = {
        match users.lock().unwrap().get_mut(&r.account_name) {
            None => false,
            Some(server) => server.verify_reconnection_attempt(proof_data, client_proof),
        }
    };

    if !success {
        CMD_AUTH_RECONNECT_PROOF_Server {
            result: LoginResult::FailIncorrectPassword,
        }
            .write_protocol(&mut stream, protocol_version)
            .unwrap();

        return;
    }

    CMD_AUTH_RECONNECT_PROOF_Server {
        result: LoginResult::Success,
    }
        .write_protocol(&mut stream, protocol_version)
        .unwrap();

    print_version(stream, protocol_version, &username, options);
}

fn login(
    mut stream: TcpStream,
    l: CMD_AUTH_LOGON_CHALLENGE_Client,
    users: Arc<Mutex<HashMap<String, SrpServer>>>,
    options: &Options,
    protocol_version: ProtocolVersion,
) {
    info!(
        "[AUTH] '{}' Login version: {}",
        l.account_name, l.protocol_version
    );
    let p = get_proof(&l.account_name);
    let username = l.account_name;

    CMD_AUTH_LOGON_CHALLENGE_Server {
        result: CMD_AUTH_LOGON_CHALLENGE_Server_LoginResult::Success {
            crc_salt: [0; 16],
            generator: vec![GENERATOR],
            large_safe_prime: LARGE_SAFE_PRIME_LITTLE_ENDIAN.into(),
            salt: *p.salt(),
            security_flag: CMD_AUTH_LOGON_CHALLENGE_Server_SecurityFlag::empty(),
            server_public_key: *p.server_public_key(),
        },
    }
        .write_protocol(&mut stream, protocol_version)
        .unwrap();

    let c = expect_client_message_protocol::<CMD_AUTH_LOGON_PROOF_Client, _>(
        &mut stream,
        protocol_version,
    )
        .unwrap();
    let (client_public_key, client_proof) = (c.client_public_key, c.client_proof);

    let (p, proof) = if let Ok(a) = p.into_server(
        PublicKey::from_le_bytes(client_public_key).unwrap(),
        client_proof,
    ) {
        a
    } else {
        CMD_AUTH_LOGON_PROOF_Server {
            result: CMD_AUTH_LOGON_PROOF_Server_LoginResult::FailIncorrectPassword,
        }
            .write_protocol(&mut stream, protocol_version)
            .unwrap();

        sleep(Duration::from_secs(1));
        return;
    };

    CMD_AUTH_LOGON_PROOF_Server {
        result: CMD_AUTH_LOGON_PROOF_Server_LoginResult::Success {
            account_flag: AccountFlag::empty(),
            hardware_survey_id: 0,
            server_proof: proof,
            unknown: 0,
        },
    }
        .write_protocol(&mut stream, protocol_version)
        .unwrap();

    users.lock().unwrap().insert(username.clone(), p);

    print_version(stream, protocol_version, &username, options);
}

fn get_proof(username: &str) -> SrpProof {
    let username = NormalizedString::new(username.to_string()).unwrap();
    let password = NormalizedString::new(username.to_string()).unwrap();
    SrpVerifier::from_username_and_password(username, password).into_proof()
}

fn print_version(
    mut stream: TcpStream,
    protocol_version: ProtocolVersion,
    username: &str,
    options: &Options,
) {
    while expect_client_message_protocol::<CMD_REALM_LIST_Client, _>(&mut stream, protocol_version)
        .is_ok()
    {
        CMD_REALM_LIST_Server {
            realms: vec![Realm {
                realm_type: RealmType::PlayerVsEnvironment,
                locked: false,
                flag: Realm_RealmFlag::empty(),
                name: "Tester".to_string(),
                address: options.world.to_string(),
                population: Default::default(),
                number_of_characters_on_realm: 0,
                category: Default::default(),
                realm_id: 0,
            }],
        }
            .write_protocol(&mut stream, protocol_version)
            .unwrap();

        info!("[AUTH] '{}' Sent Version 2/3 Realm List", username);
    }
}
