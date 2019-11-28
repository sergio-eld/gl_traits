#include "gl_traits.hpp"

/*

GLuint shader_traits::GenShaderPrivate(ShaderTarget target)
{
	return glCreateShader((GLenum)target);
}

bool shader_traits::CompileStatusPrivate(GLuint handle)
{
	int success;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
	return (bool)success;
}

bool shader_traits::CompileShaderPrivate(GLuint handle, const std::string & source)
{
	const char* c_src = source.c_str();
	glShaderSource(handle, 1, &c_src, 0);
	glCompileShader(handle);

	return CompileStatusPrivate(handle);
}

void shader_traits::AttachShaderPrivate(GLuint prog, GLuint shader)
{
	glAttachShader(prog, shader);
}

std::string shader_traits::ShaderInfoLogPrivate(GLuint shader)
{
	GLsizei returned = 0;
	char buffer[512];

	glGetShaderInfoLog(shader, 512, &returned, buffer);
	return std::string(std::begin(buffer), std::end(buffer));
}

Handle<glProgramTarget::program> shader_traits::GenProgram()
{
	return glCreateProgram();
}

void shader_traits::LinkProgram(const Handle<glProgramTarget::program>& prog)
{
	glLinkProgram(prog);
}

bool shader_traits::LinkStatus(const Handle<glProgramTarget::program>& prog)
{
	int success = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	return (bool)success;
}
*/