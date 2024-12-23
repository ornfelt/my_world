struct VertexInput {
	@builtin(vertex_index) idx: u32,
	@location(0) position: vec4<f32>,
	@location(1) normal: vec4<f32>,
	@location(2) uv : vec2<f32>,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) diffuse: f32,
	@location(1) uv: vec2<f32>,
	@location(2) chunk_id: u32,
	@location(3) mv_position: vec4<f32>,
	@location(4) mv_normal: vec4<f32>,
	@location(5) mv_light_dir: vec4<f32>,
};

struct FragmentOutput {
	@location(0) color: vec4<f32>,
};

struct SceneBlock {
	mvp: mat4x4<f32>,
	mv: mat4x4<f32>,
};

struct ModelBlock {
	textures: array<array<vec4<u32>, 2>, 256>,
};

@binding(0) @group(0) var<uniform> scene_block: SceneBlock;
@binding(0) @group(1) var<uniform> model_block: ModelBlock;
@binding(1) @group(1) var alpha_map_sampler: sampler;
@binding(2) @group(1) var alpha_map_texture: texture_2d_array<f32>;
@binding(3) @group(1) var textures_sampler: sampler;
@binding(0) @group(2) var textures: binding_array<texture_2d<f32>>;

const diffuse_color = vec3(0.6);
const ambient_color = vec3(0.4);
const specular_color = vec4(1.0);
const light_dir = vec4(-0.57735, -0.57735, -0.57735, 0.0);
const light_color = vec4(0.7);

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	var normal = normalize(in.normal.xyz);
	out.position = scene_block.mvp * in.position;
	out.diffuse = clamp(dot(normal, -light_dir.xyz), 0.0, 1.0);
	out.uv = in.uv;
	out.chunk_id = in.idx / u32(145);
	out.mv_position = scene_block.mv * in.position;
	out.mv_normal = scene_block.mv * vec4(normal, 0.0);
	out.mv_light_dir = scene_block.mv * light_dir;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
	var out: FragmentOutput;
	var alpha = textureSample(alpha_map_texture, alpha_map_sampler, in.uv, in.chunk_id);
	var uv = in.uv * 8.0;
	var d1 = textureSample(textures[model_block.textures[in.chunk_id][0].x], textures_sampler, uv).rgb;
	var d2 = textureSample(textures[model_block.textures[in.chunk_id][0].y], textures_sampler, uv).rgb;
	var d3 = textureSample(textures[model_block.textures[in.chunk_id][0].z], textures_sampler, uv).rgb;
	var d4 = textureSample(textures[model_block.textures[in.chunk_id][0].w], textures_sampler, uv).rgb;
	var dv = mix(mix(mix(d1, d2, alpha.r), d3, alpha.g), d4, alpha.b);
	var color = dv * (ambient_color + in.diffuse * diffuse_color * (1.0 - alpha.a));
	var s1 = textureSample(textures[model_block.textures[in.chunk_id][1].x], textures_sampler, uv);
	var s2 = textureSample(textures[model_block.textures[in.chunk_id][1].y], textures_sampler, uv);
	var s3 = textureSample(textures[model_block.textures[in.chunk_id][1].z], textures_sampler, uv);
	var s4 = textureSample(textures[model_block.textures[in.chunk_id][1].w], textures_sampler, uv);
	var sv = mix(mix(mix(s1, s2, alpha.r), s3, alpha.g), s4, alpha.b);
	var sf = pow(clamp(dot(normalize(-in.mv_position), reflect(in.mv_light_dir, in.mv_normal)), 0.0, 1.0), 10.0);
	var specular = sv.rgb * sv.a * sf * 2.0 * (1.0 - alpha.a);
	out.color = vec4(color + specular * specular_color.xyz, 1.0);
	return out;
}
