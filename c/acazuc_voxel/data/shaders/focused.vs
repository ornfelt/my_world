#version 450

layout(location = 0) in vec3 vs_position;

layout(std140, binding = 1) uniform model_block
{
	mat4 mvp;
};

void main()
{
	vec4 position = vec4(vs_position, 1);
	gl_Position = mvp * position;
}
