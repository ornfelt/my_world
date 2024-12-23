#version 450

in fs_block
{
	vec3 fs_color;
	vec2 fs_uv;
};

layout(location = 0) out vec4 fragcolor;

layout(binding = 0) uniform sampler2D tex;

void main()
{
	vec4 tex_col = texture(tex, fs_uv);
	vec4 col = tex_col * vec4(fs_color, 1);
	fragcolor = col;
}
