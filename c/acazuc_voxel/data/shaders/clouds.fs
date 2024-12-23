#version 450

in fs_block
{
	vec4 fs_position;
	vec3 fs_color;
};

layout(location = 0) out vec4 fragcolor;

layout(std140, binding = 1) uniform model_block
{
	mat4 mvp;
	mat4 mv;
	vec4 fog_color;
	float fog_distance;
	float fog_density;
};

void main()
{
	vec4 col = vec4(fs_color, .8);
	float dist = length(fs_position);
	float tmp = max(0, dist - fog_distance);
	float fog = clamp(exp(-tmp * fog_density), 0, 1);
	fragcolor = mix(col, fog_color, 1 - fog);
}
