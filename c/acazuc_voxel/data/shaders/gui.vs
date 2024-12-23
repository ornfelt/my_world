#version 450

layout(location = 0) in vec2 vs_position;
layout(location = 1) in vec4 vs_color;
layout(location = 2) in vec2 vs_uv;

out fs_block
{
	vec4 fs_color;
	vec2 fs_uv;
};

layout(std140, binding = 1) uniform model_block
{
	mat4 mvp;
};

void main()
{
	vec4 position = vec4(vs_position, 0, 1);
	fs_uv = vs_uv;
	fs_color = vs_color;
	gl_Position = mvp * position;
}
