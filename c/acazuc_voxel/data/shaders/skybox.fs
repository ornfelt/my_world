#version 450

in fs_block
{
	vec3 fs_color;
};

layout(location = 0) out vec4 fragcolor;

void main()
{
	vec4 col = vec4(fs_color, 1);
	fragcolor = col;
}
