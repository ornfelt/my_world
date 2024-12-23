use super::{
	Result,
	ArrayChunk,
	FlatChunk,
	MVER,
	adt::MCLQVert,
};

use bytemuck::{
	Pod,
	Zeroable,
};

use std::io::Read;

pub const MOGP_FLAG_BSP:         u32 = 0x00000001;
pub const MOGP_FLAG_LIGHT_MAP:   u32 = 0x00000002;
pub const MOGP_FLAG_COLOR:       u32 = 0x00000004;
pub const MOGP_FLAG_OUTDOOR:     u32 = 0x00000008;
pub const MOGP_FLAG_EXT_LIGHT:   u32 = 0x00000040;
pub const MOGP_FLAG_EXT_SKYBOX:  u32 = 0x00000100;
pub const MOGP_FLAG_LIGHT:       u32 = 0x00000200;
pub const MOGP_FLAG_DOODAD:      u32 = 0x00000800;
pub const MOGP_FLAG_LIQUID:      u32 = 0x00001000;
pub const MOGP_FLAG_INDOOR:      u32 = 0x00002000;
pub const MOGP_FLAG_ALWAYS_DRAW: u32 = 0x00010000;
pub const MOGP_FLAG_MORI_MORB:   u32 = 0x00020000;
pub const MOGP_FLAG_SKYBOX:      u32 = 0x00040000;
pub const MOGP_FLAG_OCEAN:       u32 = 0x00080000;
pub const MOGP_FLAG_MOUNT:       u32 = 0x00200000;
pub const MOGP_FLAG_COLOR2:      u32 = 0x01000000;
pub const MOGP_FLAG_MOTV2:       u32 = 0x02000000;
pub const MOGP_FLAG_ANTIPORTAL:  u32 = 0x04000000;
pub const MOGP_FLAG_MOTV3:       u32 = 0x40000000;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOGPData {
	pub name: u32,
	pub desc_name: u32,
	pub flags: u32,
	pub aabb0: [f32; 3],
	pub aabb1: [f32; 3],
	pub portal_start: u16,
	pub portal_end: u16,
	pub trans_batch_count: u16,
	pub ext_batch_count: u16,
	pub int_batch_count: u16,
	pub padding: u16,
	pub fogs: [u8; 4],
	pub liquid: u32,
	pub id: u32,
	pub flags2: u32,
	pub unk: u32,
}

pub const MOPY_FLAG_TRANSPARENT:  u8 = 0x01;
pub const MOPY_FLAG_NOCAMCOLLIDE: u8 = 0x02;
pub const MOPY_FLAG_DETAIL:       u8 = 0x04;
pub const MOPY_FLAG_COLLISION:    u8 = 0x08;
pub const MOPY_FLAG_HINT:         u8 = 0x10;
pub const MOPY_FLAG_RENDER:       u8 = 0x20;
pub const MOPY_FLAG_COLLIDE_HIT:  u8 = 0x80;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOPYData {
	pub flags: u8,
	pub material: u8,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOBAData {
	pub aabb0: [i16; 3],
	pub aabb1: [i16; 3],
	pub start: u32,
	pub count: u16,
	pub min: u16,
	pub max: u16,
	pub flags: u8,
	pub material: u8,
}

pub const MOBN_FLAG_AXIS_X:    u16 = 0x0000;
pub const MOBN_FLAG_AXIS_Y:    u16 = 0x0001;
pub const MOBN_FLAG_AXIS_Z:    u16 = 0x0002;
pub const MOBN_FLAG_AXIS_MASK: u16 = 0x0003;
pub const MOBN_FLAG_LEAF:      u16 = 0x0004;
pub const MOBN_FLAG_NO_CHILD:  u16 = 0xFFFF;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MOBNData {
	pub flags: u16,
	pub neg_child: i16,
	pub pos_child: i16,
	pub faces: u16,
	pub count: u32,
	pub dist: f32,
}

pub type MLIQVert = MCLQVert;

#[derive(Debug)]
pub struct MLIQData {
	pub xverts: u32,
	pub yverts: u32,
	pub verts: Vec<MLIQVert>,
	pub xtiles: u32,
	pub ytiles: u32,
	pub tiles: Vec<u8>,
	pub position: [f32; 3],
	pub material: u16,
}

pub type MOGP = FlatChunk<0x4D4F4750, MOGPData>;
pub type MOPY = ArrayChunk<0x4D4F5059, MOPYData>;
pub type MOVI = ArrayChunk<0x4D4F5649, u16>;
pub type MOVT = ArrayChunk<0x4D4F5654, [f32; 3]>;
pub type MONR = ArrayChunk<0x4D4F4E52, [f32; 3]>;
pub type MOTV = ArrayChunk<0x4D4F5456, [f32; 2]>;
pub type MOBA = ArrayChunk<0x4D4F4241, MOBAData>;
pub type MOLR = ArrayChunk<0x4D4F4C52, u16>;
pub type MODR = ArrayChunk<0x4D4F4452, u16>;
pub type MOBN = ArrayChunk<0x4D4F424E, MOBNData>;
pub type MOBR = ArrayChunk<0x4D4F4252, u16>;
pub type MOCV = ArrayChunk<0x4D4F4356, [u8; 4]>;
pub type MLIQ = ArrayChunk<0x4D4C4951, u8>; /* XXX */

#[derive(Debug)]
pub struct MapObjGroup {
	pub mver: MVER,
	pub mogp: MOGP,
	pub mopy: MOPY,
	pub movi: MOVI,
	pub movt: MOVT,
	pub monr: MONR,
	pub motv: MOTV,
	pub moba: MOBA,
	pub molr: Option<MOLR>,
	pub modr: Option<MODR>,
	pub mobn: Option<MOBN>,
	pub mobr: Option<MOBR>,
	pub mocv: Option<MOCV>,
	pub mliq: Option<MLIQ>,
}

impl MapObjGroup {
	pub fn read(reader: &mut impl Read) -> Result<Self> {
		let mver = MVER::read(reader)?;
		let mogp = MOGP::read(reader)?;
		let mopy = MOPY::read(reader)?;
		let movi = MOVI::read(reader)?;
		let movt = MOVT::read(reader)?;
		let monr = MONR::read(reader)?;
		let motv = MOTV::read(reader)?;
		let moba = MOBA::read(reader)?;
		let molr = match mogp.data.flags & MOGP_FLAG_LIGHT {
			MOGP_FLAG_LIGHT => Some(MOLR::read(reader)?),
			_ => None,
		};
		let modr = match mogp.data.flags & MOGP_FLAG_DOODAD {
			MOGP_FLAG_DOODAD => Some(MODR::read(reader)?),
			_ => None,
		};
		let mobn = match mogp.data.flags & MOGP_FLAG_BSP {
			MOGP_FLAG_BSP => Some(MOBN::read(reader)?),
			_ => None,
		};
		let mobr = match mogp.data.flags & MOGP_FLAG_BSP {
			MOGP_FLAG_BSP => Some(MOBR::read(reader)?),
			_ => None,
		};
		let mocv = match mogp.data.flags & MOGP_FLAG_COLOR {
			MOGP_FLAG_COLOR => Some(MOCV::read(reader)?),
			_ => None,
		};
		let mliq = match mogp.data.flags & MOGP_FLAG_LIQUID {
			MOGP_FLAG_LIQUID => Some(MLIQ::read(reader)?),
			_ => None,
		};
		Ok(Self {
			mver,
			mogp,
			mopy,
			movi,
			movt,
			monr,
			motv,
			moba,
			molr,
			modr,
			mobn,
			mobr,
			mocv,
			mliq,
		})
	}
}
