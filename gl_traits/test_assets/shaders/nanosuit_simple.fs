#version 330 core

in vec2 texCoords;

out vec4 FragColor;

uniform sampler2D texture_diffuse;

void main(void)
{
	FragColor = texture(texture_diffuse, texCoords);
}