#version 330 core

layout (location = 0) in vec3 attrPosXYZ;
layout (location = 1) in vec3 attrNormals;
layout (location = 2) in vec3 attrTexCoords;

out vec2 texCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void)
{
	textureCoords = attrTexCoords.xy;
	glPosition = projection * view * model * vec4(posXYZ, 1.0f);
}