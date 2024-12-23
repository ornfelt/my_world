#version 450

in fs_block
{
	vec4 fs_color;
	vec2 fs_uv;
};

layout(location = 0) out vec4 fragcolor;

layout(binding = 0) uniform sampler2D tex;

void main()
{
	vec4 tex_col = texture(tex, fs_uv);
	vec4 col = fs_color * tex_col;
	if (col.a == 0)
		discard;
	fragcolor = col;
}
