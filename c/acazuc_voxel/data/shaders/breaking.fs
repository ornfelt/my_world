#version 450

in fs_block
{
	vec3 fs_position;
	vec2 fs_uv;
};


layout(location = 0) out vec4 fragcolor;

layout(binding = 0) uniform sampler2D tex;

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
	vec4 tex_col = texture(tex, fs_uv);
	if (tex_col.r == 127. / 255.)
		discard;
	float dist = length(fs_position);
	float tmp = max(0, dist - fog_distance);
	float fog = clamp(exp(-tmp * fog_density), 0, 1);
	fragcolor = mix(tex_col, fog_color, 1 - fog);
}