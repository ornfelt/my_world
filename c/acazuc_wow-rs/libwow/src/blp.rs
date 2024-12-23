use super::{
	Result,
	Error,
};

use bytemuck::{
	Pod,
	Zeroable,
};

use std::io::{
	Read,
	Seek,
	SeekFrom,
};

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
pub struct Header {
	pub magic: u32,
	pub version: u32,
	pub compression: u8,
	pub alpha_depth: u8,
	pub alpha_type: u8,
	pub has_mipmaps: u8,
	pub width: u32,
	pub height: u32,
	pub mipmap_offsets: [u32; 16],
	pub mipmap_lengths: [u32; 16],
	pub palette: [u32; 256],
}

#[derive(Debug)]
pub struct Mipmap {
	pub width: usize,
	pub height: usize,
	pub data: Vec<u8>,
}

#[derive(Debug)]
pub struct Texture {
	pub header: Header,
	pub mipmaps: Vec<Mipmap>,
}

impl Default for Header {
	fn default() -> Self {
		Self {
			magic: Default::default(),
			version: Default::default(),
			compression: Default::default(),
			alpha_depth: Default::default(),
			alpha_type: Default::default(),
			has_mipmaps: Default::default(),
			width: Default::default(),
			height: Default::default(),
			mipmap_offsets: Default::default(),
			mipmap_lengths: Default::default(),
			palette: [u32::default(); 256],
		}
	}
}

impl Header {
	fn read(reader: &mut impl Read) -> Result<Self> {
		let mut header: [Self; 1] = Default::default();
		reader.read_exact(bytemuck::cast_slice_mut(&mut header))?;
		if header[0].magic != 0x32504C42 {
			return Err(Error::InvalidMagic)
		}
		Ok(header[0])
	}
}

impl Mipmap {
	fn read(
		reader: &mut (impl Read + Seek),
		width: usize,
		height: usize,
		offset: u64,
		size: usize,
	) -> Result<Self> {
		let mut data: Vec<u8> = vec![0; size];
		reader.seek(SeekFrom::Start(offset))?;
		reader.read_exact(&mut data)?;
		Ok(Self {
			width,
			height,
			data,
		})
	}

	fn read_indexed(
		reader: &mut (impl Read + Seek),
		width: usize,
		height: usize,
		offset: u64,
		palette: &[u32; 256],
		alpha_depth: u8,
	) -> Result<Self> {
		let mut data: Vec<u8> = vec![0; width * height * 4];
		let mut indexes: Vec<u8> = vec![0; width * height];
		let mut alphas: Vec<u8> = match alpha_depth {
			1 => vec![0; width * height / 8],
			4 => vec![0; width * height / 2],
			8 => vec![0; width * height],
			_ => vec![0; 0],
		};
		reader.seek(SeekFrom::Start(offset))?;
		reader.read_exact(&mut indexes)?;
		if alphas.len() > 0 {
			reader.read_exact(&mut alphas)?;
		}
		indexes.iter().enumerate().for_each(|(i, index)| {
			let v = palette[*index as usize];
			data[i * 4 + 0] = ((v >> 16) & 0xFF) as u8;
			data[i * 4 + 1] = ((v >>  8) & 0xFF) as u8;
			data[i * 4 + 2] = ((v >>  0) & 0xFF) as u8;
			data[i * 4 + 3] = match alpha_depth {
				1 => ((alphas[i / 8] >> (i % 8)) & 1) * 0xFF,
				4 => ((alphas[i / 2] >> ((i % 2) * 4)) & 0xF) * 0x11,
				8 => alphas[i],
				_ => 0xFF,
			};
		});
		Ok(Self {
			width,
			height,
			data,
		})
	}
}

impl Texture {
	pub fn read(reader: &mut (impl Read + Seek)) -> Result<Self> {
		let header = Header::read(reader)?;
		if header.version != 1 {
			return Err(Error::InvalidVersion);
		}
		let mipmaps_nb = match header.has_mipmaps {
			0 => 1,
			_ => {
				let mut nb = 0;
				while nb < 16
				   && header.mipmap_offsets[nb] != 0
				   && header.mipmap_lengths[nb] != 0
				   && header.width >> nb != 0
				   && header.height >> nb != 0 {
					nb += 1;
				}
				nb
			}
		};
		let mipmaps = match header.compression {
			1 => Self::read_indexed_mipmaps(reader, &header, mipmaps_nb)?,
			2 => Self::read_s3tc_mipmaps(reader, &header, mipmaps_nb)?,
			3 => Self::read_rgba_mipmaps(reader, &header, mipmaps_nb)?,
			_ => return Err(Error::UnsupportedCompression),
		};
		Ok(Self {
			header,
			mipmaps,
		})
	}

	fn read_indexed_mipmaps(
		reader: &mut (impl Read + Seek),
		header: &Header,
		mipmaps_nb: usize,
	) -> Result<Vec<Mipmap>> {
		(0..mipmaps_nb).map(|x| {
			Mipmap::read_indexed(
				reader,
				(header.width >> x) as usize,
				(header.height >> x) as usize,
				header.mipmap_offsets[x].into(),
				&header.palette,
				header.alpha_depth,
			)
		}).collect()
	}

	fn read_s3tc_mipmaps(
		reader: &mut (impl Read + Seek),
		header: &Header,
		mipmaps_nb: usize,
	) -> Result<Vec<Mipmap>> {
		let mult = match header.alpha_type {
			0 => 8,
			1 => 16,
			7 => 16,
			_ => return Err(Error::InvalidAlphaType),
		};
		(0..mipmaps_nb).map(|x| {
			let width = std::cmp::max(1, header.width >> x) as usize;
			let height = std::cmp::max(1, header.height >> x) as usize;
			let size = ((width + 3) / 4) * ((height + 3) / 4) * mult;
			Mipmap::read(
				reader,
				width,
				height,
				header.mipmap_offsets[x].into(),
				size,
			)
		}).collect()
	}

	fn read_rgba_mipmaps(
		reader: &mut (impl Read + Seek),
		header: &Header,
		mipmaps_nb: usize,
	) -> Result<Vec<Mipmap>> {
		(0..mipmaps_nb).map(|x| {
			Mipmap::read(
				reader,
				(header.width >> x) as usize,
				(header.height >> x) as usize,
				header.mipmap_offsets[x].into(),
				header.mipmap_lengths[x] as usize,
			)
		}).collect()
	}
}
