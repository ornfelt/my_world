use super::{
	Result,
	Error,
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

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct ArrayDef {
	pub count: u32,
	pub offset: u32,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
pub struct Range {
	minimum: u32,
	maximum: u32,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
pub struct Bounds {
	pub aabb0: [f32; 3],
	pub aabb1: [f32; 3],
	pub radius: f32,
}

pub struct Spline<T> {
	pub value: T,
	pub in_tan: T,
	pub out_tan: T,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
pub struct Sequence {
	pub id: u16,
	pub variation_index: u16,
	pub start: u32,
	pub end: u32,
	pub movespeed: f32,
	pub flags: u32,
	pub frequency: i16,
	pub padding: u16,
	pub reply: Range,
	pub blend_time: u32,
	pub bounds: Bounds,
	pub variation_next: i16,
	pub alias_next: i16,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
pub struct PlayableAnimation {
	pub id: u16,
	pub flags: u16,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct TrackBaseDef {
	interpolation_type: u16,
	global_sequence: i16,
	interpolation_ranges: ArrayDef, /* Range */
	timestamps: ArrayDef, /* u32 */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct TrackDef {
	base: TrackBaseDef,
	values: ArrayDef,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct BoneDef {
	key_bone_id: i32,
	flags: u32,
	parent_bone: i16,
	submesh_id: u16,
	bone_name_crc: u32,
	translation: TrackDef, /* [f32; 3] */
	rotation: TrackDef, /* [i16; 4] */
	scale: TrackDef, /* [f32; 3] */
	pivot: [f32; 3],
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
pub struct Vertex {
	pub pos: [f32; 3],
	pub bone_weights: [u8; 4],
	pub bone_indices: [u8; 4],
	pub normal: [f32; 3],
	pub tex_coords: [[f32; 2]; 2],
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
pub struct SkinSection {
	pub skin_section_id: u16,
	pub level: u16,
	pub vertex_start: u16,
	pub vertex_count: u16,
	pub index_start: u16,
	pub index_count: u16,
	pub bone_count: u16,
	pub bone_combo_index: u16,
	pub bone_influences: u16,
	pub center_bone_idnex: u16,
	pub center_position: [f32; 3],
	pub sort_center_position: [f32; 3],
	pub sort_radius: f32,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
pub struct Batch {
	pub flags: u8,
	pub priority_plane: i8,
	pub shader_id: i16,
	pub skin_section_index: u16,
	pub geoset_index: u16,
	pub color_index: u16,
	pub material_index: u16,
	pub material_layer: u16,
	pub texture_count: u16,
	pub texture_combo_index: u16,
	pub texture_coord_combo_idex: u16,
	pub texture_weight_combo_index: u16,
	pub texture_transform_combo_index: u16,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct SkinProfileDef {
	vertexes: ArrayDef, /* u16 */
	indices: ArrayDef, /* u16 */
	bones: ArrayDef, /* u32 */
	sections: ArrayDef, /* SkinSection */
	batches: ArrayDef, /* Batch */
	bone_count_max: u32,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct ColorDef {
	color: TrackDef, /* [f32; 3] */
	alpha: TrackDef, /* i16 */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct TextureDef {
	_type: u32,
	flags: u32,
	filename: ArrayDef, /* u8 */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct TextureWeightDef {
	weight: TrackDef, /* i16 */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct TextureFlipbookDef {
	flipbook: TrackDef, /* i16 */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct TextureTransformDef {
	translation: TrackDef, /* [f32; 3] */
	rotation: TrackDef, /* [f32; 4] */
	scale: TrackDef, /* [f32; 3] */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
pub struct Material {
	flags: u16,
	blend_mode: u16,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct AttachmentDef {
	id: u32,
	bone: u16,
	unknown: u16,
	position: [f32; 3],
	animate_attached: TrackDef, /* u8 */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct EventDef {
	id: u32,
	data: u32,
	bone: u32,
	position: [f32; 3],
	enabled: TrackBaseDef,
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct LightDef {
	_type: u16,
	bone: i16,
	position: [f32; 3],
	ambient_color: TrackDef, /* [f32; 3] */
	ambient_intensity: TrackDef, /* f32 */
	diffuse_color: TrackDef, /* [f32; 3] */
	diffuse_intensity: TrackDef, /* f32 */
	attenuation_start: TrackDef, /* f32 */
	attenuation_end: TrackDef, /* f32 */
	visibility: TrackDef, /* u8 */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct CameraDef {
	_type: u32,
	fov: f32,
	far_clip: f32,
	near_clip: f32,
	position: TrackDef, /* Spline<[f32; 3]> */
	position_base: [f32; 3],
	target_position: TrackDef, /* Spline<[f32; 3]> */
	target_position_base: [f32; 3],
	roll: TrackDef, /* Spline<f32> */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct RibbonDef {
	id: u32,
	bone_index: u32,
	position: [f32; 3],
	texture_indices: ArrayDef, /* u16 */
	material_indices: ArrayDef, /* u16 */
	color: TrackDef, /* [f32; 3] */
	alpha: TrackDef, /* i16 */
	height_above: TrackDef, /* f32 */
	height_below: TrackDef, /* f32 */
	edges_per_second: f32,
	edge_lifetime: f32,
	gravity: f32,
	texture_rows: u16,
	texture_cols: u16,
	tex_slot: TrackDef, /* u16 */
	visibility: TrackDef, /* u8 */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct ParticleDef {
	id: u32,
	flags: u32,
	position: [f32; 3],
	bone: u16,
	texture: u16,
	geometry_model_filename: ArrayDef, /* u8 */
	recursion_model_filename: ArrayDef, /* u8 */
	blending_type: u16,
	emitter_type: u16,
	particle_type: u8,
	head_or_tail: u8,
	texture_tile_rotation: i16,
	texture_dimensions_rows: u16,
	texture_demensions_columns: u16,
	emission_speed: TrackDef, /* f32 */
	speed_variation: TrackDef, /* f32 */
	vertical_range: TrackDef, /* f32 */
	horizontal_range: TrackDef, /* f32 */
	gravity: TrackDef, /* f32 */
	lifespan: TrackDef, /* f32 */
	emission_rate: TrackDef, /* f32 */
	emission_area_length: TrackDef, /* f32 */
	emission_area_width: TrackDef, /* f32 */
	z_source: TrackDef, /* f32 */
	mid_point: f32,
	color_values: [[u8; 4]; 3],
	scale_values: [f32; 3],
	lifespan_uv_anim: [u16; 3],
	decay_uv_anim: [u16; 3],
	tail_uv_anim: [i16; 2],
	tail_decay_uv_anim: [i16; 2],
	tail_length: f32,
	twinkle_speed: f32,
	twinkle_percent: f32,
	twinkle_scale_min: f32,
	twinkle_scale_max: f32,
	burst_multiplier: f32,
	drag: f32,
	spin: f32,
	tumble: [[f32; 3]; 2],
	wind_vector: [f32; 3],
	wind_time: f32,
	follow_speed1: f32,
	follow_scale1: f32,
	follow_speed2: f32,
	follow_scale2: f32,
	spline_points: ArrayDef, /* [f32; 3] */
	enabled_in: TrackDef, /* u8 */
}

#[repr(C)]
#[derive(Copy, Clone, Debug, Pod, Zeroable)]
struct HeaderDef {
	magic : u32,
	version: u32,
	name: ArrayDef, /* u8 */
	global_sequences: ArrayDef, /* u32 */
	sequences: ArrayDef, /* Sequence */
	sequence_lookup_table: ArrayDef, /* u16 */
	playable_animations: ArrayDef, /* PlayableAnimation */
	bones: ArrayDef, /* BoneDef */
	key_bone_lookup_table: ArrayDef, /* u16 */
	vertexes: ArrayDef, /* Vertex */
	skin_profiles: ArrayDef, /* SkinProfileDef */
	colors: ArrayDef, /* ColorDef */
	textures: ArrayDef, /* TextureDef */
	texture_weights: ArrayDef, /* TextureWeightDef */
	texture_flipbooks: ArrayDef, /* TextureFlipbookDef */
	texture_transforms: ArrayDef, /* TextureTransformDef */
	replacable_texture_lookup: ArrayDef, /* u16 */
	materials: ArrayDef, /* Material */
	bone_lookup_table: ArrayDef, /* u16 */
	texture_lookup_table: ArrayDef, /* u16 */
	texture_unit_lookup_table: ArrayDef, /* u16 */
	texture_weights_lookup_table: ArrayDef, /* u16 */
	texture_transforms_lookup_table: ArrayDef, /* u16 */
	aabb0: [f32; 3],
	aabb1: [f32; 3],
	bounding_sphere_radius: f32,
	caabb0: [f32; 3],
	caabb1: [f32; 3],
	collision_sphere_radius: f32,
	collision_triangles: ArrayDef, /* u16 */
	collision_vertexes: ArrayDef, /* [f32; 3] */
	collision_normals: ArrayDef, /* [f32; 3] */
	attachments: ArrayDef, /* AttachmentDef */
	attachment_lookup_table: ArrayDef, /* u16 */
	events: ArrayDef, /* EventDef */
	lights: ArrayDef, /* LightDef */
	cameras: ArrayDef, /* CameraDef */
	camera_lookup_table: ArrayDef, /* u16 */
	ribbons: ArrayDef, /* RibbonDef */
	particles: ArrayDef, /* ParticleDef */
	texture_combiner_combos: ArrayDef, /* u16 */
}

pub struct Model {
}
