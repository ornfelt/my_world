use super::{
	Result,
	ArrayChunk,
	MVER,
};

use std::io::Read;

pub type MAOF = ArrayChunk<0x4D414F46, u32>;
pub type MARE = ArrayChunk<0x4D415245, i16>;

#[derive(Debug)]
pub struct LowResMap {
	pub mver: MVER,
	pub maof: MAOF,
	pub mare: MARE,
}

impl LowResMap {
	pub fn read(reader: &mut impl Read) -> Result<Self> {
		let mver = MVER::read(reader)?;
		let maof = MAOF::read(reader)?;
		let mare = MARE::read(reader)?;
		Ok(Self {
			mver,
			maof,
			mare,
		})
	}
}
