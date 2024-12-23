#![allow(dead_code)]

pub mod adt;
pub mod blp;
pub mod m2;
pub mod mpq;
pub mod wdl;
pub mod wdt;
pub mod wmo;
pub mod wmo_group;

pub use adt::*;
pub use blp::*;
pub use m2::*;
pub use mpq::*;
pub use wdl::*;
pub use wdt::*;
pub use wmo::*;
pub use wmo_group::*;

use bytemuck::{
	Pod,
	Zeroable,
};

use std::io::{
	Read,
};

#[derive(Debug)]
pub enum Error {
	IO(std::io::Error),
	Inflate(fdeflate::DecompressionError),
	Path,
	FileNotFound,
	FileDeleted,
	InvalidBlock,
	UnsupportedCompression,
	InvalidVersion,
	InvalidAlphaType,
	InvalidMagic,
	InvalidLength,
}

type Result<T> = std::result::Result<T, Error>;

impl From<std::io::Error> for Error {
	fn from(value: std::io::Error) -> Self {
		Self::IO(value)
	}
}

impl From<fdeflate::DecompressionError> for Error {
	fn from(value: fdeflate::DecompressionError) -> Self {
		Self::Inflate(value)
	}
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct ChunkHeader {
	pub magic: u32,
	pub size: u32,
}

#[derive(Debug)]
pub struct ArrayChunk<const M: u32, T> {
	pub header: ChunkHeader,
	pub data: Vec<T>,
}

#[derive(Debug)]
pub struct FlatChunk<const M: u32, T> {
	pub header: ChunkHeader,
	pub data: T,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MODFData {
	pub name: u32,
	pub id: u32,
	pub position: [f32; 3],
	pub rotation: [f32; 3],
	pub aabb0: [f32; 3],
	pub aabb1: [f32; 3],
	pub flags: u16,
	pub doodad_set: u16,
	pub name_set: u16,
	pub pad: u16,
}

pub type MVER = FlatChunk<0x4D564552, u32>;
pub type MWMO = ArrayChunk<0x4D574D4F, u8>;
pub type MODF = ArrayChunk<0x4D4F4446, MODFData>;

impl ChunkHeader {
	fn read(reader: &mut impl Read) -> Result<Self> {
		let mut header: [Self; 1] = Default::default();
		reader.read_exact(bytemuck::cast_slice_mut(&mut header))?;
		Ok(header[0])
	}

	fn test_magic(&self, magic: u32) -> Result<()> {
		if self.magic != magic {
			println!("magic {} != {}", self.magic, magic);
			return Err(Error::InvalidMagic)
		}
		Ok(())
	}
}

impl<const M: u32, T: std::default::Default + std::clone::Clone + Pod> ArrayChunk<M, T> {
	fn read(reader: &mut impl Read) -> Result<Self> {
		let header = ChunkHeader::read(reader)?;
		header.test_magic(M)?;
		if header.size as usize % std::mem::size_of::<T>() != 0 {
			println!("length {} {} % {} != 0", header.magic, header.size, std::mem::size_of::<T>());
			return Err(Error::InvalidLength)
		}
		let mut data: Vec<T> = vec![Default::default(); header.size as usize / std::mem::size_of::<T>()];
		reader.read_exact(bytemuck::cast_slice_mut(&mut data))?;
		Ok(Self {
			header,
			data,
		})
	}
}

impl<const M: u32, T: std::default::Default + std::clone::Clone + Pod> FlatChunk<M, T> {
	fn read(reader: &mut impl Read) -> Result<Self> {
		let header = ChunkHeader::read(reader)?;
		header.test_magic(M)?;
		/* NB chunks can be more because they encapsulate other ones */
		if (header.size as usize) < std::mem::size_of::<T>() {
			println!("length {} {} != {}", header.magic, header.size, std::mem::size_of::<T>());
			return Err(Error::InvalidLength)
		}
		let mut data: [T; 1] = [Default::default(); 1];
		reader.read_exact(bytemuck::cast_slice_mut(&mut data))?;
		Ok(Self {
			header,
			data: data[0],
		})
	}
}
