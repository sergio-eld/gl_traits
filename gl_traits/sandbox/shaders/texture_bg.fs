#version 330 core 

in vec2 tex_coord;
out vec4 color;

uniform sampler2D texBG;

void main(void)
{
	color = texture(texBG, vec2(tex_coord.x, 1 - tex_coord.y));
}