#include "helpers.hpp"

// TODO: add generated shader headers files

#include "glt_Common.h"

#include <fstream>
#include <streambuf>


int main(int argc, const char *argv[])
{
	std::filesystem::path exePath{ argv[0] };

	SmartGLFW glfw{ 3, 3 };
	SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "Test shaders" };

	glfw.MakeContextCurrent(window);
	glfw.LoadOpenGL();

	std::fstream f{ exePath.parent_path().append("vshader.vs"), std::fstream::in };

	std::string vertexSource{ std::istreambuf_iterator<char>(f),
	std::istreambuf_iterator<char>() };

	if (!glt::shader_traits::check_source(vertexSource))
	{
		std::cerr << "check_source default True expected" << std::endl;
		return -1;
	}

	if (!glt::shader_traits::check_source<std::tuple<aPos_vec3, aTexCoord_vec2>,
		std::tuple<TexCoord_vec2>>(vertexSource))
	{
		std::cerr << "check_source True expected!" << std::endl;
		return -1;
	}

    glt::identical_sets_v<std::tuple<>, std::tuple<>>;

	glt::Shader<glt::ShaderTarget::vertex> vertexShader{ vertexSource };

	return 0;
}