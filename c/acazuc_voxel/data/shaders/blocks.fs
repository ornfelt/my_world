#version 450

in fs_block
{
	vec3 fs_position;
	vec3 fs_color;
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
	float time_factor;
	int disable_tex;
};

void main()
{
	vec4 tex_col = texture(tex, fs_uv);
	if (disable_tex == 1)
	{
		tex_col.rgb = vec3(1, 1, 1);
	}
	else
	{
		//float rand = fract(sin(dot(fs_position.xy, vec2(12.9898, 78.233 * time_factor))) * 43758.5453);
		//tex_col = vec4(rand, rand, rand, 1);
		//tex_col = texture2D(tex, fs_uv);
	}
	if (tex_col.a == 0)
		discard;
	vec4 col = tex_col * vec4(fs_color, 1);
	float dist = length(fs_position);
	float tmp = max(0, dist - fog_distance);
	float fog = clamp(exp(-tmp * fog_density), 0, 1);
	fragcolor = mix(col, fog_color, 1 - fog);
}