use super::{
	Result,
	ArrayChunk,
	FlatChunk,
	MVER,
};

use bytemuck::{
	Pod,
	Zeroable,
};

use std::io::Read;

pub const MOHD_FLAG_ATTENUATE_PORTAL_DISTANCE: u16 = 0x0001;
pub const MOHD_FLAG_SKIP_BASE_COLOR:           u16 = 0x0002;
pub const MOHD_FLAG_USE_DBC_LIQUID_TYPE:       u16 = 0x0004;
pub const MOHD_FLAG_LIGHTEN_INTERIORS:         u16 = 0x0008;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOHDData {
	pub textures_nb: u32,
	pub groups_nb: u32,
	pub portals_nb: u32,
	pub light_nb: u32,
	pub models_nb: u32,
	pub doodad_defs_nb: u32,
	pub doodad_sets_nb: u32,
	pub ambient: [u8; 4],
	pub wmo_id: u32,
	pub aabb0: [f32; 3],
	pub aabb1: [f32; 3],
	pub flags: u16,
	pub num_lod: u16,
}

pub const MOMT_FLAG_UNLIT:    u32 = 0x00000001;
pub const MOMT_FLAG_UNFOGGED: u32 = 0x00000002;
pub const MOMT_FLAG_UNCULLED: u32 = 0x00000004;
pub const MOMT_FLAG_EXTLIGHT: u32 = 0x00000008;
pub const MOMT_FLAG_SIDN:     u32 = 0x00000010;
pub const MOMT_FLAG_WINDOW:   u32 = 0x00000020;
pub const MOMT_FLAG_CLAMP_S:  u32 = 0x00000040;
pub const MOMT_FLAG_CLAMP_T:  u32 = 0x00000080;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOMTData {
	pub flags: u32,
	pub shader: u32,
	pub blend_mode: u32,
	pub texture1: u32,
	pub emissive: [u8; 4],
	pub sidn_emissive: [u8; 4],
	pub texture2: u32,
	pub diff_color: u32,
	pub group_type: u32,
	pub texture3: u32,
	pub color2: u32,
	pub flags2: u32,
	pub pad: [u32; 4],
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOGIData {
	pub flags: u32,
	pub aabb0: [f32; 3],
	pub aabb1: [f32; 3],
	pub name_offset: i32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOPTData {
	pub first: u16,
	pub count: u16,
	pub normal: [f32; 3],
	pub distance: f32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOPRData {
	pub portal: u16,
	pub group: u16,
	pub side: i16,
	pub pad: u16,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOVBData {
	pub first: u16,
	pub count: u16,
}

pub const MOLT_LIGHT_OMNI:    u8 = 0x00;
pub const MOLT_LIGHT_SPOT:    u8 = 0x01;
pub const MOLT_LIGHT_DIRECT:  u8 = 0x02;
pub const MOLT_LIGHT_AMBIENT: u8 = 0x03;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOLTData {
	pub light_type: u8,
	pub use_attenuation: u8,
	pub pad: [u8; 2],
	pub color: [u8; 4],
	pub position: [f32; 3],
	pub intensity: f32,
	pub unknown: [f32; 4],
	pub attenuation_start: f32,
	pub attenuation_end: f32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MODSData {
	pub name: [u8; 20],
	pub start: u32,
	pub count: u32,
	pub pad: u32,
}

pub const MODD_FLAG_NAME_MASK: u32 = 0x00FFFFFF;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MODDData {
	pub name_flags: u32,
	pub position: [f32; 3],
	pub rotation: [f32; 4],
	pub scale: f32,
	pub color: [u8; 4],
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MFOGData {
	pub flags: u32,
	pub position: [f32; 3],
	pub small_radius: f32,
	pub large_radius: f32,
	pub fog1_end: f32,
	pub fog1_start: f32,
	pub fog1_color: [u8; 4],
	pub fog2_end: f32,
	pub fog2_start: f32,
	pub fog2_color: [u8; 4],
}

pub type MOHD = FlatChunk<0x4D4F4844, MOHDData>;
pub type MOTX = ArrayChunk<0x4D4F5458, u8>;
pub type MOMT = ArrayChunk<0x4D4F4D54, MOMTData>;
pub type MOGN = ArrayChunk<0x4D4F474E, u8>;
pub type MOGI = ArrayChunk<0x4D4F4749, MOGIData>;
pub type MOSB = ArrayChunk<0x4D4F5342, u8>;
pub type MOPV = ArrayChunk<0x4D4F5056, [f32; 3]>;
pub type MOPT = ArrayChunk<0x4D4F5054, MOPTData>;
pub type MOPR = ArrayChunk<0x4D4F5052, MOPRData>;
pub type MOVV = ArrayChunk<0x4D4F5656, [f32; 3]>;
pub type MOVB = ArrayChunk<0x4D4F5642, MOVBData>;
pub type MOLT = ArrayChunk<0x4D4F4C54, MOLTData>;
pub type MODS = ArrayChunk<0x4D4F4453, MODSData>;
pub type MODN = ArrayChunk<0x4D4F444E, u8>;
pub type MODD = ArrayChunk<0x4D4F4444, MODDData>;
pub type MFOG = ArrayChunk<0x4D464F47, MFOGData>;
pub type MCVP = ArrayChunk<0x4D435650, [f32; 4]>;

#[derive(Debug)]
pub struct MapObj {
	pub mver: MVER,
	pub mohd: MOHD,
	pub motx: MOTX,
	pub momt: MOMT,
	pub mogn: MOGN,
	pub mogi: MOGI,
	pub mosb: MOSB,
	pub mopv: MOPV,
	pub mopt: MOPT,
	pub mopr: MOPR,
	pub movv: MOVV,
	pub movb: MOVB,
	pub molt: MOLT,
	pub mods: MODS,
	pub modn: MODN,
	pub modd: MODD,
	pub mfog: MFOG,
	pub mcvp: Option<MCVP>,
}

impl MapObj {
	pub fn read(reader: &mut impl Read) -> Result<Self> {
		let mver = MVER::read(reader)?;
		let mohd = MOHD::read(reader)?;
		let motx = MOTX::read(reader)?;
		let momt = MOMT::read(reader)?;
		let mogn = MOGN::read(reader)?;
		let mogi = MOGI::read(reader)?;
		let mosb = MOSB::read(reader)?;
		let mopv = MOPV::read(reader)?;
		let mopt = MOPT::read(reader)?;
		let mopr = MOPR::read(reader)?;
		let movv = MOVV::read(reader)?;
		let movb = MOVB::read(reader)?;
		let molt = MOLT::read(reader)?;
		let mods = MODS::read(reader)?;
		let modn = MODN::read(reader)?;
		let modd = MODD::read(reader)?;
		let mfog = MFOG::read(reader)?;
		let mcvp = None;
		Ok(Self {
			mver,
			mohd,
			motx,
			momt,
			mogn,
			mogi,
			mosb,
			mopv,
			mopt,
			mopr,
			movv,
			movb,
			molt,
			mods,
			modn,
			modd,
			mfog,
			mcvp,
		})
	}
}
