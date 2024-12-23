use crate::{
	assets::{
		Assets,
		MapObjHandleAsset,
		TileAsset,
	},
	liquid::{self},
	mapobj::{self},
	math::{
		Vec3,
		Mat2,
		Mat4,
		Frustum,
	},
	terrain::{self},
	window::Window,
	world::World,
};

use winit::{
	event::{
		MouseButton,
	},
	keyboard::{
		SmolStr,
		PhysicalKey::{
			self,
			Code,
		},
		KeyCode,
	},
};

use std::{
	time::Instant,
	f32::consts::PI,
	sync::Arc,
	collections::VecDeque,
};

use glam::Vec3Swizzles;

const KEY_FORWARD:      u32 = 0x1;
const KEY_BACKWARD:     u32 = 0x2;
const KEY_TURN_LEFT:    u32 = 0x4;
const KEY_TURN_RIGHT:   u32 = 0x8;
const KEY_STRAFE_LEFT:  u32 = 0x10;
const KEY_STRAFE_RIGHT: u32 = 0x20;
const KEY_UPWARD:       u32 = 0x40;
const KEY_DOWNWARD:     u32 = 0x80;

const CAMERA_SENSIBILITY: f32 = PI / 5000.0;

pub struct Wow {
	assets: Arc<Assets>,
	pub window: Window,
	pub context: Arc<Context>,
	camera: Camera,
	world: World,
	start_time: Instant,
	frame_time: Instant,
	last_frame_time: Instant,
	last_fps: Instant,
	fps: u32,
	dt: f32,
	t: f32,
	frames: VecDeque<Frame>,
}

pub struct Camera {
	pub p: Mat4,
	pub v: Mat4,
	pub vp: Mat4,
	pub position: Vec3,
	pub rotation: Vec3,
	pub speed: f32,
	pub keys: u32,
	pub frustum: Frustum,
	pub fov: f32,
}

pub struct Context {
	pub instance: wgpu::Instance,
	pub surface: wgpu::Surface,
	pub surface_capabilities: wgpu::SurfaceCapabilities,
	pub adapter: wgpu::Adapter,
	pub device: wgpu::Device,
	pub queue: wgpu::Queue,
	pub terrain: terrain::Graphics,
	pub water: liquid::WaterGraphics,
	pub magma: liquid::MagmaGraphics,
	pub mapobj: mapobj::Graphics,
}

pub struct Frame {
	pub frustum: Frustum,
	pub position: Vec3,
	pub rotation: Vec3,
	pub vp: Mat4,
	pub p: Mat4,
	pub v: Mat4,
	pub t: f32,
	pub terrains: Vec<TileAsset>,
	pub rivers: Vec<TileAsset>,
	pub oceans: Vec<TileAsset>,
	pub magmas: Vec<TileAsset>,
	pub slimes: Vec<TileAsset>,
	pub mapobjs: Vec<MapObjHandleAsset>,
}

impl Wow {
	pub async fn new(event_loop: &winit::event_loop::EventLoop<()>) -> Self {
		let mut assets = Assets::new();
		let window = Window::new(event_loop);
		let context = Context::new(&window).await;
		let world = World::new(&context, &mut assets);
		let start_time = Instant::now();
		let camera = Camera::new();
		let frames = VecDeque::from([
			Frame::new(),
			Frame::new(),
			Frame::new(),
		]);
		Self {
			assets: Arc::new(assets),
			window,
			context: Arc::new(context),
			world,
			start_time,
			frame_time: start_time,
			last_frame_time: start_time,
			last_fps: start_time,
			fps: 0,
			t: 0.0,
			dt: 0.0,
			camera,
			frames,
		}
	}

	pub fn render(&mut self) {
		self.last_frame_time = self.frame_time;
		self.frame_time = Instant::now();
		self.t = (self.frame_time - self.start_time).as_secs_f32();
		self.dt = (self.frame_time - self.last_frame_time).as_secs_f32();
		self.fps += 1;
		if (self.frame_time - self.last_fps).as_secs_f32() >= 1.0 {
			println!("fps: {}", self.fps);
			self.last_fps += std::time::Duration::from_secs(1);
			self.fps = 0;
		}
		self.camera.handle_movements(self.dt);
		self.camera.update_matrix(self.window.aspect_ratio());
		let framebuffer = self.context.surface.get_current_texture().expect("todo ptdr");
		let mut encoder = self.context.device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
			label: None,
		});
		self.world.tick(&self.context, &self.assets);
		self.render_pass(&framebuffer, &mut encoder);
		self.frames.rotate_left(1);
		self.context.queue.submit(Some(encoder.finish()));
		framebuffer.present();
	}

	pub fn on_key_down(&mut self, key: PhysicalKey) {
		//println!("key down {:?}", key);
		match key {
			Code(KeyCode::KeyW)      => self.camera.keys |= KEY_FORWARD,
			Code(KeyCode::KeyS)      => self.camera.keys |= KEY_BACKWARD,
			Code(KeyCode::KeyA)      => self.camera.keys |= KEY_TURN_LEFT,
			Code(KeyCode::KeyD)      => self.camera.keys |= KEY_TURN_RIGHT,
			Code(KeyCode::KeyQ)      => self.camera.keys |= KEY_STRAFE_LEFT,
			Code(KeyCode::KeyE)      => self.camera.keys |= KEY_STRAFE_RIGHT,
			Code(KeyCode::Space)     => self.camera.keys |= KEY_UPWARD,
			Code(KeyCode::ShiftLeft) => self.camera.keys |= KEY_DOWNWARD,
			_ => {},
		}
	}

	pub fn on_key_press(&mut self, _key: PhysicalKey) {
		//println!("key press {:?}", key);
	}

	pub fn on_key_up(&mut self, key: PhysicalKey) {
		//println!("key up {:?}", key);
		match key {
			Code(KeyCode::KeyW)      => self.camera.keys &= !KEY_FORWARD,
			Code(KeyCode::KeyS)      => self.camera.keys &= !KEY_BACKWARD,
			Code(KeyCode::KeyA)      => self.camera.keys &= !KEY_TURN_LEFT,
			Code(KeyCode::KeyD)      => self.camera.keys &= !KEY_TURN_RIGHT,
			Code(KeyCode::KeyQ)      => self.camera.keys &= !KEY_STRAFE_LEFT,
			Code(KeyCode::KeyE)      => self.camera.keys &= !KEY_STRAFE_RIGHT,
			Code(KeyCode::Space)     => self.camera.keys &= !KEY_UPWARD,
			Code(KeyCode::ShiftLeft) => self.camera.keys &= !KEY_DOWNWARD,
			_ => {},
		}
	}

	pub fn on_char(&mut self, _text: SmolStr) {
		//println!("text {:?}", text);
	}

	pub fn on_mouse_down(&mut self, button: MouseButton) {
		match button {
			MouseButton::Left => self.window.grab(),
			_ => {},
		}
	}

	pub fn on_mouse_up(&mut self, button: MouseButton) {
		match button {
			MouseButton::Left => self.window.ungrab(),
			_ => {},
		}
	}

	pub fn on_mouse_move(&mut self, x: f32, y: f32) {
		//println!("mouse move {} {}", x, y);
		if self.window.grabbed() {
			self.camera.rotation.y += x * CAMERA_SENSIBILITY;
			self.camera.rotation.x += y * CAMERA_SENSIBILITY;
			self.camera.rotation.x = self.camera.rotation.x.min(PI * 0.5).max(-PI * 0.5);
		}
	}

	fn render_pass(
		&mut self,
		framebuffer: &wgpu::SurfaceTexture,
		encoder: &mut wgpu::CommandEncoder,
	) {
		let view = framebuffer.texture.create_view(&wgpu::TextureViewDescriptor::default());
		let depth = self.context.device.create_texture(&wgpu::TextureDescriptor {
			label: None,
			size: wgpu::Extent3d {
				width: framebuffer.texture.width(),
				height: framebuffer.texture.height(),
				depth_or_array_layers: 1,
			},
			mip_level_count: 1,
			sample_count: 1,
			dimension: wgpu::TextureDimension::D2,
			format: wgpu::TextureFormat::Depth24PlusStencil8,
			usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
			view_formats: &[wgpu::TextureFormat::Depth24PlusStencil8],
		});
		let depth_view = depth.create_view(&wgpu::TextureViewDescriptor::default());
		let frame = self.frames.front_mut().expect("no frames available");
		let mut render_pass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
			label: None,
			color_attachments: &[Some(wgpu::RenderPassColorAttachment {
				view: &view,
				resolve_target: None,
				ops: wgpu::Operations {
					load: wgpu::LoadOp::Clear(wgpu::Color {
						r: 0.5,
						g: 0.5,
						b: 0.5,
						a: 0.5,
					}),
					store: wgpu::StoreOp::Store,
				},
			})],
			depth_stencil_attachment: Some(wgpu::RenderPassDepthStencilAttachment {
				view: &depth_view,
				depth_ops: Some(wgpu::Operations {
					load: wgpu::LoadOp::Clear(1.0),
					store: wgpu::StoreOp::Discard,
				}),
				stencil_ops: Some(wgpu::Operations {
					load: wgpu::LoadOp::Clear(0),
					store: wgpu::StoreOp::Discard,
				}),
			}),
			timestamp_writes: None,
			occlusion_query_set: None,
		});
		frame.begin(&self.camera, self.t);
		frame.collect(&self.world);
		frame.end();
		World::draw(
			&mut render_pass,
			&self.context,
			&self.world,
			&frame,
		);
	}

	pub fn load_world(&mut self, name: String) {
		self.world.load(&self.assets, name);
	}
}

impl Camera {
	pub fn new() -> Self {
		Self {
			v: Mat4::IDENTITY,
			p: Mat4::IDENTITY,
			vp: Mat4::IDENTITY,
			position: Vec3::splat(0.0),
			rotation: Vec3::splat(0.0),
			keys: 0,
			speed: 1000.0,
			frustum: Frustum::new(),
			fov: 90.0 / 180.0 * PI,
		}
	}

	fn update_matrix(&mut self, aspect_ratio: f32) {
		self.p = Mat4::perspective_lh(self.fov, aspect_ratio, 0.5, World::WIDTH / 2.0);
		self.v = Mat4::from_rotation_x(-self.rotation.x);
		self.v = self.v.mul_mat4(&Mat4::from_rotation_y(-self.rotation.y));
		self.v = self.v.mul_mat4(&Mat4::from_rotation_z(-self.rotation.z));
		self.v = self.v.mul_mat4(&Mat4::from_translation(-self.position));
		self.vp = self.p.mul_mat4(&self.v);
		self.frustum.clear();
		self.frustum.push_mat4(&self.vp);
	}

	fn handle_movements(&mut self, dt: f32) {
		if self.keys == 0 {
			return;
		}
		let velocity = Vec3::new(
			match self.keys & (KEY_STRAFE_LEFT | KEY_STRAFE_RIGHT) {
				KEY_STRAFE_LEFT  => -1.0,
				KEY_STRAFE_RIGHT =>  1.0,
				_                =>  0.0,
			},
			match self.keys & (KEY_UPWARD | KEY_DOWNWARD) {
				KEY_UPWARD   =>  1.0,
				KEY_DOWNWARD => -1.0,
				_            =>  0.0,
			},
			match self.keys & (KEY_FORWARD | KEY_BACKWARD) {
				KEY_FORWARD  =>  1.0,
				KEY_BACKWARD => -1.0,
				_            =>  0.0,
			},
		);
		let mat = Mat2::from_angle(-self.rotation.y);
		let rotated = mat.mul_vec2(velocity.xz());
		self.position += Vec3::new(rotated.x, velocity.y, rotated.y).normalize_or_zero() * self.speed * dt;
	}
}

impl Context {
	pub async fn new(window: &Window) -> Self {
		let instance = wgpu::Instance::new(wgpu::InstanceDescriptor {
			backends: wgpu::Backends::VULKAN,
			dx12_shader_compiler: Default::default(),
			flags: Default::default(),
			gles_minor_version: Default::default(),
		});
		let surface = unsafe {
			instance.create_surface(&window.window)
		}.unwrap();
		for adapter in instance.enumerate_adapters(wgpu::Backends::all()) {
			println!("{:?}", adapter.get_info())
		}
		let adapter = instance.request_adapter(&wgpu::RequestAdapterOptions {
			power_preference: Default::default(),
			force_fallback_adapter: false,
			compatible_surface: Some(&surface),
		}).await.expect("no adapter found");
		let (device, queue) = adapter.request_device(&wgpu::DeviceDescriptor {
			label: None,
			features: wgpu::Features::TEXTURE_COMPRESSION_BC | wgpu::Features::TEXTURE_BINDING_ARRAY | wgpu::Features::SAMPLED_TEXTURE_AND_STORAGE_BUFFER_ARRAY_NON_UNIFORM_INDEXING | wgpu::Features::POLYGON_MODE_LINE,
			limits: Default::default(),
		}, None).await.expect("no device found");
		let surface_capabilities = surface.get_capabilities(&adapter);
		surface.configure(&device, &wgpu::SurfaceConfiguration {
			usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
			format: wgpu::TextureFormat::Bgra8Unorm,
			width: window.width(),
			height: window.height(),
			present_mode: wgpu::PresentMode::AutoNoVsync,
			alpha_mode: wgpu::CompositeAlphaMode::Opaque,
			view_formats: vec![],
		});

		let terrain = terrain::Graphics::new(&device);
		let water = liquid::WaterGraphics::new(&device);
		let magma = liquid::MagmaGraphics::new(&device);
		let mapobj = mapobj::Graphics::new(&device);
		Self {
			instance,
			surface,
			surface_capabilities,
			adapter,
			device,
			queue,
			terrain,
			water,
			magma,
			mapobj,
		}
	}

	pub fn resize(&self, width: u32, height: u32) {
		self.instance.poll_all(true);
		self.surface.configure(&self.device, &wgpu::SurfaceConfiguration {
			usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
			format: wgpu::TextureFormat::Bgra8Unorm,
			width: width,
			height: height,
			present_mode: wgpu::PresentMode::AutoNoVsync,
			alpha_mode: wgpu::CompositeAlphaMode::Opaque,
			view_formats: vec![],
		});
	}
}

impl Frame {
	pub fn new() -> Self {
		Self {
			frustum: Frustum::new(),
			position: Vec3::default(),
			rotation: Vec3::default(),
			vp: Mat4::default(),
			p: Mat4::default(),
			v: Mat4::default(),
			t: 0.0,
			terrains: Vec::new(),
			rivers: Vec::new(),
			oceans: Vec::new(),
			magmas: Vec::new(),
			slimes: Vec::new(),
			mapobjs: Vec::new(),
		}
	}

	pub fn begin(&mut self, camera: &Camera, t: f32) {
		self.frustum = camera.frustum.clone();
		self.position = camera.position;
		self.rotation = camera.rotation;
		self.vp = camera.vp;
		self.p = camera.p;
		self.v = camera.v;
		self.t = t;
		self.terrains.clear();
		self.rivers.clear();
		self.oceans.clear();
		self.magmas.clear();
		self.slimes.clear();
		self.mapobjs.clear();
	}

	pub fn collect(&mut self, world: &World) {
		world.collect_frame(self)
	}

	pub fn end(&mut self) {
		for mapobj in &self.mapobjs {
			mapobj.clear_culled();
		}
	}
}
