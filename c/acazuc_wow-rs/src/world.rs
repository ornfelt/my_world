use crate::{
	assets::{
		Assets,
		MapObjHandleAsset,
		TextureAsset,
		TileAsset,
	},
	liquid::{
		self,
		Liquid,
	},
	mapobj::{self},
	terrain::{
		self,
		Terrain,
	},
	wow::{
		Context,
		Frame,
	},
};

use std::{
	sync::{
		Arc,
		mpsc,
	},
};

#[derive(Debug)]
pub enum TileHandle {
	DontExists,
	NotLoaded,
	Loading,
	Loaded(TileAsset),
}

#[derive(Debug)]
pub struct TileLoadTask {
	tile: Option<TileAsset>,
	id: u16,
}

#[derive(Debug)]
pub struct Chunk {
}

#[derive(Debug)]
pub struct Tile {
	pub terrain: Terrain,
	pub river: Option<Box<Liquid>>,
	pub ocean: Option<Box<Liquid>>,
	pub magma: Option<Box<Liquid>>,
	pub slime: Option<Box<Liquid>>,
	_chunks: Vec<Chunk>,
	pub mapobjs: Vec<Option<MapObjHandleAsset>>,
}

#[derive(Debug)]
pub struct World {
	terrain_scene: terrain::Scene,
	river_scene: liquid::WaterScene,
	ocean_scene: liquid::WaterScene,
	magma_scene: liquid::MagmaScene,
	slime_scene: liquid::MagmaScene,
	lavag_scene: liquid::MagmaScene,
	mapobj_scene: mapobj::Scene,
	placeholder_texture: TextureAsset,
	tiles: Vec<TileHandle>,
	tiles_to_load: Vec<u16>,
	tiles_loaded: Vec<u16>,
	name: String,
	max_textures_per_batch: usize,
	loader_tx: mpsc::Sender<TileLoadTask>,
	loader_rx: mpsc::Receiver<TileLoadTask>,
	_river_textures: Vec<TextureAsset>,
	_ocean_textures: Vec<TextureAsset>,
	_magma_textures: Vec<TextureAsset>,
	_slime_textures: Vec<TextureAsset>,
	_lavag_textures: Vec<TextureAsset>,
	_liquid_sampler: wgpu::Sampler,
}

impl TileLoadTask {
	fn new(id: u16, tile: Option<TileAsset>) -> Self {
		Self {
			id,
			tile,
		}
	}
}

impl Chunk {
	pub const WIDTH: f32 = 33.334;

	fn new() -> Self {
		Self {
		}
	}
}

impl Tile {
	pub const WIDTH: f32 = Chunk::WIDTH * 16.0;

	fn new(
		context: &Arc<Context>,
		assets: &Arc<Assets>,
		max_textures_per_batch: usize,
		placeholder_texture: &TextureAsset,
		tile: &libwow::Tile,
	) -> Self {
		let mapobjs = tile.modf.data.iter().map(|modf| {
			let name = tile.mwmo.data.split(|&x| x == 0).nth(
				modf.name as usize
			).expect("invalid modf name");
			assets.get_mapobjhandle(
				context,
				modf,
				&libwow::Path::from(name),
			)
		}).collect();
		let chunks = (0..256).map(|_| Chunk::new()).collect::<Vec<Chunk>>();
		let terrain = Terrain::new(
			context,
			assets,
			max_textures_per_batch,
			placeholder_texture,
			tile,
		);
		let river = Liquid::water_tile(
			context,
			tile,
			libwow::adt::MCNK_FLAG_LQ_RIVER,
		);
		let ocean = Liquid::water_tile(
			context,
			tile,
			libwow::adt::MCNK_FLAG_LQ_OCEAN,
		);
		let magma = Liquid::magma_tile(
			context,
			tile,
			libwow::adt::MCNK_FLAG_LQ_MAGMA,
		);
		let slime = Liquid::magma_tile(
			context,
			tile,
			libwow::adt::MCNK_FLAG_LQ_SLIME,
		);
		Self {
			terrain,
			river,
			ocean,
			magma,
			slime,
			_chunks: chunks,
			mapobjs,
		}
	}

	fn load(
		context: Arc<Context>,
		assets: Arc<Assets>,
		loader_tx: mpsc::Sender<TileLoadTask>,
		path: libwow::Path,
		max_textures_per_batch: usize,
		placeholder_texture: TextureAsset,
		id: u16,
	) {
		rayon::spawn_fifo(move || {
			let tile = assets.get_adt(&path).map(|adt|
				Arc::new(Tile::new(
					&context,
					&assets,
					max_textures_per_batch,
					&placeholder_texture,
					&adt,
				))
			);
			loader_tx.send(TileLoadTask::new(
				id,
				tile,
			)).expect("failed to send tile load");
		});
	}

	fn collect_frame(tile: &TileAsset, frame: &mut Frame) {
		if tile.terrain.cull(&frame.frustum) {
			frame.terrains.push(tile.clone());
		}
		if let Some(river) = &tile.river {
			if river.cull(&frame.frustum) {
				frame.rivers.push(tile.clone());
			}
		}
		if let Some(ocean) = &tile.ocean {
			if ocean.cull(&frame.frustum) {
				frame.oceans.push(tile.clone());
			}
		}
		if let Some(magma) = &tile.magma {
			if magma.cull(&frame.frustum) {
				frame.magmas.push(tile.clone());
			}
		}
		if let Some(slime) = &tile.slime {
			if slime.cull(&frame.frustum) {
				frame.slimes.push(tile.clone());
			}
		}
		for mapobj in &tile.mapobjs {
			if let Some(mapobj) = mapobj {
				if mapobj.cull(&frame.frustum) {
					mapobj.mark_culled();
					frame.mapobjs.push(mapobj.clone());
				}
			}
		}
	}
}
impl World {
	pub const WIDTH: f32 = Tile::WIDTH * 64.0;

	pub fn new(
		context: &Context,
		assets: &mut Assets,
	) -> Self {
		let placeholder_texture = assets.get_texture(
			&context,
			&libwow::Path::from("tileset\\generic\\grey.blp"),
		).expect("texture not found");
		let river_textures = (1..=30).map(|i| {
			assets.get_texture(&context, &libwow::Path::from(
				format!("XTEXTURES\\RIVER\\LAKE_A.{}.BLP", i)
			)).expect("texture not found")
		}).collect::<Vec<TextureAsset>>();
		let ocean_textures = (1..=30).map(|i| {
			assets.get_texture(&context, &libwow::Path::from(
				format!("XTEXTURES\\OCEAN\\OCEAN_H.{}.BLP", i)
			)).expect("texture not found")
		}).collect::<Vec<TextureAsset>>();
		let magma_textures = (1..=30).map(|i| {
			assets.get_texture(&context, &libwow::Path::from(
				format!("XTEXTURES\\LAVA\\LAVA.{}.BLP", i)
			)).expect("texture not found")
		}).collect::<Vec<TextureAsset>>();
		let slime_textures = (1..=30).map(|i| {
			assets.get_texture(&context, &libwow::Path::from(
				format!("XTEXTURES\\SLIME\\SLIME.{}.BLP", i)
			)).expect("texture not found")
		}).collect::<Vec<TextureAsset>>();
		let lavag_textures = (1..=30).map(|i| {
			assets.get_texture(&context, &libwow::Path::from(
				format!("XTEXTURES\\LAVAGREEN\\LAVAGREEN.{}.BLP", i)
			)).expect("texture not found")
		}).collect::<Vec<TextureAsset>>();
		let liquid_sampler = context.device.create_sampler(&wgpu::SamplerDescriptor {
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
		let terrain_scene = terrain::Scene::new(context);
		let river_scene = liquid::WaterScene::new(context, &river_textures, &liquid_sampler);
		let ocean_scene = liquid::WaterScene::new(context, &ocean_textures, &liquid_sampler);
		let magma_scene = liquid::MagmaScene::new(context, &magma_textures, &liquid_sampler);
		let slime_scene = liquid::MagmaScene::new(context, &slime_textures, &liquid_sampler);
		let lavag_scene = liquid::MagmaScene::new(context, &lavag_textures, &liquid_sampler);
		let mapobj_scene = mapobj::Scene::new(context);
		let tiles = (0..64 * 64).map(|_| TileHandle::DontExists).collect();
		let (loader_tx, loader_rx) = mpsc::channel();
		Self {
			terrain_scene,
			river_scene,
			ocean_scene,
			magma_scene,
			slime_scene,
			lavag_scene,
			mapobj_scene,
			placeholder_texture,
			tiles,
			name: String::new(),
			tiles_to_load: Vec::new(),
			tiles_loaded: Vec::new(),
			max_textures_per_batch: context.device.limits().max_samplers_per_shader_stage as usize / 2,
			loader_tx,
			loader_rx,
			_river_textures: river_textures,
			_ocean_textures: ocean_textures,
			_magma_textures: magma_textures,
			_slime_textures: slime_textures,
			_lavag_textures: lavag_textures,
			_liquid_sampler: liquid_sampler,
		}
	}

	pub fn load(
		&mut self,
		assets: &Arc<Assets>,
		name: String,
	) {
		self.name = name.clone();
		let wdt = assets.get_wdt(&libwow::Path::from(format!(
			"world\\maps\\{}\\{}.wdt",
			name,
			name,
		))).expect("wdt not found");
		let _wdl = assets.get_wdl(&libwow::Path::from(format!(
			"world\\maps\\{}\\{}.wdl",
			name,
			name,
		))).expect("wdl not found");
		self.tiles.iter_mut().enumerate().filter(|(i, _)|
			wdt.main.data[*i].flags & 1 == 1
		).for_each(|(i, handle)| {
			self.tiles_to_load.push(i as u16);
			*handle = TileHandle::NotLoaded
		});
	}

	pub fn tick(
		&mut self,
		context: &Arc<Context>,
		assets: &Arc<Assets>,
	) {
		while let Ok(task) = self.loader_rx.try_recv() {
			self.tiles[task.id as usize] = match task.tile {
				Some(tile) => {
					self.tiles_loaded.push(task.id);
					TileHandle::Loaded(tile)
				},
				None => TileHandle::DontExists,
			};
		}
		if let Some(i) = self.tiles_to_load.pop() {
			let x = i as usize % 64;
			let z = i as usize / 64;
			self.tiles[i as usize] = TileHandle::Loading;
			Tile::load(
				context.clone(),
				assets.clone(),
				self.loader_tx.clone(),
				libwow::Path::from(format!(
					"world\\maps\\{}\\{}_{}_{}.adt",
					self.name,
					self.name,
					x,
					z,
				)),
				self.max_textures_per_batch,
				self.placeholder_texture.clone(),
				i,
			);
		}
	}

	pub fn draw<'a>(
		render_pass: &mut wgpu::RenderPass<'a>,
		context: &'a Context,
		world: &'a World,
		frame: &'a Frame,
	) {
		terrain::Scene::draw(
			render_pass,
			&world.terrain_scene,
			context,
			frame,
		);
		liquid::WaterScene::draw(
			render_pass,
			&world.river_scene,
			context,
			frame,
			false,
		);
		liquid::WaterScene::draw(
			render_pass,
			&world.ocean_scene,
			context,
			frame,
			true,
		);
		liquid::MagmaScene::draw(
			render_pass,
			if world.name == "expansion01" {
				&world.lavag_scene
			} else {
				&world.magma_scene
			},
			context,
			frame,
			false,
		);
		liquid::MagmaScene::draw(
			render_pass,
			&world.slime_scene,
			context,
			frame,
			true,
		);
		mapobj::Scene::draw(
			render_pass,
			&world.mapobj_scene,
			context,
			frame,
		);
	}

	pub fn collect_frame(&self, frame: &mut Frame) {
		self.tiles_loaded.iter().for_each(|&i| match &self.tiles[i as usize] {
			TileHandle::Loaded(tile) => Tile::collect_frame(&tile, frame),
			_ => {},
		});
	}
}
