use super::{
	Error,
	Result,
};

use bytemuck::{
	Pod,
	Zeroable,
};

use std::{
	fs,
	io::{
		Cursor,
		Read,
		SeekFrom,
		Seek,
	},
	sync::Mutex,
};

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
struct Header {
	magic: u32,
	header_size: u32,
	archive_size: u32,
	format_version: u16,
	block_size: u16,
	hash_table_pos: u32,
	block_table_pos: u32,
	hash_table_size: u32,
	block_table_size: u32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
struct Header2 {
	ext_block_table_pos_low: u32,
	ext_block_table_pos_hi: u32,
	hi_hash_table_pos: u16,
	hi_block_table_pos: u16,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
struct Block {
	offset: u32,
	block_size: u32,
	file_size: u32,
	flags: u32,
}

#[repr(C)]
#[derive(Default, Copy, Clone, Debug, Pod, Zeroable)]
struct Hash {
	name_a: u32,
	name_b: u32,
	locale: u16,
	platform: u16,
	block: u32,
}

#[derive(Debug)]
pub struct File {
	pub data: Cursor<Vec<u8>>,
}

#[derive(Debug)]
pub struct Archive {
	file: Mutex<fs::File>,
	header: Header,
	header2: Option<Header2>,
	blocks: Vec<Block>,
	hashes: Vec<Hash>,
	crypt_table: [u32; 0x500],
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Path {
	pub value: String,
}

const CRYPT_OFFSET_HASH_BUCKET:  u32  = 0x00000000;
const CRYPT_OFFSET_HASH_NAME_A:  u32  = 0x00000100;
const CRYPT_OFFSET_HASH_NAME_B:  u32  = 0x00000200;
const CRYPT_OFFSET_HASH_FILE_KEY:u32  = 0x00000300;
const CRYPT_OFFSET_DECRYPT_TABLE:u32  = 0x00000400;

const KEY_HASH_TABLE:  u32 = 0xC3AF3770; /* hash("(hash table)" , WOW_MPQ_CRYPT_OFFSET_HASH_FILE_KEY) */
const KEY_BLOCK_TABLE: u32 = 0xEC83B3A3; /* hash("(block table)", WOW_MPQ_CRYPT_OFFSET_HASH_FILE_KEY) */

const BLOCK_IMPLODE:       u32 = 0x00000100;
const BLOCK_COMPRESS:      u32 = 0x00000200;
const BLOCK_ENCRYPTED:     u32 = 0x00010000;
const BLOCK_FIX_KEY:       u32 = 0x00020000;
const BLOCK_PATCH_FILE:    u32 = 0x00100000;
const BLOCK_DELETE_MARKER: u32 = 0x02000000;
const BLOCK_SINGLE_UNIT:   u32 = 0x01000000;
const BLOCK_SECTOR_CRC:    u32 = 0x04000000;
const BLOCK_EXISTS:        u32 = 0x80000000;

const COMPRESSION_NONE:   u8 = 0x00;
const COMPRESSION_ZLIB:   u8 = 0x02;
const COMPRESSION_PKWARE: u8 = 0x08;
const COMPRESSION_BZIP2:  u8 = 0x10;
const COMPRESSION_SPARSE: u8 = 0x20;
const COMPRESSION_LZMA:   u8 = 0x12;

fn reader_read(reader: &mut impl Read, data: &mut [u8]) -> Result<()> {
	Ok(reader.read_exact(data)?)
}

fn reader_seek(reader: &mut impl Seek, off: u64) -> Result<()> {
	reader.seek(SeekFrom::Start(off))?;
	Ok(())
}

fn reader_read_at(
	reader: &mut (impl Read + Seek),
	data: &mut [u8],
	off: u64,
) -> Result<()> {
	reader_seek(reader, off)?;
	Ok(reader_read(reader, data)?)
}

impl Header {
	fn read(reader: &mut impl Read) -> Result<Self> {
		let mut header: [Self; 1] = Default::default();
		reader.read_exact(bytemuck::cast_slice_mut(&mut header))?;
		Ok(header[0])
	}
}

impl Header2 {
	fn read(reader: &mut impl Read) -> Result<Self> {
		let mut header: [Self; 1] = Default::default();
		reader.read_exact(bytemuck::cast_slice_mut(&mut header))?;
		Ok(header[0])
	}
}

impl File {
	fn from_block(archive: &Archive, block: &Block) -> Result<Self> {
		let max_sector_size = (512 << archive.header.block_size) as usize;
		let (sectors, sectors_nb) = Self::read_sectors(
			block,
			&archive.file,
			max_sector_size,
		)?;
		let mut data = vec![0; block.file_size as usize];
		let mut size: usize = 0;
		for i in 0..sectors_nb {
			let offset = block.offset + sectors[i];
			let in_size = sectors[i + 1] - sectors[i];
			let mut out_size = block.file_size as usize - size;
			if out_size > max_sector_size {
				out_size = max_sector_size;
			}
			Self::read_sector(
				block,
				offset.into(),
				in_size as usize,
				out_size as usize,
				&archive.file,
				&mut data,
				&mut size,
			)?;
			if size >= block.file_size as usize {
				break;
			}
		}
		Ok(Self {
			data: std::io::Cursor::new(data),
		})
	}

	fn read_sectors(
		block: &Block,
		file: &Mutex<impl Read + Seek>,
		max_sector_size: usize,
	) -> Result<(Vec<u32>, usize)> {
		let mut file_locked = file.lock().unwrap();
		let sectors_nb = (block.file_size as usize + max_sector_size - 1) / max_sector_size;
		let mut sectors: Vec<u32> = vec![0; 1 + sectors_nb];
		reader_read_at(
			&mut *file_locked,
			bytemuck::cast_slice_mut(&mut sectors),
			block.offset.into(),
		)?;
		Ok((sectors, sectors_nb))
	}

	fn read_sector(
		block: &Block,
		offset: u64,
		mut in_size: usize,
		out_size: usize,
		file: &Mutex<impl Read + Seek>,
		data: &mut Vec<u8>,
		size: &mut usize,
	) -> Result<()> {
		/* XXX check for CRC */
		let mut compression: [u8; 1] = [0; 1];
		let mut buf: [u8; 4096] = [0; 4096];
		/* XXX it's a bit sad code because of locking policy
		 * but it's not *that* dramatic
		 */
		{
			let mut file_locked = file.lock().unwrap();
			reader_seek(&mut *file_locked, offset)?;
			if (block.flags & BLOCK_COMPRESS) != 0 && in_size < out_size {
				in_size -= 1;
				reader_read(&mut *file_locked, bytemuck::cast_slice_mut(&mut compression))?;
			}
			match compression[0] {
				COMPRESSION_NONE => {
					let old_size = *size;
					*size += out_size;
					reader_read(&mut *file_locked, &mut data[old_size..*size])
				}
				COMPRESSION_ZLIB => {
					reader_read(&mut *file_locked, &mut buf[0..in_size])
				}
				_ => return Err(Error::UnsupportedCompression)
			}?;
		}
		match compression[0] {
			COMPRESSION_ZLIB => match fdeflate::Decompressor::new().read(&mut buf[0..in_size], data, *size, true) {
				Ok((_r, w)) => {
					*size += w;
					Ok(())
				}
				Err(e) => Err(Error::Inflate(e))
			}
			_ => Ok(())
		}
	}
}

impl Seek for File {
	fn seek(&mut self, pos: SeekFrom) -> std::io::Result<u64> {
		self.data.seek(pos)
	}
}

impl Read for File {
	fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
		self.data.read(buf)
	}
}

impl Archive {
	pub fn open(path: &std::path::Path) -> Result<Self> {
		let mut file = std::fs::OpenOptions::new().read(true).open(path)?;
		let header = Header::read(&mut file)?;
		if header.magic != 0x1A51504D {
			return Err(Error::InvalidMagic);
		}
		let header2 = if header.format_version >= 1 {
			Some(Header2::read(&mut file)?)
		} else {
			None
		};

		let crypt_table = Self::gen_crypt_table();

		/* XXX some sort of file mapping ? */
		let mut blocks = vec![Block::default(); header.block_table_size as usize];
		reader_read_at(
			&mut file,
			bytemuck::cast_slice_mut(&mut blocks.as_mut_slice()),
			header.block_table_pos.into(),
		)?;
		Self::decrypt_table(
			crypt_table,
			bytemuck::cast_slice_mut(&mut blocks.as_mut_slice()),
			KEY_BLOCK_TABLE,
		);

		/* XXX some sort of file mapping ? */
		let mut hashes = vec![Hash::default(); header.hash_table_size as usize];
		reader_read_at(
			&mut file,
			bytemuck::cast_slice_mut(&mut hashes.as_mut_slice()),
			header.hash_table_pos.into(),
		)?;
		Self::decrypt_table(
			crypt_table,
			bytemuck::cast_slice_mut(&mut hashes.as_mut_slice()),
			KEY_HASH_TABLE,
		);

		Ok(Self {
			file: Mutex::new(file),
			header,
			header2,
			blocks,
			hashes,
			crypt_table,
		})
	}

	fn read(&self, data: &mut [u8]) -> Result<()> {
		reader_read(&mut *self.file.lock().unwrap(), data)
	}

	fn seek(&self, off: u64) -> Result<()> {
		reader_seek(&mut *self.file.lock().unwrap(), off)
	}

	fn read_at(&self, data: &mut [u8], off: u64) -> Result<()> {
		reader_read_at(&mut *self.file.lock().unwrap(), data, off)
	}

	fn gen_crypt_table() -> [u32; 0x500] {
		let mut crypt_table: [u32; 0x500] = [0; 0x500];
		let mut seed: u32 = 0x00100001;
		for i in 0..0x100 {
			for j in 0..=4 {
				seed = (seed.wrapping_mul(125).wrapping_add(3)) % 0x2AAAAB;
				let tmp1 = (seed & 0xFFFF) << 0x10;
				seed = (seed.wrapping_mul(125).wrapping_add(3)) % 0x2AAAAB;
				let tmp2 = seed & 0xFFFF;
				crypt_table[i + 0x100 * j] = tmp1 | tmp2;
			}
		}
		crypt_table
	}

	fn decrypt_table(crypt_table: [u32; 0x500], data: &mut [u32], mut key: u32) {
		let mut seed: u32 = 0xEEEEEEEE;
		data.iter_mut().for_each(|v| {
			seed = seed.wrapping_add(crypt_table[(CRYPT_OFFSET_DECRYPT_TABLE + (key & 0xFF)) as usize]);
			let ch = *v ^ key.wrapping_add(seed);
			*v = ch;
			key = ((!key << 0x15).wrapping_add(0x11111111)) | (key >> 0x0B);
			seed = seed.wrapping_add(ch).wrapping_add(seed << 5).wrapping_add(3);
		})
	}

	fn hash_path(&self, path: &Path, hash_type: u32) -> Result<u32> {
		let mut seed1: u32 = 0x7FED7FED;
		let mut seed2: u32 = 0xEEEEEEEE;
		path.value.as_bytes().iter().for_each(|c| {
			seed1 = self.crypt_table[hash_type.wrapping_add(*c as u32) as usize] ^ (seed1.wrapping_add(seed2));
			seed2 = (*c as u32).wrapping_add(seed1).wrapping_add(seed2).wrapping_add(seed2 << 5).wrapping_add(3);
		});
		Ok(seed1)
	}

	fn get_block(&self, bucket: u32, name_a: u32, name_b: u32) -> Result<Block> {
		let start_hash = bucket & (self.header.hash_table_size - 1);
		for i in start_hash..self.header.hash_table_size {
			let hash = &self.hashes[i as usize];
			if hash.name_a == name_a && hash.name_b == name_b {
				let block_index = hash.block;
				if block_index >= self.header.block_table_size {
					return Err(Error::FileNotFound)
				}
				let block = &self.blocks[block_index as usize];
				if block.flags & BLOCK_DELETE_MARKER != 0 {
					return Err(Error::FileDeleted)
				}
				return Ok(*block)
			}
			if hash.block == 0xFFFFFFFF {
				return Err(Error::FileNotFound)
			}
		}
		Err(Error::FileNotFound)
	}

	pub fn get_file(&self, path: &Path) -> Result<File> {
		let bucket = self.hash_path(path, CRYPT_OFFSET_HASH_BUCKET)?;
		let name_a = self.hash_path(path, CRYPT_OFFSET_HASH_NAME_A)?;
		let name_b = self.hash_path(path, CRYPT_OFFSET_HASH_NAME_B)?;
		let block = self.get_block(bucket, name_a, name_b)?;
		File::from_block(self, &block)
	}
}

impl Path {
	pub fn new(value: String) -> Self {
		Self {
			value
		}
	}
}

impl From<String> for Path {
	fn from(data: String) -> Self {
		Self {
			value: data.to_uppercase(),
		}
	}
}

impl From<&str> for Path {
	fn from(data: &str) -> Self {
		Self::from(String::from(data))
	}
}

impl From<&[u8]> for Path {
	fn from(data: &[u8]) -> Self {
		Self::from(std::str::from_utf8(data).expect("invalid path encoding"))
	}
}
