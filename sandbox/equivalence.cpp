// this unit tests for equivalence
// Run compilation test
#include "helpers.hpp"



int main()
{

	static_assert(glt::is_initializable_from_v<glm::vec3, glm::vec3>);

	
	static_assert(!std::is_same_v<glm::vec3, std::tuple<glm::vec3>>);
	static_assert(glt::is_equivalent_v<glm::vec3, std::tuple<glm::vec3>>);
	static_assert(glt::is_equivalent_v<glm::vec3, glt::compound<glm::vec3>>);

	static_assert(!std::is_same_v<std::tuple<glm::vec3>, glm::vec3>);
	static_assert(glt::is_equivalent_v<std::tuple<glm::vec3>, glm::vec3>);
	static_assert(glt::is_equivalent_v<glt::compound<glm::vec3>, glm::vec3>);

	static_assert(std::is_same_v<std::tuple<glm::vec3, glm::vec2>, 
		std::tuple<glm::vec3, glm::vec2>>);
	static_assert(!std::is_same_v<std::tuple<glm::vec3, glm::vec2>,
		std::tuple<glm::vec2, glm::vec3>>);

	static_assert(glt::is_equivalent_v<glt::compound<glm::vec3, glm::vec2>,
		glt::compound<glm::vec3, glm::vec2>>);
	static_assert(!glt::is_equivalent_v<glt::compound<glm::vec3, glm::vec2>,
		glt::compound<glm::vec2, glm::vec3>>);

	static_assert(glt::is_equivalent_v<std::tuple<glm::vec3, glm::vec2>,
		std::tuple<glm::vec3, glm::vec2>>);
	static_assert(!glt::is_equivalent_v<std::tuple<glm::vec3, glm::vec2>,
		std::tuple<glm::vec2, glm::vec3>>);

	// case 0
	static_assert(glt::is_equivalent_v<float, float>);
	static_assert(glt::is_equivalent_v<glm::vec3, glm::vec3>);
	static_assert(!glt::is_equivalent_v<glm::vec3, glm::vec2>);

	// case 1
	static_assert(glt::is_equivalent_v<glm::vec3, glt::compound<glm::vec3>>);
	static_assert(!glt::is_equivalent_v<glm::vec3, glt::compound<glm::vec2>>);

	// case 2
	static_assert(!glt::is_equivalent_v<glm::vec3, glt::compound<glm::vec3, glm::vec2>>);

	// case 3*
	static_assert(glt::is_equivalent_v<glt::compound<glm::vec3>, glm::vec3>);

	// case 4:
	static_assert(!glt::is_equivalent_v<glt::compound<glm::vec3, glm::vec2>, glm::vec3>);

	// case 5: Assertion fails for True case w
	static_assert(std::is_same_v<std::tuple<glm::vec3, glm::vec2>,
		std::tuple<glm::vec3, glm::vec2>>);
	static_assert(glt::is_equivalent_v<std::tuple<glm::vec3, glm::vec2>, 
		std::tuple<glm::vec3, glm::vec2>>);
	static_assert(!glt::is_equivalent_v<std::tuple<glm::vec3, glm::vec2>,
		glt::compound<glm::vec3>>);


	// tuples from compounds
	static_assert(std::is_same_v<std::tuple<glm::vec3>,
		glt::recover_tuple<glm::vec3>::type>);	// from simple
	static_assert(std::is_same_v<std::tuple<glm::vec3>,
		glt::recover_tuple<glt::compound<glm::vec3>>::type>);	// from compound
	static_assert(std::is_same_v<std::tuple<glm::vec3, glm::vec2>,
		glt::recover_tuple<glt::compound<glm::vec3, glm::vec2>>::type>);	// from compound


	// tuples from user-defined (vertex) class
	static_assert(std::is_same_v<std::tuple<glm::vec3, glm::vec2>,
		glt::recover_tuple<vertex, std::tuple<glm::vec3, glm::vec2>>::type>);
	static_assert(std::is_same_v<std::tuple<glm::vec3>,
		glt::recover_tuple<vertex, std::tuple<glm::vec3>>::type>); // false positive

	static_assert(glt::is_equivalent_v<std::tuple<glm::vec3, glm::vec2>, vertex>);

	struct abc
	{
		char a;
		long b,
			c;
	};


	static_assert(glt::is_equivalent_v<vertex, vertex>);
	static_assert(!glt::is_equivalent_v<abc, vertex>);

	struct Coords
	{
		glm::vec3 xyz;
	};

	static_assert(glt::is_equivalent_v<glm::vec3, Coords>);

	// check returned sizes
	static_assert(glt::vao_attrib_size<glm::vec1>() == (glt::VAOAttribSize)1);
	static_assert(glt::vao_attrib_size<glm::vec2>() == (glt::VAOAttribSize)2);
	static_assert(glt::vao_attrib_size<glm::vec3>() == (glt::VAOAttribSize)3);
	static_assert(glt::vao_attrib_size<glm::vec4>() == (glt::VAOAttribSize)4);


	using test_tuple = std::tuple<glm::vec2, float, glm::vec4, float, glm::vec3>;
	using seq = std::integer_sequence<size_t, 4, 8, 15, 3, 8, 12, 16, 2>;

	static_assert(glt::get_max<size_t, 4, 8, 15, 3, 8, 12, 16, 2>() == 16);
	static_assert(glt::get_max(seq()) == 16);


	struct Layout
	{
		char c0, c1;
		float f;
		glm::vec2 a;
		char c2,
			c3;
		glm::vec3 b;
	};

	struct Layout1
	{
		char s0;
		glm::vec3 s1;
		char s2;
	};

	using LayoutTuple = glt::compound<char, char, float, glm::vec2, char, char, glm::vec3>;
	using LayoutTuple1 = glt::compound<char, glm::vec3, char>;


	constexpr size_t s0 = offsetof(Layout, c0),
		s1 = offsetof(Layout, c1),
		s2 = offsetof(Layout, f),
		s3 = offsetof(Layout, a),
		s4 = offsetof(Layout, c2),
		s5 = offsetof(Layout, c3),
		s6 = offsetof(Layout, b),
		sLayout = sizeof(Layout);

	static_assert(glt::is_equivalent_v<LayoutTuple, Layout>);
	static_assert(s0 == glt::get_tuple_member_offset_v<0, LayoutTuple>);
	static_assert(s1 == glt::get_tuple_member_offset_v<1, LayoutTuple>);
	static_assert(s2 == glt::get_tuple_member_offset_v<2, LayoutTuple>);
	static_assert(s3 == glt::get_tuple_member_offset_v<3, LayoutTuple>);
	static_assert(s4 == glt::get_tuple_member_offset_v<4, LayoutTuple>);
	static_assert(s5 == glt::get_tuple_member_offset_v<5, LayoutTuple>);
	static_assert(s6 == glt::get_tuple_member_offset_v<6, LayoutTuple>);

	sizeof(LayoutTuple);

	static_assert(glt::class_size_from_tuple_v<LayoutTuple> == sizeof(Layout));
	static_assert(glt::class_size_from_tuple_v<LayoutTuple1> == sizeof(Layout1));

	//////////////////////////////////////////////

	using TestTuple0 = glt::compound<char, char, float, glm::vec2, char, char, glm::vec3>;
	 
	// static_assert(std::is_same_v<char,
	//	glt::nth_attribute_t<0, 0, char, char, float, glm::vec2, char, char, glm::vec3>>);

	return 0;
}