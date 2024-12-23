struct VertexInput {
	@location(0) position: vec4<f32>,
	@location(1) uv: vec2<f32>,
	@location(2) depth: f32,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) uv: vec2<f32>,
	@location(1) color: vec4<f32>,
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

@binding(0) @group(0) var<uniform> scene_block: SceneBlock;
@binding(1) @group(0) var texture_sampler: sampler;
@binding(2) @group(0) var texture: texture_2d<f32>;

const base_color = vec4(0.5, 0.5, 0.6, 0.4);
const final_color = vec4(0.0, 0.0, 0.5, 0.7);
const specular_color = vec4(1.0);
const light_dir = vec4(-0.57735, -0.57735, -0.57735, 0.0);
const light_color = vec4(0.7);

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	var normal = vec3(0.0, 1.0, 0.0);
	out.position = scene_block.mvp * in.position;
	out.uv = in.uv;
	var depth_factor = min(in.depth * 4.0, 1.0);
	out.color = mix(base_color, final_color, depth_factor);
	out.mv_position = scene_block.mv * in.position;
	out.mv_normal = scene_block.mv * vec4(normal, 0.0);
	out.mv_light_dir = scene_block.mv * light_dir;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
	var out: FragmentOutput;
	var texture_color = textureSample(texture, texture_sampler, in.uv);
	var sf = pow(clamp(dot(normalize(-in.mv_position), normalize(reflect(in.mv_light_dir, in.mv_normal))), 0.0, 1.0), 10.0);
	out.color = vec4(in.color.xyz + texture_color.xyz * (1.0 + specular_color.xyz * sf * 20.0), in.color.a);
	return out;
}
