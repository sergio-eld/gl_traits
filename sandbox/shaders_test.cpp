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

	static_assert(glt::FetchedAttrib<glm::vec3>::size == glt::FetchedAttrib<aPos_vec3>::size);
	static_assert(glt::FetchedAttrib<glm::vec3>::glType == glt::FetchedAttrib<aPos_vec3>::glType);


	constexpr glt::FetchedAttrib<glm::vec3> v3{};
	constexpr glt::FetchedAttrib<aPos_vec3> v3pos{};

	constexpr  glt::FetchedAttrib<glm::vec3> v31{ glt::FetchedAttrib<aPos_vec3>()};
	constexpr  glt::FetchedAttrib<aPos_vec3> v31pos{ glt::FetchedAttrib<glm::vec3>() };

    static_assert(std::is_same_v<glt::FetchedAttrib<glm::vec3>::ConvType<glm::vec3>,
        glt::FetchedAttrib<glm::vec3>>);
    static_assert(std::is_same_v<glt::FetchedAttrib<glm::vec3>::ConvType<aPos_vec3>,
        glt::FetchedAttrib<aPos_vec3>>);

	glt::Buffer<glm::vec3> sbuf;
	sbuf.Bind(glt::BufferTarget::array);
	glt::Buffer<glm::vec3> sbuf2{ std::move(sbuf) };

	sbuf = std::move(sbuf2);

	sbuf.AllocateMemory(36, glt::BufUsage::static_draw);
	

	glt::Buffer2<glm::vec3, glm::vec2, float> buf;
    buf.Bind(glt::BufferTarget::array);
	buf.AllocateMemory(1, 2, 3, glt::BufUsage::static_draw);

    glt::FetchedAttrib<glm::vec2> attr = buf.Fetch(glt::tag_s<1>());

    glt::Buffer2<glm::vec2, glt::compound<glm::vec3, glm::vec2, float>> bufComp;
    bufComp.Bind(glt::BufferTarget::array);
    bufComp.AllocateMemory(16, 24, glt::BufUsage::static_draw);

    static_assert(std::is_same_v<glt::fetch_type<glt::compound<glm::vec3, glm::vec2, float>>, glm::vec3>);
    static_assert(!std::is_same_v<glt::fetch_type<glt::compound<glm::vec3, glm::vec2, float>>, glm::vec2>);

    glt::FetchedAttrib<glm::vec3> attr2 = bufComp.Fetch(glt::tag_s<1>());
    glt::FetchedAttrib<aPos_vec3> attr21 = bufComp.Fetch(glt::tag_s<1>());

    static_assert(std::is_constructible_v<glt::FetchedAttrib<glm::vec3>, glt::FetchedAttrib<glm::vec3>>);
    static_assert(std::is_constructible_v<glt::FetchedAttrib<glm::vec3>, glt::FetchedAttrib<aPos_vec3>>);
    static_assert(std::is_constructible_v<glt::FetchedAttrib<glm::vec2>, glt::FetchedAttrib<aPos_vec3>>); // false

    //glt::FetchedAttrib<aPos_vec3> attr2{ bufComp.Fetch(glt::tag_s<0>()) };


    //glt::FetchedAttrib<glm::vec2> attr = buf

	return 0;
}