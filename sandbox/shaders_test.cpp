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

    VAO_vshader vs{};

    //vs.EnablePointer(glt::tag_s<0>());

	vs.Bind();
    vs.EnablePointer(1);

	static_assert(glt::AttribPtr<glm::vec3>::size == glt::AttribPtr<aPos_vec3>::size);
	static_assert(glt::AttribPtr<glm::vec3>::glType == glt::AttribPtr<aPos_vec3>::glType);

    /*
	constexpr glt::AttribPtr<glm::vec3> v3{};
	constexpr glt::AttribPtr<aPos_vec3> v3pos{};

	constexpr  glt::AttribPtr<glm::vec3> v31{ glt::AttribPtr<aPos_vec3>()};
	constexpr  glt::AttribPtr<aPos_vec3> v31pos{ glt::AttribPtr<glm::vec3>() };
    */


	glt::Buffer<glm::vec3> sbuf;
	sbuf.Bind(glt::BufferTarget::array);
    sbuf.AllocateMemory(36, glt::BufUsage::static_draw);

    const glt::Sequence<glm::vec3>& cseqVec3 = sbuf;
    glt::Sequence<glm::vec3>& seqVec3 = sbuf;
    glt::AttribPtr<glm::vec3> attr = seqVec3;

    glt::Buffer<glt::compound<glm::vec3, glm::vec2, float>> sbufCmp;
   // sbufCmp.Bind(glt::BufferTarget::array);
    sbufCmp.AllocateMemory(36, glt::BufUsage::static_draw);

    const glt::Sequence<glm::vec3, glm::vec2, float>& cseqCmp = sbufCmp;
    glt::Sequence<glm::vec3, glm::vec2, float>& seqCmp = sbufCmp;

    sizeof(glt::Buffer_base);
    sizeof(glt::Buffer<glm::vec3>);

	return 0;
}