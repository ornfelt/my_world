use crate::{
	assets::{
		Assets,
		MapObjAsset,
		MapObjGroupAsset,
		TextureAsset,
	},
	math::{
		Aabb,
		Frustum,
		Mat4,
		Vec3,
	},
	world::{
		World,
	},
	wow::{
		Context,
		Frame,
	},
};

use std::{
	sync::{
		Arc,
		atomic::{
			AtomicBool,
			Ordering,
		},
	},
	ops::Range,
	f32::consts::PI,
};

use bytemuck::{
	Pod,
	Zeroable,
};

use glam::{
	EulerRot,
	Quat,
};

use wgpu::util::DeviceExt;

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct Vertex {
	position: [f32; 4],
	normal: [f32; 4],
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
	mvp: [f32; 16],
	mv: [f32; 16],
	m: [f32; 16],
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct MeshBlock {
	dummy: u32,
}

#[derive(Debug)]
pub struct MapObjHandle {
	pub instance: Arc<MapObjInstance>,
	culled: AtomicBool,
}

#[derive(Debug)]
pub struct MapObjInstance {
	mapobj: Option<MapObjAsset>,
	matrix: Mat4,
	model_binds: wgpu::BindGroup,
	model_block: wgpu::Buffer,
	aabb: Aabb,
}

#[derive(Debug)]
pub struct MapObjBatch {
	mesh_binds: wgpu::BindGroup,
	_sampler: wgpu::Sampler,
	indices: Range<u32>,
}

#[derive(Debug)]
pub struct MapObjGroup {
	vertex_buffer: wgpu::Buffer,
	index_buffer: wgpu::Buffer,
	aabb: Aabb,
	batches: Vec<MapObjBatch>,
}

#[derive(Debug)]
pub struct MapObjMaterial {
	_momt: libwow::MOMTData,
	texture: Option<TextureAsset>,
}

#[derive(Debug)]
pub struct MapObj {
	groups: Vec<Option<MapObjGroupAsset>>,
	materials: Vec<MapObjMaterial>,
	aabb: Aabb,
	mohd: libwow::MOHDData,
	path: libwow::Path,
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
	fn new(wmo_group: &libwow::MapObjGroup, i: usize) -> Self {
		let position = wmo_group.movt.data[i];
		let normal = wmo_group.monr.data[i];
		let uv = wmo_group.motv.data[i];
		Self {
			position: [
				position[0],
				position[2],
				position[1],
				1.0,
			],
			normal: [
				normal[0],
				normal[2],
				normal[1],
				0.0,
			],
			uv: [
				uv[0],
				uv[1],
			],
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
				format: wgpu::VertexFormat::Float32x4,
				offset: 4 * 4,
				shader_location: 1,
			}, wgpu::VertexAttribute {
				format: wgpu::VertexFormat::Float32x2,
				offset: 4 * 8,
				shader_location: 2,
			}],
		}
	}
}

impl MapObjHandle {
	pub fn new(
		context: &Context,
		assets: &Assets,
		modf: &libwow::MODFData,
		path: &libwow::Path,
	) -> Self {
		let instance = MapObjInstance::new(context, assets, modf, path);
		Self {
			instance: Arc::new(instance),
			culled: AtomicBool::new(false),
		}
	}

	pub fn cull(&self, frustum: &Frustum) -> bool {
		!self.culled.load(Ordering::SeqCst)
		&& !self.instance.mapobj.is_none()
		&& frustum.test_aabb(&self.instance.aabb)
	}

	pub fn mark_culled(&self) {
		self.culled.fetch_or(true, Ordering::SeqCst);
	}

	pub fn clear_culled(&self) {
		self.culled.fetch_and(false, Ordering::SeqCst);
	}
}

impl MapObjInstance {
	pub fn new(
		context: &Context,
		assets: &Assets,
		modf: &libwow::MODFData,
		path: &libwow::Path,
	) -> Self {
		let mapobj = assets.get_mapobj(context, path);
		let offset = World::WIDTH / 2.0;// - Chunk::WIDTH * 8.0;
		let matrix = Mat4::from_scale_rotation_translation(
			Vec3::splat(1.0),
			Quat::from_euler(
				EulerRot::YZX,
				-(modf.rotation[1] + 180.0) / 180.0 * PI,
				-(modf.rotation[0]) / 180.0 * PI,
				(modf.rotation[2]) / 180.0 * PI,
			),
			Vec3::new(
				offset - modf.position[2],
				modf.position[1],
				offset - modf.position[0],
			),
		);
		let model_block = context.device.create_buffer(&wgpu::BufferDescriptor {
			label: None,
			size: std::mem::size_of::<ModelBlock>() as u64,
			usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
			mapped_at_creation: false,
		});
		let model_binds = context.device.create_bind_group(&wgpu::BindGroupDescriptor {
			label: None,
			layout: &context.mapobj.model_layout,
			entries: &[wgpu::BindGroupEntry {
				binding: 0,
				resource: model_block.as_entire_binding(),
			}],
		});
		let aabb = match &mapobj {
			Some(mapobj) => mapobj.aabb.transform(&matrix),
			None => Aabb::new(Vec3::splat(0.0), Vec3::splat(0.0)),
		};
		Self {
			mapobj,
			matrix,
			model_block,
			model_binds,
			aabb,
		}
	}

	pub fn draw<'a>(
		render_pass: &mut wgpu::RenderPass<'a>,
		context: &'a Context,
		instance: &'a MapObjInstance,
		frame: &'a Frame,
	) {
		context.queue.write_buffer(
			&instance.model_block,
			0,
			bytemuck::cast_slice(&[ModelBlock {
				mvp: frame.vp.mul_mat4(&instance.matrix).to_cols_array(),
				mv: frame.v.mul_mat4(&instance.matrix).to_cols_array(),
				m: instance.matrix.to_cols_array(),
			}]),
		);
		render_pass.set_pipeline(&context.mapobj.pipeline);
		render_pass.set_bind_group(1, &instance.model_binds, &[]);
		instance.mapobj.as_ref().unwrap().groups.iter().for_each(|group| {
			if let Some(group) = group {
				render_pass.set_vertex_buffer(0, group.vertex_buffer.slice(..));
				render_pass.set_index_buffer(group.index_buffer.slice(..), wgpu::IndexFormat::Uint16);
				group.batches.iter().for_each(|batch| {
					render_pass.set_bind_group(2, &batch.mesh_binds, &[]);
					render_pass.draw_indexed(batch.indices.clone(), 0, 0..1);
				});
			}
		});
	}
}

impl MapObjBatch {
	pub fn new(
		context: &Context,
		placeholder_texture: &TextureAsset,
		mapobj: &MapObj,
		moba: &libwow::MOBAData,
	) -> Self {
		let sampler = context.device.create_sampler(&wgpu::SamplerDescriptor {
			label: None,
			address_mode_u: wgpu::AddressMode::Repeat,
			address_mode_v: wgpu::AddressMode::Repeat,
			address_mode_w: wgpu::AddressMode::Repeat,
			mag_filter: wgpu::FilterMode::Linear,
			min_filter: wgpu::FilterMode::Linear,
			mipmap_filter: wgpu::FilterMode::Linear,
			lod_min_clamp: 0.0,
			lod_max_clamp: 0.0,
			compare: None,
			anisotropy_clamp: 16,
			border_color: None,
		});
		let mesh_block = context.device.create_buffer(&wgpu::BufferDescriptor {
			label: None,
			size: std::mem::size_of::<MeshBlock>() as u64,
			usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
			mapped_at_creation: false,
		});
		let mesh_binds = context.device.create_bind_group(&wgpu::BindGroupDescriptor {
			label: None,
			layout: &context.mapobj.mesh_layout,
			entries: &[wgpu::BindGroupEntry {
				binding: 0,
				resource: mesh_block.as_entire_binding(),
			}, wgpu::BindGroupEntry {
				binding: 1,
				resource: wgpu::BindingResource::Sampler(&sampler),
			}, wgpu::BindGroupEntry {
				binding: 2,
				resource: wgpu::BindingResource::TextureView(
					mapobj.materials[moba.material as usize].texture.as_ref().or_else(|| Some(placeholder_texture)).unwrap().texture_view()
				),
			}],
		});
		Self {
			mesh_binds,
			_sampler: sampler,
			indices: moba.start..moba.start + moba.count as u32,
		}
	}
}

impl MapObjGroup {
	pub fn new(
		context: &Context,
		assets: &Assets,
		mapobj: &MapObj,
		wmo_group: &libwow::MapObjGroup,
	) -> Self {
		let indices = &wmo_group.movi.data;
		if wmo_group.movt.data.len() != wmo_group.monr.data.len()
		|| wmo_group.movt.data.len() != wmo_group.motv.data.len() {
			panic!("invalid wmo group data length");
		}
		let vertexes = (0..wmo_group.movt.data.len()).map(|i| {
			Vertex::new(wmo_group, i)
		}).collect::<Vec<Vertex>>();
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
		let mut pmin = Vec3::splat(f32::INFINITY);
		let mut pmax = Vec3::splat(-f32::INFINITY);
		for vertex in &vertexes {
			let vec = Vec3::from_slice(&vertex.position[0..3]);
			pmin = pmin.min(vec);
			pmax = pmax.max(vec);
		}
		let placeholder_texture = assets.get_texture(
			&context,
			&libwow::Path::from("tileset\\generic\\grey.blp"),
		).expect("texture not found");
		let batches = wmo_group.moba.data.iter().map(|x| {
			MapObjBatch::new(context, &placeholder_texture, mapobj, x)
		}).collect();
		Self {
			vertex_buffer,
			index_buffer,
			aabb: Aabb::new(pmin, pmax),
			batches,
		}
	}
}

impl MapObjMaterial {
	pub fn new(
		context: &Context,
		assets: &Assets,
		motx: &libwow::MOTX,
		momt: &libwow::MOMTData,
	) -> Self {
		let motx_begin = momt.texture1 as usize;
		let motx_end = motx_begin + motx.data.iter().skip(momt.texture1 as usize).position(|&x| x == 0).expect("invalid texture string");
		let texture = assets.get_texture(context, &libwow::Path::from(
			std::str::from_utf8(
				&motx.data[motx_begin..motx_end],
			).expect("invalid texture string"),
		));
		Self {
			_momt: *momt,
			texture,
		}
	}
}

impl MapObj {
	pub fn new(
		context: &Context,
		assets: &Assets,
		wmo: &libwow::MapObj,
		path: &libwow::Path,
	) -> Self {
		let materials = wmo.momt.data.iter().map(|x| {
			MapObjMaterial::new(context, assets, &wmo.motx, x)
		}).collect::<Vec<MapObjMaterial>>();
		Self {
			groups: Vec::new(),
			materials,
			aabb: Aabb::new(Vec3::splat(f32::INFINITY), Vec3::splat(-f32::INFINITY)),
			mohd: wmo.mohd.data,
			path: path.clone(),
		}
	}

	pub fn load_groups(
		&mut self,
		context: &Context,
		assets: &Assets,
	) {
		self.groups = (0..self.mohd.groups_nb).map(|x| {
			assets.get_mapobjgroup(context, &self, &libwow::Path::from(
				format!(
					"{}_{:03}.WMO",
					String::from(std::str::from_utf8(
						&self.path.value.as_bytes()[0..self.path.value.len() - 4]
					).expect("invalid texture string")),
					x,
				),
			))
		}).collect::<Vec<Option<MapObjGroupAsset>>>();
		let mut pmin = Vec3::splat(f32::INFINITY);
		let mut pmax = Vec3::splat(-f32::INFINITY);
		for group in &self.groups {
			if let Some(group) = &group {
				pmin = pmin.min(group.aabb.p0);
				pmax = pmax.max(group.aabb.p1);
			}
		}
		self.aabb = Aabb::new(pmin, pmax);
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
			layout: &context.mapobj.scene_layout,
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
		render_pass.set_pipeline(&context.mapobj.pipeline);
		render_pass.set_bind_group(0, &scene.scene_binds, &[]);
		for mapobj in &frame.mapobjs {
			MapObjInstance::draw(
				render_pass,
				context,
				&mapobj.instance,
				frame,
			);
		}
	}
}

impl Graphics {
	pub fn new(device: &wgpu::Device) -> Self {
		let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
			label: None,
			source: wgpu::ShaderSource::Wgsl(include_str!("shaders/mapobj.wgsl").into()),
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
				visibility: wgpu::ShaderStages::VERTEX,
				ty: wgpu::BindingType::Buffer {
					ty: wgpu::BufferBindingType::Uniform,
					has_dynamic_offset: false,
					min_binding_size: None,
				},
				count: None,
			}],
		});
		let mesh_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
			label: None,
			entries: &[wgpu::BindGroupLayoutEntry {
				binding: 0,
				visibility: wgpu::ShaderStages::VERTEX | wgpu::ShaderStages::FRAGMENT,
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
