use crate::{
	assets::TextureAsset,
	math::{
		Aabb,
		Frustum,
		Vec3,
	},
	world::Chunk,
	wow::{
		Context,
		Frame,
	},
};

use std::{
	sync::{
		Arc,
	},
};

use bytemuck::{
	Pod,
	Zeroable,
};

use wgpu::util::DeviceExt;

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct WaterVertex {
	position: [f32; 4],
	uv: [f32; 2],
	depth: f32,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct WaterSceneBlock {
	mvp: [f32; 16],
	mv: [f32; 16],
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct MagmaVertex {
	position: [f32; 4],
	uv: [f32; 2],
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct MagmaSceneBlock {
	mvp: [f32; 16],
	mv: [f32; 16],
}

#[derive(Debug)]
pub struct Liquid {
	vertex_buffer: wgpu::Buffer,
	index_buffer: wgpu::Buffer,
	aabb: Aabb,
	indices_nb: u32,
}

#[derive(Debug)]
pub struct WaterScene {
	scene_binds: Vec<wgpu::BindGroup>,
	scene_block: wgpu::Buffer,
}

#[derive(Debug)]
pub struct MagmaScene {
	scene_binds: Vec<wgpu::BindGroup>,
	scene_block: wgpu::Buffer,
}

#[derive(Debug)]
pub struct WaterGraphics {
	_shader: wgpu::ShaderModule,
	_layout: wgpu::PipelineLayout,
	pipeline: wgpu::RenderPipeline,
	scene_layout: wgpu::BindGroupLayout,
}

#[derive(Debug)]
pub struct MagmaGraphics {
	_shader: wgpu::ShaderModule,
	_layout: wgpu::PipelineLayout,
	pipeline: wgpu::RenderPipeline,
	scene_layout: wgpu::BindGroupLayout,
}

impl WaterVertex {
	fn new(
		chunk: &libwow::Chunk,
		height: f32,
		depth: u8,
		i: usize,
	) -> Self {
		let x = i % 9;
		let z = i / 9;
		Self {
			position: [
				chunk.mcnk.data.position[0] - z as f32 * (Chunk::WIDTH / 8.0),
				height,
				chunk.mcnk.data.position[1] - x as f32 * (Chunk::WIDTH / 8.0),
				1.0,
			],
			uv: [x as f32 / 2.0, z as f32 / 2.0],
			depth: depth as f32 / 255.0,
		}
	}

	fn river(
		chunk: &libwow::Chunk,
		river: &libwow::MCLQRiver,
		i: usize,
	) -> Self {
		Self::new(chunk, river.height, river.depth, i)
	}

	fn ocean(
		chunk: &libwow::Chunk,
		ocean: &libwow::MCLQOcean,
		i: usize,
	) -> Self {
		Self::new(chunk, ocean.height, ocean.depth, i)
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
				format: wgpu::VertexFormat::Float32x2,
				offset: 4 * 4,
				shader_location: 1,
			}, wgpu::VertexAttribute {
				format: wgpu::VertexFormat::Float32,
				offset: 4 * 6,
				shader_location: 2,
			}],
		}
	}
}

impl MagmaVertex {
	fn new(
		chunk: &libwow::Chunk,
		height: f32,
		s: u16,
		t: u16,
		i: usize,
	) -> Self {
		let x = i % 9;
		let z = i / 9;
		Self {
			position: [
				chunk.mcnk.data.position[0] - z as f32 * (Chunk::WIDTH / 8.0),
				height,
				chunk.mcnk.data.position[1] - x as f32 * (Chunk::WIDTH / 8.0),
				1.0,
			],
			uv: [
				s as f32 / 80.0,
				t as f32 / 80.0,
			],
		}
	}

	fn magma(
		chunk: &libwow::Chunk,
		magma: &libwow::MCLQMagma,
		i: usize,
	) -> Self {
		Self::new(chunk, magma.height, magma.s, magma.t, i)
	}

	fn slime(
		chunk: &libwow::Chunk,
		slime: &libwow::MCLQSlime,
		i: usize,
	) -> Self {
		Self::new(chunk, slime.height, slime.s, slime.t, i)
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
				format: wgpu::VertexFormat::Float32x2,
				offset: 4 * 4,
				shader_location: 1,
			}],
		}
	}
}

impl Liquid {
	fn tile(
		context: &Arc<Context>,
		vertexes: &Vec<impl Pod>,
		indices: &Vec<u16>,
		aabb: Aabb,
	) -> Option<Box<Self>> {
		if indices.is_empty() {
			return None;
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
		Some(Box::new(Self {
			vertex_buffer,
			index_buffer,
			aabb,
			indices_nb: indices.len() as u32,
		}))
	}

	pub fn water_tile(
		context: &Arc<Context>,
		tile: &libwow::Tile,
		liquid: u32,
	) -> Option<Box<Self>> {
		let mut vertexes = Vec::new();
		let mut indices = Vec::new();
		for chunk in &tile.chunks {
			for mclq in &chunk.mclq.data {
				match mclq.verts {
					libwow::adt::MCLQVert::River(river) => {
						if liquid != libwow::adt::MCNK_FLAG_LQ_RIVER {
							continue;
						}
						if mclq.min_height != mclq.max_height || river.iter().find(|vert|
							vert.height != river[0].height || vert.depth != river[0].depth
						).is_some() {
							let base = vertexes.len() as u16;
							vertexes.extend(river.iter().enumerate().map(|(i, river)| {
								WaterVertex::river(chunk, &river, i)
							}));
							Self::collect_indices(mclq, &mut indices, base);
						} else {
							let base = vertexes.len() as u16;
							vertexes.extend_from_slice(&[
								WaterVertex::river(chunk, &river[0], 0),
								WaterVertex::river(chunk, &river[8], 8),
								WaterVertex::river(chunk, &river[80], 80),
								WaterVertex::river(chunk, &river[72], 72),
							]);
							indices.extend_from_slice(&[
								base + 0, base + 3, base + 1,
								base + 1, base + 3, base + 2,
							]);
						}
					},
					libwow::adt::MCLQVert::Ocean(ocean) => {
						if liquid != libwow::adt::MCNK_FLAG_LQ_OCEAN {
							continue;
						}
						if mclq.min_height != mclq.max_height || ocean.iter().find(|vert| {
							vert.height != ocean[0].height || vert.depth != ocean[0].depth
						}).is_some() {
							let base = vertexes.len() as u16;
							vertexes.extend(ocean.iter().enumerate().map(|(i, ocean)| {
								WaterVertex::ocean(chunk, &ocean, i)
							}));
							Self::collect_indices(mclq, &mut indices, base);
						} else {
							let base = vertexes.len() as u16;
							vertexes.extend_from_slice(&[
								WaterVertex::ocean(chunk, &ocean[0], 0),
								WaterVertex::ocean(chunk, &ocean[8], 8),
								WaterVertex::ocean(chunk, &ocean[80], 80),
								WaterVertex::ocean(chunk, &ocean[72], 72),
							]);
							indices.extend_from_slice(&[
								base + 0, base + 3, base + 1,
								base + 1, base + 3, base + 2,
							]);
						}
					}
					_ => {},
				}
			}
		}
		let mut pmin = Vec3::splat(f32::INFINITY);
		let mut pmax = Vec3::splat(-f32::INFINITY);
		for vertex in &vertexes {
			let vec = Vec3::from_slice(&vertex.position[0..3]);
			pmin = pmin.min(vec);
			pmax = pmax.max(vec);
		}
		Self::tile(
			context,
			&vertexes,
			&indices,
			Aabb::new(pmin, pmax),
		)
	}

	pub fn magma_tile(
		context: &Arc<Context>,
		tile: &libwow::Tile,
		liquid: u32,
	) -> Option<Box<Self>> {
		let mut vertexes = Vec::new();
		let mut indices = Vec::new();
		for chunk in &tile.chunks {
			for mclq in &chunk.mclq.data {
				match mclq.verts {
					libwow::adt::MCLQVert::Magma(magma) => {
						if liquid != libwow::adt::MCNK_FLAG_LQ_MAGMA {
							continue;
						}
						let base = vertexes.len() as u16;
						vertexes.extend(magma.iter().enumerate().map(|(i, magma)| {
							MagmaVertex::magma(chunk, &magma, i)
						}));
						Self::collect_indices(mclq, &mut indices, base);
					},
					libwow::adt::MCLQVert::Slime(slime) => {
						if liquid != libwow::adt::MCNK_FLAG_LQ_SLIME {
							continue;
						}
						let base = vertexes.len() as u16;
						vertexes.extend(slime.iter().enumerate().map(|(i, slime)| {
							MagmaVertex::slime(chunk, &slime, i)
						}));
						Self::collect_indices(mclq, &mut indices, base);
					}
					_ => {},
				}
			}
		}
		let mut pmin = Vec3::splat(f32::INFINITY);
		let mut pmax = Vec3::splat(-f32::INFINITY);
		for vertex in &vertexes {
			let vec = Vec3::from_slice(&vertex.position[0..3]);
			pmin = pmin.min(vec);
			pmax = pmax.max(vec);
		}
		Self::tile(
			context,
			&vertexes,
			&indices,
			Aabb::new(pmin, pmax),
		)
	}

	fn collect_indices(
		mclq: &libwow::adt::MCLQData,
		indices: &mut Vec<u16>,
		base_idx: u16,
	) {
		mclq.tiles.iter().enumerate().filter(|(_, &tile)| {
			tile & libwow::MCLQ_TILE_HIDDEN == 0
		}).for_each(|(i, _)| {
			let idx = base_idx + ((i % 8) + (i / 8 * 9)) as u16;
			indices.extend_from_slice(&[
				idx,
				idx + 10,
				idx + 1,
				idx,
				idx + 9,
				idx + 10,
			]);
		});
	}

	pub fn draw<'a>(
		render_pass: &mut wgpu::RenderPass<'a>,
		liquid: &'a Liquid,
	) {
		render_pass.set_vertex_buffer(0, liquid.vertex_buffer.slice(..));
		render_pass.set_index_buffer(liquid.index_buffer.slice(..), wgpu::IndexFormat::Uint16);
		render_pass.draw_indexed(0..liquid.indices_nb, 0, 0..1);
	}

	pub fn cull(&self, frustum: &Frustum) -> bool {
		frustum.test_aabb(&self.aabb)
	}
}

impl WaterScene {
	pub fn new(
		context: &Context,
		textures: &Vec<TextureAsset>,
		sampler: &wgpu::Sampler,
	) -> Self {
		let scene_block = context.device.create_buffer(&wgpu::BufferDescriptor {
			label: None,
			size: std::mem::size_of::<WaterSceneBlock>() as u64,
			usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
			mapped_at_creation: false,
		});
		let scene_binds = textures.iter().map(|texture|
			context.device.create_bind_group(&wgpu::BindGroupDescriptor {
				label: None,
				layout: &context.water.scene_layout,
				entries: &[wgpu::BindGroupEntry {
					binding: 0,
					resource: scene_block.as_entire_binding(),
				}, wgpu::BindGroupEntry {
					binding: 1,
					resource: wgpu::BindingResource::Sampler(sampler),
				}, wgpu::BindGroupEntry {
					binding: 2,
					resource: wgpu::BindingResource::TextureView(&texture.texture_view()),
				}],
			})
		).collect();
		Self {
			scene_block,
			scene_binds,
		}
	}

	pub fn draw<'a>(
		render_pass: &mut wgpu::RenderPass<'a>,
		scene: &'a WaterScene,
		context: &'a Context,
		frame: &'a Frame,
		ocean: bool,
	) {
		let id = (frame.t / 0.03) as usize % scene.scene_binds.len();
		context.queue.write_buffer(
			&scene.scene_block,
			0,
			bytemuck::cast_slice(&[WaterSceneBlock {
				mvp: frame.vp.to_cols_array(),
				mv: frame.v.to_cols_array(),
			}]),
		);
		render_pass.set_pipeline(&context.water.pipeline);
		render_pass.set_bind_group(0, &scene.scene_binds[id], &[]);
		if ocean { /* XXX this is bad */
			for tile in &frame.oceans {
				Liquid::draw(
					render_pass,
					&tile.ocean.as_ref().unwrap(),
				);
			}
		} else {
			for tile in &frame.rivers {
				Liquid::draw(
					render_pass,
					&tile.river.as_ref().unwrap(),
				);
			}
		}
	}
}

impl MagmaScene {
	pub fn new(
		context: &Context,
		textures: &Vec<TextureAsset>,
		sampler: &wgpu::Sampler,
	) -> Self {
		let scene_block = context.device.create_buffer(&wgpu::BufferDescriptor {
			label: None,
			size: std::mem::size_of::<MagmaSceneBlock>() as u64,
			usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
			mapped_at_creation: false,
		});
		let scene_binds = textures.iter().map(|texture|
			context.device.create_bind_group(&wgpu::BindGroupDescriptor {
				label: None,
				layout: &context.magma.scene_layout,
				entries: &[wgpu::BindGroupEntry {
					binding: 0,
					resource: scene_block.as_entire_binding(),
				}, wgpu::BindGroupEntry {
					binding: 1,
					resource: wgpu::BindingResource::Sampler(sampler),
				}, wgpu::BindGroupEntry {
					binding: 2,
					resource: wgpu::BindingResource::TextureView(&texture.texture_view()),
				}],
			})
		).collect();
		Self {
			scene_block,
			scene_binds,
		}
	}

	pub fn draw<'a>(
		render_pass: &mut wgpu::RenderPass<'a>,
		scene: &'a MagmaScene,
		context: &'a Context,
		frame: &'a Frame,
		slime: bool,
	) {
		let id = (frame.t / 0.03) as usize % scene.scene_binds.len();
		context.queue.write_buffer(
			&scene.scene_block,
			0,
			bytemuck::cast_slice(&[MagmaSceneBlock {
				mvp: frame.vp.to_cols_array(),
				mv: frame.v.to_cols_array(),
			}]),
		);
		render_pass.set_pipeline(&context.magma.pipeline);
		render_pass.set_bind_group(0, &scene.scene_binds[id], &[]);
		if slime { /* XXX this is bad */
			for tile in &frame.slimes {
				Liquid::draw(
					render_pass,
					&tile.slime.as_ref().unwrap(),
				);
			}
		} else {
			for tile in &frame.magmas {
				Liquid::draw(
					render_pass,
					&tile.magma.as_ref().unwrap(),
				);
			}
		}
	}
}

impl WaterGraphics {
	pub fn new(device: &wgpu::Device) -> Self {
		let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
			label: None,
			source: wgpu::ShaderSource::Wgsl(include_str!("shaders/water.wgsl").into()),
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
					view_dimension: wgpu::TextureViewDimension::D2,
					multisampled: false,
				},
				count: None,
			}],
		});
		let layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
			label: None,
			bind_group_layouts: &[
				&scene_layout,
			],
			push_constant_ranges: &[],
		});
		let pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
			label: None,
			layout: Some(&layout),
			vertex: wgpu::VertexState {
				module: &shader,
				entry_point: "vs_main",
				buffers: &[WaterVertex::get_vertex_layout()],
			},
			fragment: Some(wgpu::FragmentState {
				module: &shader,
				entry_point: "fs_main",
				targets: &[Some(wgpu::ColorTargetState {
					format: wgpu::TextureFormat::Bgra8Unorm,
					blend: Some(wgpu::BlendState {
						color: wgpu::BlendComponent::OVER,
						alpha: wgpu::BlendComponent::OVER,
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
		}
	}
}

impl MagmaGraphics {
	pub fn new(device: &wgpu::Device) -> Self {
		let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
			label: None,
			source: wgpu::ShaderSource::Wgsl(include_str!("shaders/magma.wgsl").into()),
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
					view_dimension: wgpu::TextureViewDimension::D2,
					multisampled: false,
				},
				count: None,
			}],
		});
		let layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
			label: None,
			bind_group_layouts: &[
				&scene_layout,
			],
			push_constant_ranges: &[],
		});
		let pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
			label: None,
			layout: Some(&layout),
			vertex: wgpu::VertexState {
				module: &shader,
				entry_point: "vs_main",
				buffers: &[MagmaVertex::get_vertex_layout()],
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
		}
	}
}
