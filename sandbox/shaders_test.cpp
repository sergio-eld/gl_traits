#include "helpers.hpp"

// TODO: add generated shader headers files
/*
Module testing:
1. generate glt_Common from a list of shader files
2. generate code for N shader sources specified in the list:
- write a template list of pairs "FileName - Shader-Type"
- write a function that will unfold that list into a sequence of "test" function invokations
3. Test "TRY_COMPILE"
4. Run Test. Return int flag result (all bits = 0 if success)
*/

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

	//glt::Shader<glt::ShaderTarget::vertex> vertexShader{ vertexSource };
    //Shader_vshader vertexShader{ vertexSource };

    class VAO_test : //  glt::VAO_base,
        glt::VAO_attrib<0, glm::vec3>
    {
    public:
        VAO_test(glt::HandleVAO&& handle)
            : VAO_base(std::move(handle))
        {}

        using glt::VAO_attrib<0, glm::vec3>::EnablePointer;
        using glt::VAO_attrib<0, glm::vec3>::AttributePointer;

    };

    class VAO_test2 : // glt::VAO_base,
        glt::VAO_attrib<0, aPos_vec3>
    {
    public:
        VAO_test2(glt::HandleVAO&& handle)
            : VAO_base(std::move(handle))
        {}

        using glt::VAO_attrib<0, aPos_vec3>::EnablePointer;
        using glt::VAO_attrib<0, aPos_vec3>::AttributePointer;

    };

    VAO_test vt{ glt::Allocator::Allocate(glt::VAOTarget()) };
    VAO_test2 vt2{ glt::Allocator::Allocate(glt::VAOTarget()) };


    VAO_vshader vs{};

    vs.EnablePointer(glt::tag_s<0>());

    vs.EnablePointer(1);

    vt.EnablePointer(glt::tag_s<0>());
    vt.AttributePointer(glt::tag_s<0>(), glt::VertexAttrib<glm::vec3>(0));
    //vt.EnablePointer(glt::tag_t<glm::vec3>());
    vt2.EnablePointer(glt::tag_s<0>());
    vt2.EnablePointer(glt::tag_t<aPos_vec3>());
    vt2.AttributePointer(glt::tag_s<0>(), glt::VertexAttrib<glm::vec3>(0));
    vt2.AttributePointer(glt::VertexAttrib<aPos_vec3>(0));



	return 0;
}