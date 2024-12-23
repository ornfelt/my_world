struct VertexInput {
	@location(0) position: vec4<f32>,
	@location(1) normal: vec4<f32>,
	@location(2) uv: vec2<f32>,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) uv: vec2<f32>,
	@location(1) diffuse: f32,
	@location(2) mv_position: vec4<f32>,
	@location(3) mv_normal: vec4<f32>,
	@location(4) mv_light_dir: vec4<f32>,
};

struct FragmentOutput {
	@location(0) color: vec4<f32>,
};

struct SceneBlock {
	mvp: mat4x4<f32>,
	mv: mat4x4<f32>,
};

struct ModelBlock {
	mvp: mat4x4<f32>,
	mv: mat4x4<f32>,
	m: mat4x4<f32>,
};

struct MeshBlock {
	dummy: u32,
};

@binding(0) @group(0) var<uniform> scene_block: SceneBlock;
@binding(0) @group(1) var<uniform> model_block: ModelBlock;
@binding(0) @group(2) var<uniform> mesh_block: MeshBlock;
@binding(1) @group(2) var texture_sampler: sampler;
@binding(2) @group(2) var texture: texture_2d<f32>;

const diffuse_color = vec3(0.6);
const ambient_color = vec3(0.4);
const specular_color = vec4(0.0);
const light_dir = vec4(-0.57735, -0.57735, -0.57735, 0.0);
const light_color = vec4(0.7);

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	var normal = normalize(in.normal.xyz);
	var normal4 = vec4(normal, 0.0);
	out.position = model_block.mvp * in.position;
	out.uv = in.uv;
	out.diffuse = clamp(dot((model_block.m * normal4).xyz, -light_dir.xyz), 0.0, 1.0);
	out.mv_position = model_block.mv * in.position;
	out.mv_normal = model_block.mv * normal4;
	out.mv_light_dir = model_block.mv * light_dir;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
	var out: FragmentOutput;
	let color = vec3(vec3(in.diffuse) + ambient_color);
	var specular = pow(clamp(dot(normalize(-in.mv_position), reflect(in.mv_light_dir, in.mv_normal)), 0.0, 1.0), 10.0);
	var tex = textureSample(texture, texture_sampler, in.uv);
	out.color = vec4(color * tex.xyz + specular * specular_color.xyz, tex.a);
	return out;
}
