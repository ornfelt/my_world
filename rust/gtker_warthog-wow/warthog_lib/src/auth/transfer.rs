use tokio::net::TcpStream;
use tracing::{error, info, trace, warn};
use wow_login_messages::all::CMD_AUTH_LOGON_CHALLENGE_Client;
use wow_login_messages::version_8::opcodes::ClientOpcodeMessage;
use wow_login_messages::version_8::CMD_AUTH_LOGON_CHALLENGE_Server;
use wow_login_messages::version_8::{CMD_XFER_DATA, CMD_XFER_INITIATE};
use wow_login_messages::{CollectiveMessage, Message};

#[tracing::instrument]
pub(crate) async fn transfer(
    mut stream: TcpStream,
    c: CMD_AUTH_LOGON_CHALLENGE_Client,
    data: &[u8],
    file_size: u64,
    file_md5: [u8; 16],
) -> std::io::Result<()> {
    trace!("starting file transfer");
    CMD_AUTH_LOGON_CHALLENGE_Server::LoginDownloadFile
        .tokio_write_protocol(&mut stream, c.protocol_version)
        .await?;

    CMD_XFER_INITIATE {
        filename: "Patch".to_string(),
        file_size,
        file_md5,
    }
    .tokio_write(&mut stream)
    .await?;

    let s = match ClientOpcodeMessage::tokio_read_protocol(&mut stream, c.protocol_version).await {
        Ok(s) => s,
        Err(err) => {
            error!(?err, "incorrect opcode received during transfer or resume");
            return Ok(());
        }
    };

    let offset = match s {
        ClientOpcodeMessage::CMD_XFER_ACCEPT => 0,
        ClientOpcodeMessage::CMD_XFER_RESUME(r) => match r.offset.try_into() {
            Ok(e) => {
                if e > data.len() {
                    error!(message = ?c, size = e, "transfer offset larger data");
                    return Ok(());
                } else {
                    e
                }
            }
            Err(_) => {
                error!(message = ?c, size = r.offset, "transfer offset too large to fit in usize");
                return Ok(());
            }
        },
        opcode => {
            warn!(message = ?c, ?opcode, "invalid message received");
            return Ok(());
        }
    };

    const TRANSFER_CHUNK: usize = 64;

    for i in data[offset..].chunks(TRANSFER_CHUNK) {
        let mut buf = [0_u8; 1];
        let size = stream.peek(&mut buf).await?;
        if size != 0 {
            // Client doesn't send any messages other than CMD_XFER_CANCEL
            info!("client sent CMD_XFER_CANCEL");
            return Ok(());
        }

        CMD_XFER_DATA { data: i.to_vec() }
            .tokio_write(&mut stream)
            .await?;
    }

    // Keep the connection alive until the client breaks it off and updates
    while let Ok(m) = ClientOpcodeMessage::tokio_read(&mut stream).await {
        dbg!(m);
    }

    Ok(())
}
