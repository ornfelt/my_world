use crate::{
	mapobj::{
		MapObj,
		MapObjGroup,
		MapObjHandle,
	},
	texture::Texture,
	world::Tile,
	wow::Context,
};

use std::{
	collections::HashMap,
	hash::Hash,
	sync::{
		Arc,
		Mutex,
		Weak,
	},
};

/* XXX this whole code organization feels sooooo wrong
 */

type CollectionMap<K, V> = Arc<Mutex<HashMap<K, Weak<Asset<K, V>>>>>;

#[derive(Debug)]
pub struct Asset<K, V> where K: Hash + Eq + PartialEq {
	collection: CollectionMap<K, V>,
	key: K,
	value: V,
}

#[derive(Debug)]
struct AssetCollection<K, V> where K: Hash + Eq + PartialEq {
	assets: CollectionMap<K, V>,
}

pub type TileAsset = Arc<Tile>;
pub type TextureAsset = Arc<Asset<libwow::Path, Texture>>;
pub type MapObjHandleAsset = Arc<Asset<u32, MapObjHandle>>;
pub type MapObjAsset = Arc<Asset<libwow::Path, MapObj>>;
pub type MapObjGroupAsset = Arc<Asset<libwow::Path, MapObjGroup>>;

#[derive(Debug)]
pub struct Assets {
	archives: Vec<libwow::Archive>,
	textures: AssetCollection<libwow::Path, Texture>,
	mapobjhandles: AssetCollection<u32, MapObjHandle>,
	mapobjs: AssetCollection<libwow::Path, MapObj>,
	mapobjgroups: AssetCollection<libwow::Path, MapObjGroup>,
}

const ARCHIVES: [&str; 14] = [
	"patch-5.MPQ",
	"patch-3.MPQ",
	"patch-2.MPQ",
	"patch.MPQ",
	"frFR/patch-frFR-2.MPQ",
	"frFR/patch-frFR.MPQ",
	"expansion.MPQ",
	"common.MPQ",
	"frFR/base-frFR.MPQ",
	"frFR/backup-frFR.MPQ",
	"frFR/expansion-locale-frFR.MPQ",
	"frFR/locale-frFR.MPQ",
	"frFR/expansion-speech-frFR.MPQ",
	"frFR/speech-frFR.MPQ",
];

impl Assets {
	pub fn new() -> Self {
		let archives = ARCHIVES.iter().map(|file| {
			libwow::Archive::open(std::path::Path::new(
				&("./WoW/Data/".to_owned() + file),
			)).expect("failed to open file")
		}).collect();
		let textures = AssetCollection::new();
		let mapobjhandles = AssetCollection::new();
		let mapobjs = AssetCollection::new();
		let mapobjgroups = AssetCollection::new();
		Self {
			archives,
			textures,
			mapobjhandles,
			mapobjs,
			mapobjgroups,
		}
	}

	pub fn get_file(&self, path: &libwow::Path) -> Option<libwow::File> {
		self.archives.iter().find_map(|archive|
			archive.get_file(path).ok()
		)
	}

	pub fn get_texture(
		&self,
		context: &Context,
		path: &libwow::Path,
	) -> Option<TextureAsset> {
		self.textures.get(path, |_| Some(Texture::new(
			&context,
			&libwow::Texture::read(&mut self.get_file(path)?).ok()?,
		)))
	}

	pub fn get_mapobjhandle(
		&self,
		context: &Context,
		modf: &libwow::MODFData,
		path: &libwow::Path,
	) -> Option<MapObjHandleAsset> {
		self.mapobjhandles.get(&modf.id, |_| Some(MapObjHandle::new(
			context,
			self,
			modf,
			path,
		)))
	}

	pub fn get_mapobjgroup(
		&self,
		context: &Context,
		mapobj: &MapObj,
		path: &libwow::Path,
	) -> Option<MapObjGroupAsset> {
		self.mapobjgroups.get(path, |_| Some(MapObjGroup::new(
			&context,
			self,
			mapobj,
			&libwow::MapObjGroup::read(&mut self.get_file(path)?).ok()?,
		)))
	}

	pub fn get_mapobj(
		&self,
		context: &Context,
		path: &libwow::Path,
	) -> Option<MapObjAsset> {
		self.mapobjs.get(path, |_| {
			let mut mapobj = MapObj::new(
				&context,
				self,
				&libwow::MapObj::read(&mut self.get_file(path)?).ok()?,
				path,
			);
			mapobj.load_groups(context, self);
			Some(mapobj)
		})
	}

	pub fn get_wdt(&self, path: &libwow::Path) -> Option<libwow::World> {
		libwow::World::read(&mut self.get_file(path)?).ok()
	}

	pub fn get_adt(&self, path: &libwow::Path) -> Option<libwow::Tile> {
		libwow::Tile::read(&mut self.get_file(path)?).ok()
	}

	pub fn get_wdl(&self, path: &libwow::Path) -> Option<libwow::LowResMap> {
		libwow::LowResMap::read(&mut self.get_file(path)?).ok()
	}
}

impl<K: Hash + Eq + PartialEq + Clone, V> AssetCollection<K, V> {
	fn get<F>(&self, key: &K, value_fn: F) -> Option<Arc<Asset<K, V>>> where F: FnOnce(&K) -> Option<V> {
		let mut assets_locked = self.assets.lock().unwrap();
		if let Some(asset) = assets_locked.get(key).and_then(|asset| asset.upgrade()) {
			return Some(asset);
		}
		let asset = Arc::new(Asset {
			collection: self.assets.clone(),
			key: key.clone(),
			value: value_fn(key)?,
		});
		assets_locked.insert(key.clone(), Arc::downgrade(&asset));
		Some(asset)
	}
}

impl<K: Hash + Eq + PartialEq, V> std::ops::Deref for Asset<K, V> {
	type Target = V;

	fn deref(&self) -> &Self::Target {
		&self.value
	}
}

impl<K: Hash + Eq + PartialEq, V> Drop for Asset<K, V> {
	fn drop(&mut self) {
		self.collection.lock().unwrap().remove(&self.key);
	}
}

impl<K: Hash + Eq + PartialEq, V> AssetCollection<K, V> {
	pub fn new() -> Self {
		Self {
			assets: Arc::new(Mutex::new(HashMap::new())),
		}
	}
}
