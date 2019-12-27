#include "helpers.hpp"

// TODO: add generated shader headers files

#include "glt_Common.h"

#include <fstream>
#include <streambuf>


int main(int argc, const char *argv[])
{

	static_assert(!glt::has_location_v<aPos_vec3>);
	using VAOaPos_vec3 = glt::add_qualifiers<aPos_vec3, glt::q_location<1>>;
	static_assert(glt::has_location_v<VAOaPos_vec3>);
	static_assert(glt::has_name_v<VAOaPos_vec3>);
	static_assert(glt::has_type_v<VAOaPos_vec3>);
	static_assert(sizeof(aPos_vec3) == sizeof(VAOaPos_vec3));

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