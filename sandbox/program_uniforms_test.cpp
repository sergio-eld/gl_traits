#include "helpers.hpp"

#include <streambuf>

#include "glt_Common.h"
#include "glt_CommonValidate.h"

using unif_collect = glt::uniform_collection<std::tuple<model_mat4,
	view_mat4,
	projection_mat4,
	texture1_sampler2D,
	texture2_sampler2D>>;

int main(int argc, const char** argv)
{
	fsys::path path = fsys::path(argv[0]).parent_path();

	SmartGLFW glfw{ 3, 3 };
	SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "test program and uniforms" };

	glfw.MakeContextCurrent(window);
	glfw.LoadOpenGL();

	std::fstream fileSource{ fsys::path(path).append("vshader.vs"),
		std::fstream::in };

	if (!fileSource)
	{
		std::cerr << "Failed to open vertex shader source file!" << std::endl;
		return -1;
	}

	std::string vSource{ std::istreambuf_iterator<char>(fileSource),
		std::istreambuf_iterator<char>() };

	fileSource.close();
	fileSource.open(fsys::path(path).append("fshader.fs"), std::fstream::in);

	if (!fileSource)
	{
		std::cerr << "Failed to open fragment shader source file!" << std::endl;
		return -1;
	}

	std::string fSource{ std::istreambuf_iterator<char>(fileSource),
		std::istreambuf_iterator<char>() };

	glt::VertexShader vShader{ vSource };
	glt::FragmentShader fShader{ fSource };

	glt::Program<VAO_vshader, unif_collect> 
		prog{glt::Allocator::Allocate(glt::ProgramTarget()), vShader, fShader };


	return 0;
}