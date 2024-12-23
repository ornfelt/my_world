use crate::{
	assets::{
		Assets,
		TextureAsset,
	},
	math::{
		Aabb,
		Frustum,
		Vec3,
	},
	world::{
		Chunk,
		Tile,
	},
	wow::{
		Context,
		Frame,
	},
};

use std::{
	sync::{
		Arc,
		Mutex,
	},
	ops::Range,
};

use bytemuck::{
	Pod,
	Zeroable,
};

use wgpu::util::DeviceExt;

use glam::Vec3Swizzles;

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct Vertex {
	position: [f32; 4],
	normal: [i8; 4],
	uv: [f32; 2],
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct SceneBlock {
	mvp: [f32; 16],
	mv: [f32; 16],
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct ModelBlock {
	textures: [[[u32; 4]; 2]; 256],
}

#[derive(Debug)]
struct Batch {
	mesh_binds: Option<wgpu::BindGroup>, /* XXX this shouldn't be a option */
	textures: Vec<u32>,
	indices: [Range<u32>; 4],
	chunks: Vec<u8>,
}

#[derive(Debug)]
pub struct Terrain {
	vertex_buffer: wgpu::Buffer,
	index_buffer: wgpu::Buffer,
	_alpha_map_texture: wgpu::Texture,
	_alpha_map_view: wgpu::TextureView,
	_alpha_map_sampler: wgpu::Sampler,
	_textures_sampler: wgpu::Sampler,
	model_binds: wgpu::BindGroup,
	_model_block: wgpu::Buffer,
	_textures: Mutex<Vec<[Option<TextureAsset>; 2]>>,
	batches: Vec<Batch>,
	aabb: Aabb,
	center: Vec3,
}

#[derive(Debug)]
pub struct Scene {
	scene_binds: wgpu::BindGroup,
	scene_block: wgpu::Buffer,
}

#[derive(Debug)]
pub struct Graphics {
	_shader: wgpu::ShaderModule,
	_layout: wgpu::PipelineLayout,
	pipeline: wgpu::RenderPipeline,
	scene_layout: wgpu::BindGroupLayout,
	model_layout: wgpu::BindGroupLayout,
	mesh_layout: wgpu::BindGroupLayout,
}

impl Vertex {
	fn new(chunk: &libwow::Chunk, i: usize) -> Self {
		let y2 = i % 17;
		let (x, z) = if y2 < 9 {
			(y2 * 2, i / 17 * 2)
		} else {
			((y2 - 9) * 2 + 1, i / 17 * 2 + 1)
		};
		Self {
			position: [
				chunk.mcnk.data.position[0] - z as f32 * (Chunk::WIDTH / 16.0),
				chunk.mcnk.data.position[2] + chunk.mcvt.data[i],
				chunk.mcnk.data.position[1] - x as f32 * (Chunk::WIDTH / 16.0),
				1.0,
			],
			normal: [
				chunk.mcnr.data[i * 3 + 0],
				chunk.mcnr.data[i * 3 + 2],
				chunk.mcnr.data[i * 3 + 1],
				0,
			],
			uv: [x as f32 / 16.0, z as f32 / 16.0],
		}
	}

	fn get_vertex_layout() -> wgpu::VertexBufferLayout<'static> {
		wgpu::VertexBufferLayout {
			array_stride: std::mem::size_of::<Self>() as u64,
			step_mode: wgpu::VertexStepMode::Vertex,
			attributes: &[wgpu::VertexAttribute {
				format: wgpu::VertexFormat::Float32x4,
				offset: 0,
				shader_location: 0,
			}, wgpu::VertexAttribute {
				format: wgpu::VertexFormat::Snorm8x4,
				offset: 4 * 4,
				shader_location: 1,
			}, wgpu::VertexAttribute {
				format: wgpu::VertexFormat::Float32x2,
				offset: 4 * 5,
				shader_location: 2,
			}],
		}
	}
}

impl ModelBlock {
	pub fn new(chunks_batched_textures: &Vec<[u32; 4]>) -> Self {
		let textures = chunks_batched_textures.iter().map(|textures| [
			[
				textures[0] * 2 + 0,
				textures[1] * 2 + 0,
				textures[2] * 2 + 0,
				textures[3] * 2 + 0,
			],
			[
				textures[0] * 2 + 1,
				textures[1] * 2 + 1,
				textures[2] * 2 + 1,
				textures[3] * 2 + 1,
			],
		]).collect::<Vec<[[u32; 4]; 2]>>();
		Self {
			textures: textures.try_into().unwrap(),
		}
	}
}

impl Batch {
	fn new() -> Self {
		Self {
			mesh_binds: None,
			textures: Vec::new(),
			indices: [0..0, 0..0, 0..0, 0..0],
			chunks: Vec::new(),
		}
	}

	fn can_accept_chunk(
		&self,
		max_textures_per_batch: usize,
		layers: usize,
		textures: &[u32; 4],
	) -> bool {
		if self.textures.len() + layers < max_textures_per_batch {
			return true;
		}
		let mut missing_textures = 0;
		for i in 0..layers {
			if !self.textures.contains(&textures[i]) {
				missing_textures += 1;
			}
		}
		self.textures.len() + missing_textures < max_textures_per_batch
	}

	fn add_chunk(
		&mut self,
		chunk: u8,
		layers: usize,
		textures: &[u32; 4],
	) {
		for i in 0..layers as usize {
			if self.textures.iter().position(|&idx| idx == textures[i]) == None {
				self.textures.push(textures[i]);
			}
		}
		self.chunks.push(chunk);
	}

	fn generate_mesh_binds(
		&mut self,
		context: &Context,
		textures: &Vec<[Option<TextureAsset>; 2]>,
		placeholder_texture: &TextureAsset,
	) {
		let used_textures = textures.iter().map(|textures| [
			textures[0].as_ref().or_else(|| Some(placeholder_texture)).unwrap().clone(),
			textures[1].as_ref().or_else(|| Some(placeholder_texture)).unwrap().clone(),
		]).collect::<Vec<[TextureAsset; 2]>>();
		let entries = (0..context.device.limits().max_samplers_per_shader_stage).map(|i| wgpu::BindGroupEntry {
			binding: i as u32,
			resource: wgpu::BindingResource::TextureView(
				&used_textures[*self.textures.get(i as usize / 2).or_else(|| Some(&0)).unwrap() as usize][(i & 1) as usize].texture_view()
			),
		}).collect::<Vec<wgpu::BindGroupEntry<'_>>>();
		self.mesh_binds = Some(context.device.create_bind_group(&wgpu::BindGroupDescriptor {
			label: None,
			layout: &context.terrain.mesh_layout,
			entries: &entries,
		}));
	}
}

impl Terrain {
	pub fn new(
		context: &Arc<Context>,
		assets: &Arc<Assets>,
		max_textures_per_batch: usize,
		placeholder_texture: &TextureAsset,
		tile: &libwow::Tile,
	) -> Self {
		let mut vertexes = Vec::with_capacity((9 * 9 + 8 * 8) * 256);
		let mut indices = Vec::with_capacity((8 * 8) * 256);
		let mut alpha_map = Vec::with_capacity(64 * 64 * 256);
		alpha_map.resize(64 * 64 * 256, [0; 4]);
		let chunks_textures = (0..256).map(|x| {
			let mut textures: [u32; 4] = [0; 4];
			for i in 0..tile.chunks[x].mcnk.data.layers as usize {
				textures[i] = tile.chunks[x].mcly.data[i].texture;
			}
			for i in (tile.chunks[x].mcnk.data.layers as usize)..4 {
				textures[i] = 0;
			}
			textures
		}).collect::<Vec<[u32; 4]>>();
		let mut batches = Vec::<Batch>::new();
		(0..256).for_each(|i| {
			let layers = tile.chunks[i].mcnk.data.layers as usize;
			let batch = match batches.iter_mut().find(|batch| {
				batch.can_accept_chunk(
					max_textures_per_batch,
					layers,
					&chunks_textures[i],
				)
			}) {
				Some(batch) => batch,
				None => {
					batches.push(Batch::new());
					batches.last_mut().unwrap()
				}
			};
			batch.add_chunk(i as u8, layers, &chunks_textures[i]);
		});
		let mut textures = Self::collect_textures(
			context,
			assets,
			&tile.mtex.data,
		);
		if textures.is_empty() {
			textures.push([
				Some(placeholder_texture.clone()),
				Some(placeholder_texture.clone()),
			])
		}
		let mut chunk_id = 0;
		let mut chunks_batched_textures = Vec::with_capacity(256);
		batches.iter_mut().for_each(|batch| {
			batch.chunks.iter().for_each(|&i| {
				Self::collect_chunk_data(
					&tile.chunks[i as usize],
					&batch.textures,
					&mut vertexes,
					&mut alpha_map[(chunk_id as usize * 64 * 64)..((chunk_id as usize + 1) * 64 * 64)],
					&mut chunks_batched_textures,
				);
				chunk_id += 1;
			});
			batch.generate_mesh_binds(
				context,
				&textures,
				placeholder_texture,
			);
		});
		for lod in 0..4 {
			chunk_id = 0;
			batches.iter_mut().for_each(|batch| {
				let first = indices.len() as u32;
				batch.chunks.iter().for_each(|&i| {
					Self::collect_chunk_indices(
						&tile.chunks[i as usize],
						chunk_id * (9 * 9 + 8 * 8),
						i,
						&mut indices,
						lod,
					);
					chunk_id += 1;
				});
				let last = indices.len() as u32;
				batch.indices[lod as usize] = first..last;
			});
		}
		let vertex_buffer = context.device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
			label: None,
			contents: bytemuck::cast_slice(&vertexes),
			usage: wgpu::BufferUsages::VERTEX,
		});
		let index_buffer = context.device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
			label: None,
			contents: bytemuck::cast_slice(&indices),
			usage: wgpu::BufferUsages::INDEX,
		});
		let alpha_map_texture = context.device.create_texture_with_data(&context.queue, &wgpu::TextureDescriptor {
			label: None,
			size: wgpu::Extent3d {
				width: 64,
				height: 64,
				depth_or_array_layers: 256,
			},
			mip_level_count: 1,
			sample_count: 1,
			dimension: wgpu::TextureDimension::D2,
			format: wgpu::TextureFormat::Rgba8Unorm,
			usage: wgpu::TextureUsages::TEXTURE_BINDING,
			view_formats: &[wgpu::TextureFormat::Rgba8Unorm],
		}, bytemuck::cast_slice(&alpha_map));
		let alpha_map_view = alpha_map_texture.create_view(&wgpu::TextureViewDescriptor {
			label: None,
			format: Some(wgpu::TextureFormat::Rgba8Unorm),
			dimension: Some(wgpu::TextureViewDimension::D2Array),
			aspect: wgpu::TextureAspect::All,
			base_mip_level: 0,
			mip_level_count: Some(1),
			base_array_layer: 0,
			array_layer_count: Some(256),
		});
		let alpha_map_sampler = context.device.create_sampler(&wgpu::SamplerDescriptor {
			label: None,
			address_mode_u: wgpu::AddressMode::ClampToEdge,
			address_mode_v: wgpu::AddressMode::ClampToEdge,
			address_mode_w: wgpu::AddressMode::ClampToEdge,
			mag_filter: wgpu::FilterMode::Linear,
			min_filter: wgpu::FilterMode::Linear,
			mipmap_filter: wgpu::FilterMode::Linear,
			lod_min_clamp: 0.0,
			lod_max_clamp: 0.0,
			compare: None,
			anisotropy_clamp: 16,
			border_color: None,
		});
		let textures_sampler = context.device.create_sampler(&wgpu::SamplerDescriptor {
			label: None,
			address_mode_u: wgpu::AddressMode::Repeat,
			address_mode_v: wgpu::AddressMode::Repeat,
			address_mode_w: wgpu::AddressMode::Repeat,
			mag_filter: wgpu::FilterMode::Linear,
			min_filter: wgpu::FilterMode::Linear,
			mipmap_filter: wgpu::FilterMode::Linear,
			lod_min_clamp: 0.0,
			lod_max_clamp: 16.0,
			compare: None,
			anisotropy_clamp: 16,
			border_color: None,
		});
		let model_block = context.device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
			label: None,
			contents: bytemuck::cast_slice(&[ModelBlock::new(&chunks_batched_textures)]),
			usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
		});
		let model_binds = context.device.create_bind_group(&wgpu::BindGroupDescriptor {
			label: None,
			layout: &context.terrain.model_layout,
			entries: &[wgpu::BindGroupEntry {
				binding: 0,
				resource: model_block.as_entire_binding(),
			}, wgpu::BindGroupEntry {
				binding: 1,
				resource: wgpu::BindingResource::Sampler(&alpha_map_sampler),
			}, wgpu::BindGroupEntry {
				binding: 2,
				resource: wgpu::BindingResource::TextureView(&alpha_map_view),
			}, wgpu::BindGroupEntry {
				binding: 3,
				resource: wgpu::BindingResource::Sampler(&textures_sampler),
			}],
		});
		let mut pmin = Vec3::splat(f32::INFINITY);
		let mut pmax = Vec3::splat(-f32::INFINITY);
		for vertex in &vertexes {
			let vec = Vec3::from_slice(&vertex.position[0..3]);
			pmin = pmin.min(vec);
			pmax = pmax.max(vec);
		}
		Self {
			vertex_buffer,
			index_buffer,
			_alpha_map_texture: alpha_map_texture,
			_alpha_map_view: alpha_map_view,
			_alpha_map_sampler: alpha_map_sampler,
			_textures_sampler: textures_sampler,
			model_binds,
			_model_block: model_block,
			_textures: Mutex::new(textures),
			batches,
			aabb: Aabb::new(pmin, pmax),
			center: (pmin + pmax) / 2.0,
		}
	}

	fn collect_chunk_data(
		chunk: &libwow::Chunk,
		batch_textures: &Vec<u32>,
		vertexes: &mut Vec<Vertex>,
		alpha_map: &mut [[u8; 4]],
		chunks_batched_textures: &mut Vec<[u32; 4]>,
	) {
		vertexes.extend((0..9 * 9 + 8 * 8).map(|i| {
			Vertex::new(&chunk, i)
		}));
		let mut textures = [0; 4];
		for i in 0..chunk.mcnk.data.layers as usize {
			textures[i] = batch_textures.iter().position(|&texture| {
				texture == chunk.mcly.data[i].texture
			}).expect("texture not found back in batch") as u32;
		}
		chunks_batched_textures.push(textures);
		for i in 1..chunk.mcnk.data.layers as usize {
			let offset = chunk.mcly.data[i].offset as usize;
			let mcal_data = &chunk.mcal.as_ref().unwrap().data[offset..(offset + 64 * 32)];
			for z in 0..64 {
				for x in 0..64 {
					let (tx, tz) = match chunk.mcnk.data.flags & libwow::adt::MCNK_FLAG_FIX_MCAL {
						0 => match (x, z) {
							(63, 63) => (62, 62),
							(63, z) => (62, z),
							(x, 63) => (x, 62),
							_ => (x, z),
						}
						_ => (x, z),
					};
					let nibble = mcal_data[tz * 32 + tx / 2] >> ((tx & 1) * 4) & 0xF;
					alpha_map[z * 64 + x][i - 1] = (nibble << 4) | nibble;
				}
			}
		}
		if let Some(mcsh) = &chunk.mcsh {
			for z in 0..64 {
				for x in 0..64 {
					alpha_map[z * 64 + x][3] = if mcsh.data[z * 8 + x / 8] & (1 << (x % 8)) != 0 {
						0xFF
					} else {
						0x00
					};
				}
			}
		}
	}

	/// LOD 1: 256
	///  1    2    3    4    5    6    7    8    9
	///    10   11   12   13   14   15   16   17
	///  18   19   20   21   22   23   24   25   26
	///    27   28   29   30   31   32   33   34
	///  35   36   37   38   39   40   41   42   43
	///    44   45   46   47   48   49   50   51
	///  52   53   54   55   56   57   58   59   60
	///    61   62   63   64   65   66   67   68
	///  69   70   71   72   73   74   75   76   77
	///    78   79   80   81   82   83   84   85
	///  86   87   88   89   90   91   92   93   94
	///    95   96   97   98  99  100  101  102
	/// 103  104  105  106  107  108  109  110  111
	///   112  113  114  115  116  117  118  119
	/// 120  121  122  123  124  125  126  127  128
	///   129  130  131  132  133  134  135  136
	/// 137  138  139  140  141  142  143  144  145
	///
	/// LOD 2: 128
	///  1    2    3    4    5    6    7    8    9
	///
	///  18   19   20   21   22   23   24   25   26
	///
	///  35   36   37   38   39   40   41   42   43
	///
	///  52   53   54   55   56   57   58   59   60
	///
	///  69   70   71   72   73   74   75   76   77
	///
	///  86   87   88   89   90   91   92   93   94
	///
	/// 103  104  105  106  107  108  109  110  111
	///
	/// 120  121  122  123  124  125  126  127  128
	///
	/// 137  138  139  140  141  142  143  144  145
	///
	/// LOD 3: 32 to 48 (depends if chunk is at the border)
	///  1    2    3    4    5    6    7    8    9
	///
	///  18                                      26
	///
	///  35        37        39        41        43
	///
	///  52                                      60
	///
	///  69        71        73        75        77
	///
	///  86                                      94
	///
	/// 103       105       107       109       111
	///
	/// 120                                     128
	///
	/// 137  138  139  140  141  142  143  144  145
	///
	/// LOD 4: 8 to 32 (depends if chunk is at the border)
	///  1    2    3    4    5    6    7    8    9
	///
	///  18                                      26
	///
	///  35                                      43
	///
	///  52                                      60
	///
	///  69                  73                  77
	///
	///  86                                      94
	///
	/// 103                                     111
	///
	/// 120                                     128
	///
	/// 137  138  139  140  141  142  143  144  145
	fn collect_chunk_indices(
		chunk: &libwow::Chunk,
		vertex_base: usize,
		chunk_id: u8,
		indices: &mut Vec<u16>,
		lod: i32,
	) {
		match lod {
			0 => for i in 0..8 * 8 {
				let x = i % 8;
				let z = i / 8;
				if chunk.mcnk.data.holes & (1 << (((z & !1) << 1) | (x >> 1))) != 0 {
					continue;
				}
				let p0 = (vertex_base + 9 + z * 17 + x) as u16;
				let p1 = p0 - 9;
				let p2 = p0 - 8;
				let p3 = p0 + 9;
				let p4 = p0 + 8;
				indices.extend_from_slice(&[
					p2, p1, p0,
					p3, p2, p0,
					p4, p3, p0,
					p1, p4, p0,
				]);
			},
			1 => for i in 0..8 * 8 {
				let x = i % 8;
				let z = i / 8;
				if chunk.mcnk.data.holes & (1 << (((z & !1) << 1) | (x >> 1))) != 0 {
					continue;
				}
				let p0 = (vertex_base + 9 + z * 17 + x) as u16;
				let p1 = p0 - 9;
				let p2 = p0 - 8;
				let p3 = p0 + 9;
				let p4 = p0 + 8;
				indices.extend_from_slice(&[
					p2, p1, p4,
					p2, p4, p3,
				]);
			},
			2 => for i in 0..4 * 4 {
				let x = i % 4;
				let z = i / 4;
				if chunk.mcnk.data.holes & (1 << ((z << 2) | x)) != 0 {
					continue;
				}
				let p0 = (vertex_base + 9 + (z * 2) * 17 + (x * 2)) as u16;
				let p00 = p0 - 9;
				let p10 = p0 - 8;
				let p20 = p0 - 7;
				let p01 = p0 + 8;
				let p21 = p0 + 10;
				let p02 = p0 + 25;
				let p12 = p0 + 26;
				let p22 = p0 + 27;
				let cx = chunk_id % 16;
				let cz = chunk_id / 16;
				match (cx, x) {
					(0, 0) => match (cz, z) {
						(0, 0) => indices.extend_from_slice(&[
							p10, p00, p01,
							p20, p10, p22,
							p10, p01, p22,
							p01, p02, p22,
						]),
						(15, 3) => indices.extend_from_slice(&[
							p01, p02, p12,
							p00, p01, p20,
							p01, p12, p20,
							p12, p22, p20,
						]),
						_ => indices.extend_from_slice(&[
							p00, p01, p20,
							p01, p02, p22,
							p20, p01, p22,
						]),
					},
					(15, 3) => match (cz, z) {
						(0, 0) => indices.extend_from_slice(&[
							p10, p21, p20,
							p10, p00, p02,
							p21, p10, p02,
							p22, p21, p02,
						]),
						(15, 3) => indices.extend_from_slice(&[
							p21, p12, p22,
							p21, p20, p00,
							p12, p21, p00,
							p02, p12, p00,
						]),
						_ => indices.extend_from_slice(&[
							p20, p00, p21,
							p02, p22, p21,
							p00, p02, p21,
						]),
					},
					_ => match (cz, z) {
						(0, 0) => indices.extend_from_slice(&[
							p00, p02, p10,
							p22, p20, p10,
							p02, p22, p10,
						]),
						(15, 3) => indices.extend_from_slice(&[
							p00, p02, p12,
							p22, p20, p12,
							p20, p00, p12,
						]),
						_ => indices.extend_from_slice(&[
							p00, p02, p20,
							p20, p02, p22,
						]),
					},
				};
			},
			3 => {
				let p = vertex_base as u16;
				let cx = chunk_id % 16;
				let cz = chunk_id / 16;
				if cx == 0 || cz == 0 {
					indices.extend_from_slice(&[
						p + 51,  p + 68,  p + 72,
						p + 34,  p + 51,  p + 72,
						p + 17,  p + 34,  p + 72,
						p + 0,   p + 17,  p + 72,
						p + 1,   p + 0,   p + 72,
						p + 2,   p + 1,   p + 72,
						p + 3,   p + 2,   p + 72,
						p + 4,   p + 3,   p + 72,
					]);
				} else {
					indices.extend_from_slice(&[
						p + 4, p + 0,  p + 72,
						p + 0, p + 68, p + 72,
					]);
				}
				if cx == 15 || cz == 0 {
					indices.extend_from_slice(&[
						p + 5,   p + 4,   p + 72,
						p + 6,   p + 5,   p + 72,
						p + 7,   p + 6,   p + 72,
						p + 8,   p + 7,   p + 72,
						p + 25,  p + 8,   p + 72,
						p + 42,  p + 25,  p + 72,
						p + 59,  p + 42,  p + 72,
						p + 76,  p + 59,  p + 72,
					]);
				} else {
					indices.extend_from_slice(&[
						p + 8,  p + 4,  p + 72,
						p + 76, p + 8,  p + 72,
					]);
				}
				if cx == 15 || cz == 15 {
					indices.extend_from_slice(&[
						p + 93,  p + 76,  p + 72,
						p + 110, p + 93,  p + 72,
						p + 127, p + 110, p + 72,
						p + 144, p + 127, p + 72,
						p + 143, p + 144, p + 72,
						p + 142, p + 143, p + 72,
						p + 141, p + 142, p + 72,
						p + 140, p + 141, p + 72,
					]);
				} else {
					indices.extend_from_slice(&[
						p + 144, p + 76,  p + 72,
						p + 140, p + 144, p + 72,
					]);
				}
				if cx == 0 || cz == 15 {
					indices.extend_from_slice(&[
						p + 139, p + 140, p + 72,
						p + 138, p + 139, p + 72,
						p + 137, p + 138, p + 72,
						p + 136, p + 137, p + 72,
						p + 119, p + 136, p + 72,
						p + 102, p + 119, p + 72,
						p + 85 , p + 102, p + 72,
						p + 68 , p + 85 , p + 72,
					]);
				} else {
					indices.extend_from_slice(&[
						p + 136, p + 140, p + 72,
						p + 68,  p + 136, p + 72,
					]);
				}
			},
			_ => panic!("invalid lod"),
		}
	}

	fn collect_textures(
		context: &Arc<Context>,
		assets: &Arc<Assets>,
		mtex: &Vec<u8>,
	) -> Vec<[Option<TextureAsset>; 2]> {
		mtex.split(|&x| x == 0).filter(|name| name.len() != 0).map(|name| {
			let diffuse_name = name;
			let specular_name = String::from(
				std::str::from_utf8(
					&name[0..name.len() - 4],
				).expect("invalid texture string"),
			) + "_s.blp";
			let diffuse = assets.get_texture(
				context,
				&libwow::Path::from(diffuse_name),
			);
			let specular = assets.get_texture(
				context,
				&libwow::Path::from(specular_name),
			).or_else(|| diffuse.as_ref().map(|diffuse| diffuse.clone()));
			[
				diffuse,
				specular,
			]
		}).collect()
	}

	pub fn draw<'a>(
		render_pass: &mut wgpu::RenderPass<'a>,
		terrain: &'a Terrain,
		frame: &'a Frame,
	) {
		let dist = (terrain.center.xz() - frame.position.xz()).length();
		let lod = if dist < Tile::WIDTH * 1.0 {
			0
		} else if dist < Tile::WIDTH * 2.0 {
			1
		} else if dist < Tile::WIDTH * 3.0 {
			2
		} else {
			3
		};
		render_pass.set_vertex_buffer(0, terrain.vertex_buffer.slice(..));
		render_pass.set_index_buffer(terrain.index_buffer.slice(..), wgpu::IndexFormat::Uint16);
		render_pass.set_bind_group(1, &terrain.model_binds, &[]);
		terrain.batches.iter().for_each(|batch| {
			render_pass.set_bind_group(2, &batch.mesh_binds.as_ref().unwrap(), &[]);
			render_pass.draw_indexed(batch.indices[lod].clone(), 0, 0..1);
		});
	}

	pub fn cull(&self, frustum: &Frustum) -> bool {
		frustum.test_aabb(&self.aabb)
	}
}

impl Scene {
	pub fn new(context: &Context) -> Self {
		let scene_block = context.device.create_buffer(&wgpu::BufferDescriptor {
			label: None,
			size: std::mem::size_of::<SceneBlock>() as u64,
			usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
			mapped_at_creation: false,
		});
		let scene_binds = context.device.create_bind_group(&wgpu::BindGroupDescriptor {
			label: None,
			layout: &context.terrain.scene_layout,
			entries: &[wgpu::BindGroupEntry {
				binding: 0,
				resource: scene_block.as_entire_binding(),
			}],
		});
		Self {
			scene_block,
			scene_binds,
		}
	}

	pub fn draw<'a>(
		render_pass: &mut wgpu::RenderPass<'a>,
		scene: &'a Scene,
		context: &'a Context,
		frame: &'a Frame,
	) {
		context.queue.write_buffer(
			&scene.scene_block,
			0,
			bytemuck::cast_slice(&[SceneBlock {
				mvp: frame.vp.to_cols_array(),
				mv: frame.v.to_cols_array(),
			}]),
		);
		render_pass.set_pipeline(&context.terrain.pipeline);
		render_pass.set_bind_group(0, &scene.scene_binds, &[]);
		for tile in &frame.terrains {
			Terrain::draw(render_pass, &tile.terrain, frame)
		}
	}
}

impl Graphics {
	pub fn new(device: &wgpu::Device) -> Self {
		let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
			label: None,
			source: wgpu::ShaderSource::Wgsl(include_str!("shaders/terrain.wgsl").into()),
		});
		let scene_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
			label: None,
			entries: &[wgpu::BindGroupLayoutEntry {
				binding: 0,
				visibility: wgpu::ShaderStages::VERTEX,
				ty: wgpu::BindingType::Buffer {
					ty: wgpu::BufferBindingType::Uniform,
					has_dynamic_offset: false,
					min_binding_size: None,
				},
				count: None,
			}],
		});
		let model_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
			label: None,
			entries: &[wgpu::BindGroupLayoutEntry {
				binding: 0,
				visibility: wgpu::ShaderStages::FRAGMENT,
				ty: wgpu::BindingType::Buffer {
					ty: wgpu::BufferBindingType::Uniform,
					has_dynamic_offset: false,
					min_binding_size: None,
				},
				count: None,
			}, wgpu::BindGroupLayoutEntry {
				binding: 1,
				visibility: wgpu::ShaderStages::FRAGMENT,
				ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
				count: None,
			}, wgpu::BindGroupLayoutEntry {
				binding: 2,
				visibility: wgpu::ShaderStages::FRAGMENT,
				ty: wgpu::BindingType::Texture {
					sample_type: wgpu::TextureSampleType::Float {
						filterable: true,
					},
					view_dimension: wgpu::TextureViewDimension::D2Array,
					multisampled: false,
				},
				count: None,
			}, wgpu::BindGroupLayoutEntry {
				binding: 3,
				visibility: wgpu::ShaderStages::FRAGMENT,
				ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
				count: None,
			}],
		});
		let mesh_entries = (0..device.limits().max_samplers_per_shader_stage).map(|x| {
			wgpu::BindGroupLayoutEntry {
				binding: x,
				visibility: wgpu::ShaderStages::FRAGMENT,
				ty: wgpu::BindingType::Texture {
					sample_type: wgpu::TextureSampleType::Float {
						filterable: true,
					},
					view_dimension: wgpu::TextureViewDimension::D2,
					multisampled: false,
				},
				count: None,
			}
		}).collect::<Vec<wgpu::BindGroupLayoutEntry>>();
		let mesh_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
			label: None,
			entries: &mesh_entries,
		});
		let layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
			label: None,
			bind_group_layouts: &[
				&scene_layout,
				&model_layout,
				&mesh_layout,
			],
			push_constant_ranges: &[],
		});
		let pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
			label: None,
			layout: Some(&layout),
			vertex: wgpu::VertexState {
				module: &shader,
				entry_point: "vs_main",
				buffers: &[Vertex::get_vertex_layout()],
			},
			fragment: Some(wgpu::FragmentState {
				module: &shader,
				entry_point: "fs_main",
				targets: &[Some(wgpu::ColorTargetState {
					format: wgpu::TextureFormat::Bgra8Unorm,
					blend: Some(wgpu::BlendState {
						color: wgpu::BlendComponent::REPLACE,
						alpha: wgpu::BlendComponent::REPLACE,
					}),
					write_mask: wgpu::ColorWrites::ALL,
				})],
			}),
			primitive: wgpu::PrimitiveState {
				topology: wgpu::PrimitiveTopology::TriangleList,
				strip_index_format: None,
				front_face: wgpu::FrontFace::Ccw,
				cull_mode: Some(wgpu::Face::Back),
				unclipped_depth: false,
				polygon_mode: wgpu::PolygonMode::Fill,
				conservative: false,
			},
			depth_stencil: Some(wgpu::DepthStencilState {
				format: wgpu::TextureFormat::Depth24PlusStencil8,
				depth_write_enabled: true,
				depth_compare: wgpu::CompareFunction::LessEqual,
				stencil: wgpu::StencilState {
					front: wgpu::StencilFaceState {
						compare: wgpu::CompareFunction::Always,
						fail_op: wgpu::StencilOperation::Keep,
						depth_fail_op: wgpu::StencilOperation::Keep,
						pass_op: wgpu::StencilOperation::Keep,
					},
					back: wgpu::StencilFaceState {
						compare: wgpu::CompareFunction::Always,
						fail_op: wgpu::StencilOperation::Keep,
						depth_fail_op: wgpu::StencilOperation::Keep,
						pass_op: wgpu::StencilOperation::Keep,
					},
					read_mask: 0,
					write_mask: 0,
				},
				bias: wgpu::DepthBiasState {
					constant: 0,
					slope_scale: 0.0,
					clamp: 0.0,
				},
			}),
			multisample: wgpu::MultisampleState {
				count: 1,
				mask: !0,
				alpha_to_coverage_enabled: false,
			},
			multiview: None,
		});
		Self {
			_shader: shader,
			_layout: layout,
			pipeline,
			scene_layout,
			model_layout,
			mesh_layout,
		}
	}
}
