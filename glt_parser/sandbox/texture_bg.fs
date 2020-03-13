#version 330 core 

in vec2 tex_coord;

uniform sampler2D texBG;

out vec2 color;

void main(void)
{
	color = texture(texBG, tex_coord);
}