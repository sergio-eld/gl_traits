
#include "gl_traits.hpp"
#include "helpers.h"

#include <streambuf>
#include <fstream>

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
    glt::LoadOpenGL(glfw.GetOpenGLLoader());

	std::fstream fileSource{ fsys::path(path).append("shaders/vshader.vs"),
		std::fstream::in };

	if (!fileSource)
	{
		std::cerr << "Failed to open vertex shader source file!" << std::endl;
		return -1;
	}

	std::string vSource{ std::istreambuf_iterator<char>(fileSource),
		std::istreambuf_iterator<char>() };

	fileSource.close();
	fileSource.open(fsys::path(path).append("shaders/fshader.fs"), std::fstream::in);

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

    prog.Use();

    constexpr glm::mat4 m{ 42 };
    int sampler1 = 1,
        sampler2 = 2;

    prog.Set(glt::glsl_cast<view_mat4>(m));
    prog.Set(glt::glsl_cast<model_mat4>(m));
    prog.Set(glt::glsl_cast<projection_mat4>(m));

    prog.Set(glt::glsl_cast<texture1_sampler2D>(sampler1));
    prog.Set(glt::glsl_cast<texture2_sampler2D>(sampler2));

    int test_res = 0;

    if (prog.Uniform(glt::tag_t<model_mat4>()).Get() != m)
        test_res |= 2;
    assert(!test_res && "model_mat4 get failed!");

    if (prog.Uniform(glt::tag_t<view_mat4>()).Get() != m)
        test_res |= 4;
    assert(!test_res && "view_mat4 get failed!");

    if (prog.Uniform(glt::tag_t<projection_mat4>()).Get() != m)
        test_res |= 6;
    assert(!test_res && "projection_mat4 get failed!");

    if (prog.Uniform(glt::tag_t<texture1_sampler2D>()).Get() != sampler1)
        test_res |= 8;
    assert(!test_res && "texture1_sampler2D get failed!");

    if (prog.Uniform(glt::tag_t<texture2_sampler2D>()).Get() != sampler2)
        test_res |= 10;
    assert(!test_res && "texture2_sampler2D get failed!");

    return test_res;
}