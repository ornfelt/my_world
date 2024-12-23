use super::{
	Result,
	ArrayChunk,
	FlatChunk,
	MVER,
	MWMO,
	MODF,
};

use bytemuck::{
	Pod,
	Zeroable,
};

use std::io::Read;

pub const MPHD_FLAG_WMO:        u32 = 0x00000001;
pub const MPHD_FLAG_MCCV:       u32 = 0x00000002;
pub const MPHD_FLAG_BIG_ALPHA:  u32 = 0x00000004;
pub const MPHD_FLAG_M2_SORT:    u32 = 0x00000008;
pub const MPHD_FLAG_MCLV:       u32 = 0x00000010;
pub const MPHD_FLAG_REVERSE:    u32 = 0x00000020;
pub const MPHD_FLAG_HEIGHT_TEX: u32 = 0x00000080;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MPHDData {
	pub flags: u32,
	pub something: u32,
	pub padding: [u32; 6],
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MAINData {
	pub flags: u32,
	pub padding: u32,
}

pub type MPHD = FlatChunk<0x4D504844, MPHDData>;
pub type MAIN = ArrayChunk<0x4D41494E, MAINData>;

#[derive(Debug)]
pub struct World {
	pub mver: MVER,
	pub mphd: MPHD,
	pub main: MAIN,
	pub mwmo: Option<MWMO>,
	pub modf: Option<MODF>,
}

impl World {
	pub fn read(reader: &mut impl Read) -> Result<Self> {
		let mver = MVER::read(reader)?;
		let mphd = MPHD::read(reader)?;
		let main = MAIN::read(reader)?;
		Ok(Self {
			mver,
			mphd,
			main,
			mwmo: None,
			modf: None,
		})
	}
}
