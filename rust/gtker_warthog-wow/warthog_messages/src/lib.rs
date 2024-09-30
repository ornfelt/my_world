mod client;
mod error;
mod server;

pub use client::*;
pub use error::*;
pub use server::*;

#[cfg(feature = "sync")]
fn read_bool<R: std::io::Read>(r: R) -> std::io::Result<bool> {
    Ok(if read_u8(r)? == 1 { true } else { false })
}

fn write_bool<W: std::io::Write>(w: W, value: bool) -> std::io::Result<()> {
    write_u8(w, if value { 1 } else { 0 })
}

#[cfg(feature = "sync")]
fn read_u8<R: std::io::Read>(mut r: R) -> std::io::Result<u8> {
    let mut buf = [0_u8; 1];
    r.read_exact(&mut buf)?;

    Ok(buf[0])
}

fn write_u8<W: std::io::Write>(mut w: W, value: u8) -> std::io::Result<()> {
    let buf = [value];
    w.write_all(&buf)
}

#[cfg(feature = "sync")]
fn read_u16<R: std::io::Read>(mut r: R) -> std::io::Result<u16> {
    let mut buf = [0_u8; 2];
    r.read_exact(&mut buf)?;

    Ok(buf[0] as u16 | (buf[1] as u16) << 8)
}

fn write_u16<W: std::io::Write>(mut w: W, value: u16) -> std::io::Result<()> {
    let buf = value.to_le_bytes();
    w.write_all(&buf)
}

#[cfg(feature = "sync")]
fn read_u32<R: std::io::Read>(mut r: R) -> std::io::Result<u32> {
    let mut buf = [0_u8; 4];
    r.read_exact(&mut buf)?;

    Ok(buf[0] as u32 | (buf[1] as u32) << 8 | (buf[2] as u32) << 16 | (buf[3] as u32) << 24)
}

fn write_u32<W: std::io::Write>(mut w: W, value: u32) -> std::io::Result<()> {
    let buf = value.to_le_bytes();
    w.write_all(&buf)
}

#[cfg(feature = "sync")]
fn read_f32<R: std::io::Read>(r: R) -> std::io::Result<f32> {
    let f = read_u32(r)?.to_le_bytes();
    Ok(f32::from_le_bytes(f))
}

fn write_f32<W: std::io::Write>(w: W, value: f32) -> std::io::Result<()> {
    Ok(write_u32(w, u32::from_le_bytes(value.to_le_bytes()))?)
}

#[cfg(feature = "sync")]
fn read_string<R: std::io::Read>(mut r: R) -> Result<String, MessageError> {
    let length = read_u8(&mut r)?;
    let mut buf = vec![0_u8; length.into()];
    r.read_exact(&mut buf)?;

    let s = String::from_utf8(buf)?;

    Ok(s)
}

fn write_string<W: std::io::Write>(mut w: W, value: &str) -> std::io::Result<()> {
    write_u8(&mut w, value.len() as u8)?;

    for b in value.as_bytes() {
        write_u8(&mut w, *b)?;
    }

    Ok(())
}

#[cfg(feature = "tokio")]
async fn read_bool_tokio<R: tokio::io::AsyncReadExt + Unpin>(r: R) -> std::io::Result<bool> {
    Ok(if read_u8_tokio(r).await? == 1 {
        true
    } else {
        false
    })
}

#[cfg(feature = "tokio")]
async fn read_u8_tokio<R: tokio::io::AsyncReadExt + Unpin>(mut r: R) -> std::io::Result<u8> {
    let mut buf = [0_u8; 1];
    r.read_exact(&mut buf).await?;

    Ok(buf[0])
}

#[cfg(feature = "tokio")]
async fn read_u16_tokio<R: tokio::io::AsyncReadExt + Unpin>(mut r: R) -> std::io::Result<u16> {
    let mut buf = [0_u8; 2];
    r.read_exact(&mut buf).await?;

    Ok(buf[0] as u16 | (buf[1] as u16) << 8)
}

#[cfg(feature = "tokio")]
async fn read_u32_tokio<R: tokio::io::AsyncReadExt + Unpin>(mut r: R) -> std::io::Result<u32> {
    let mut buf = [0_u8; 4];
    r.read_exact(&mut buf).await?;

    Ok(buf[0] as u32 | (buf[1] as u32) << 8 | (buf[2] as u32) << 16 | (buf[3] as u32) << 24)
}

#[cfg(feature = "tokio")]
async fn read_f32_tokio<R: tokio::io::AsyncReadExt + Unpin>(r: R) -> std::io::Result<f32> {
    let f = read_u32_tokio(r).await?.to_le_bytes();
    Ok(f32::from_le_bytes(f))
}

#[cfg(feature = "tokio")]
async fn read_string_tokio<R: tokio::io::AsyncReadExt + Unpin>(
    mut r: R,
) -> Result<String, MessageError> {
    let length = read_u8_tokio(&mut r).await?;
    let mut buf = vec![0_u8; length.into()];
    r.read_exact(&mut buf).await?;

    let s = String::from_utf8(buf)?;

    Ok(s)
}
