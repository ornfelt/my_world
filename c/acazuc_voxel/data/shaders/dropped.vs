#version 450

layout(location = 0) in vec3 vs_position;
layout(location = 1) in vec3 vs_color;
layout(location = 2) in vec2 vs_uv;

out fs_block
{
	vec3 fs_position;
	vec3 fs_color;
	vec2 fs_uv;
};

layout(std140, binding = 1) uniform model_block
{
	mat4 mvp;
	mat4 mv;
	vec4 fog_color;
	float fog_distance;
	float fog_density;
	int disable_tex;
};

void main()
{
	vec4 position = vec4(vs_position, 1);
	fs_uv = vs_uv;
	fs_color = vs_color;
	fs_position = (mv * position).xyz;
	gl_Position = mvp * position;
}