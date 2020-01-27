#version 330 core

layout (location = 0) in vec4 texXY_TS;

out vec2 tex_coord;

void main(void)
{
	gl_Position = vec4(texXY_TS.xy, 0, 1);
	tex_coord = texXY_TS.zw;
}
