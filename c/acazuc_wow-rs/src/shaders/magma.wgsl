struct VertexInput {
	@location(0) position: vec4<f32>,
	@location(1) uv: vec2<f32>,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) uv: vec2<f32>,
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

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.position = scene_block.mvp * in.position;
	out.uv = in.uv;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
	var out: FragmentOutput;
	out.color = textureSample(texture, texture_sampler, in.uv);
	return out;
}
