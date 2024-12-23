use super::{
	Result,
	ChunkHeader,
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

use std::io::{
	Read,
	Seek,
	SeekFrom,
};

pub const MHDR_FLAG_MFBO:       u32 = 0x00000001;
pub const MHDR_FLAG_NONRTHREND: u32 = 0x00000002;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MHDRData {
	pub flags: u32,
	mcin: u32,
	mtex: u32,
	mmdx: u32,
	mmid: u32,
	mwmo: u32,
	mwid: u32,
	mddf: u32,
	modf: u32,
	mfbo: u32,
	mh2o: u32,
	mtxf: u32,
	unused: [u32; 4],
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MCINData {
	offset: u32,
	size: u32,
	flags: u32,
	async_id: u32,
}

pub const MDDF_FLAG_BIODOME:   u16 = 0x0001;
pub const MDDF_FLAG_SHRUBBERY: u16 = 0x0002;
pub const MDDF_FLAG_LIQUID:    u16 = 0x0010;
pub const MDDF_FLAG_FILEDATA:  u16 = 0x0020;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MDDFData {
	pub name: u32,
	pub id: u32,
	pub position: [f32; 3],
	pub rotation: [f32; 3],
	pub scale: u16,
	pub flags: u16,
}

pub const MCNK_FLAG_MCSH:       u32 = 0x00000001;
pub const MCNK_FLAG_IMPASS:     u32 = 0x00000002;
pub const MCNK_FLAG_LQ_RIVER:   u32 = 0x00000004;
pub const MCNK_FLAG_LQ_OCEAN:   u32 = 0x00000008;
pub const MCNK_FLAG_LQ_MAGMA:   u32 = 0x00000010;
pub const MCNK_FLAG_LQ_SLIME:   u32 = 0x00000020;
pub const MCNK_FLAG_MCCV:       u32 = 0x00000040;
pub const MCNK_FLAG_FIX_MCAL:   u32 = 0x00008000;
pub const MCNK_FLAG_HIGH_HOLES: u32 = 0x00010000;

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MCNKData {
	pub flags: u32,
	pub index_x: u32,
	pub index_y: u32,
	pub layers: u32,
	pub doodad_refs_nb: u32,
	mcvt: u32,
	mcnr: u32,
	mcly: u32,
	mcrf: u32,
	mcal: u32,
	mcal_size: u32,
	mcsh: u32,
	mcsh_size: u32,
	pub area: u32,
	pub map_obj_refs_nb: u32,
	pub holes: u16,
	pad: u16,
	pub low_quality_textures: [u16; 8],
	pub no_effect_doodads: u64,
	mcse: u32,
	mcse_nb: u32,
	mclq: u32,
	mclq_size: u32,
	pub position: [f32; 3],
	mccv: u32,
	mclv: u32,
	pad1: u32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MCLYData {
	pub texture: u32,
	pub flags: u32,
	pub offset: u32,
	pub effect: u32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MCSEData {
	pub sound_point: u32,
	pub sound_name: u32,
	pub position: [f32; 3],
	pub min_distance: f32,
	pub max_distance: f32,
	pub cutoff_distance: f32,
	pub start_time: u16,
	pub end_time: u16,
	pub mode: u16,
	pub group_silence_min: u16,
	pub group_silence_max: u16,
	pub play_instances_min: u16,
	pub play_instances_max: u16,
	pub loop_count_min: u8,
	pub loop_count_max: u8,
	pub inter_sound_gap_min: u16,
	pub inter_sound_gap_max: u16,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MCLQFlow {
	pub position: [f32; 3],
	pub radius: f32,
	pub direction: [f32; 3],
	pub velocity: f32,
	pub amplitude: f32,
	pub frequency: f32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MCLQRiver {
	pub depth: u8,
	pub flow0_pct: i8,
	pub flow1_pct: i8,
	pad: i8,
	pub height: f32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MCLQOcean {
	pub depth: u8,
	pub foam: i8,
	pub wet: i8,
	pad: u8,
	pub height: f32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
pub struct MCLQMagma {
	pub s: u16,
	pub t: u16,
	pub height: f32,
}

pub type MCLQSlime = MCLQMagma;

pub const MCLQ_TILE_LIQUID_OCEAN: u8 = 0x01;
pub const MCLQ_TILE_LIQUID_RIVER: u8 = 0x04;
pub const MCLQ_TILE_LIQUID_MAGMA: u8 = 0x06;
pub const MCLQ_TILE_LIQUID_SLIME: u8 = 0x07;
pub const MCLQ_TILE_HIDDEN:       u8 = 0x08;
pub const MCLQ_TILE_LIQUID_TYPE:  u8 = 0x0F;
pub const MCLQ_TILE_MUST_SWIM:    u8 = 0x40;
pub const MCLQ_TILE_FATIGUE:      u8 = 0x80;

#[derive(Debug)]
pub enum MCLQVert {
	River([MCLQRiver; 9 * 9]),
	Ocean([MCLQOcean; 9 * 9]),
	Magma([MCLQMagma; 9 * 9]),
	Slime([MCLQSlime; 9 * 9]),
}

#[derive(Debug)]
pub struct MCLQData {
	pub min_height: f32,
	pub max_height: f32,
	pub verts: MCLQVert,
	pub tiles: [u8; 64],
	pub nflow: u32,
	pub flows: [MCLQFlow; 2],
}

pub type MHDR = FlatChunk<0x4D484452, MHDRData>;
pub type MCIN = ArrayChunk<0x4D43494E, MCINData>;
pub type MTEX = ArrayChunk<0x4D544558, u8>;
pub type MWID = ArrayChunk<0x4D574944, u32>;
pub type MMDX = ArrayChunk<0x4D4D4458, u8>;
pub type MMID = ArrayChunk<0x4D4D4944, u32>;
pub type MDDF = ArrayChunk<0x4D444446, MDDFData>;
pub type MCNK = FlatChunk<0x4D434E4B, MCNKData>;
pub type MCVT = ArrayChunk<0x4D435654, f32>;
pub type MCNR = ArrayChunk<0x4D434E52, i8>;
pub type MCLY = ArrayChunk<0x4D434C59, MCLYData>;
pub type MCRF = ArrayChunk<0x4D435246, u32>;
pub type MCAL = ArrayChunk<0x4D43414C, u8>;
pub type MCSH = ArrayChunk<0x4D435348, u8>;
pub type MCSE = ArrayChunk<0x4D435345, MCSEData>;
pub type MCLQ = ArrayChunk<0x4D434C51, MCLQData>;

#[derive(Debug)]
pub struct Chunk {
	pub mcnk: MCNK,
	pub mcvt: MCVT,
	pub mcnr: MCNR,
	pub mcly: MCLY,
	pub mcrf: MCRF,
	pub mcal: Option<MCAL>,
	pub mcsh: Option<MCSH>,
	pub mcse: MCSE,
	pub mclq: MCLQ,
}

#[derive(Debug)]
pub struct Tile {
	pub chunks: Vec<Chunk>,
	pub mver: MVER,
	pub mhdr: MHDR,
	pub mcin: MCIN,
	pub mtex: MTEX,
	pub mwmo: MWMO,
	pub mwid: MWID,
	pub modf: MODF,
	pub mmdx: MMDX,
	pub mmid: MMID,
	pub mddf: MDDF,
}

impl MCLQData {
	fn read(reader: &mut impl Read, liquid: u32) -> Result<Self> {
		let mut heights: [f32; 2] = [0.0; 2];
		reader.read_exact(bytemuck::cast_slice_mut(&mut heights))?;
		let verts: MCLQVert = match liquid {
			MCNK_FLAG_LQ_RIVER => {
				let mut water = [MCLQRiver::default(); 9 * 9];
				reader.read_exact(bytemuck::cast_slice_mut(&mut water))?;
				MCLQVert::River(water)
			}
			MCNK_FLAG_LQ_OCEAN => {
				let mut ocean = [MCLQOcean::default(); 9 * 9];
				reader.read_exact(bytemuck::cast_slice_mut(&mut ocean))?;
				MCLQVert::Ocean(ocean)
			}
			MCNK_FLAG_LQ_MAGMA => {
				let mut magma = [MCLQMagma::default(); 9 * 9];
				reader.read_exact(bytemuck::cast_slice_mut(&mut magma))?;
				MCLQVert::Magma(magma)
			}
			MCNK_FLAG_LQ_SLIME => {
				let mut slime = [MCLQMagma::default(); 9 * 9];
				reader.read_exact(bytemuck::cast_slice_mut(&mut slime))?;
				MCLQVert::Slime(slime)
			}
			_ => panic!("invalid liquid"),
		};
		let mut tiles: [u8; 64] = [0; 64];
		reader.read_exact(bytemuck::cast_slice_mut(&mut tiles))?;
		let mut nflow: [u32; 1] = [0; 1];
		reader.read_exact(bytemuck::cast_slice_mut(&mut nflow))?;
		let mut flows: [MCLQFlow; 2] = [Default::default(); 2];
		reader.read_exact(bytemuck::cast_slice_mut(&mut flows))?;
		Ok(Self {
			min_height: heights[0],
			max_height: heights[1],
			verts,
			tiles,
			nflow: nflow[0],
			flows,
		})
	}
}

impl MCLQ {
	fn read(reader: &mut impl Read, mcnk_flags: u32) -> Result<Self> {
		let header = ChunkHeader::read(reader)?;
		header.test_magic(0x4D434C51)?;
		let liquids = [
			MCNK_FLAG_LQ_RIVER,
			MCNK_FLAG_LQ_OCEAN,
			MCNK_FLAG_LQ_MAGMA,
			MCNK_FLAG_LQ_SLIME,
		];
		let data = liquids.iter().filter(|liquid| {
			mcnk_flags & *liquid != 0
		}).map(|liquid| {
			MCLQData::read(reader, *liquid)
		}).collect::<Result<Vec<MCLQData>>>()?;
		Ok(Self {
			header,
			data,
		})
	}
}

impl Chunk {
	fn read(reader: &mut (impl Read + Seek), mcin: &MCINData) -> Result<Self> {
		reader.seek(SeekFrom::Start(mcin.offset.into()))?;
		let mcnk = MCNK::read(reader)?;
		reader.seek(SeekFrom::Start((mcin.offset + mcnk.data.mcvt).into()))?;
		let mcvt = MCVT::read(reader)?;
		reader.seek(SeekFrom::Start((mcin.offset + mcnk.data.mcnr).into()))?;
		let mcnr = MCNR::read(reader)?;
		reader.seek(SeekFrom::Start((mcin.offset + mcnk.data.mcly).into()))?;
		let mcly = MCLY::read(reader)?;
		reader.seek(SeekFrom::Start((mcin.offset + mcnk.data.mcrf).into()))?;
		let mcrf = MCRF::read(reader)?;
		reader.seek(SeekFrom::Start((mcin.offset + mcnk.data.mcal).into()))?;
		let mcal = match mcnk.data.layers {
			0 => None,
			_ => Some(MCAL::read(reader)?),
		};
		reader.seek(SeekFrom::Start((mcin.offset + mcnk.data.mcsh).into()))?;
		let mcsh = match mcnk.data.flags & MCNK_FLAG_MCSH {
			MCNK_FLAG_MCSH => Some(MCSH::read(reader)?),
			_ => None,
		};
		reader.seek(SeekFrom::Start((mcin.offset + mcnk.data.mcse).into()))?;
		let mcse = MCSE::read(reader)?;
		reader.seek(SeekFrom::Start((mcin.offset + mcnk.data.mclq).into()))?;
		let mclq = MCLQ::read(reader, mcnk.data.flags)?;
		Ok(Self {
			mcnk,
			mcvt,
			mcnr,
			mcly,
			mcrf,
			mcal,
			mcsh,
			mcse,
			mclq,
		})
	}
}

impl Tile {
	pub fn read(reader: &mut (impl Read + Seek)) -> Result<Self> {
		let mver = MVER::read(reader)?;
		let mhdr = MHDR::read(reader)?;
		let mhdr_base = (std::mem::size_of::<MVER>() + std::mem::size_of::<ChunkHeader>()) as u64;
		reader.seek(SeekFrom::Start(mhdr_base + mhdr.data.mcin as u64))?;
		let mcin = MCIN::read(reader)?;
		reader.seek(SeekFrom::Start(mhdr_base + mhdr.data.mtex as u64))?;
		let mtex = MTEX::read(reader)?;
		reader.seek(SeekFrom::Start(mhdr_base + mhdr.data.mwmo as u64))?;
		let mwmo = MWMO::read(reader)?;
		reader.seek(SeekFrom::Start(mhdr_base + mhdr.data.mwid as u64))?;
		let mwid = MWID::read(reader)?;
		reader.seek(SeekFrom::Start(mhdr_base + mhdr.data.modf as u64))?;
		let modf = MODF::read(reader)?;
		reader.seek(SeekFrom::Start(mhdr_base + mhdr.data.mmdx as u64))?;
		let mmdx = MMDX::read(reader)?;
		reader.seek(SeekFrom::Start(mhdr_base + mhdr.data.mmid as u64))?;
		let mmid = MMID::read(reader)?;
		reader.seek(SeekFrom::Start(mhdr_base + mhdr.data.mddf as u64))?;
		let mddf = MDDF::read(reader)?;
		let chunks = (0..256).map(|i| {
			Chunk::read(reader, &mcin.data[i])
		}).collect::<Result<Vec<Chunk>>>()?;
		Ok(Self {
			chunks,
			mver,
			mhdr,
			mcin,
			mtex,
			mwmo,
			mwid,
			modf,
			mmdx,
			mmid,
			mddf,
		})
	}
}
