// this unit tests for equivalence
// Run compilation test
#include "helpers.hpp"



int main()
{
	struct abc
	{
		char a;
		long b,
			c;
	};
	// assert layouts' equivalence
	static_assert(sizeof(abc) == sizeof(std::tuple<char, long, long>));
	static_assert(sizeof(vertex) == sizeof(std::tuple<glm::vec3, glm::vec2>));

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

	static_assert(glt::is_equivalent_v<vertex, vertex>);
	static_assert(!glt::is_equivalent_v<abc, vertex>);

	struct Coords
	{
		glm::vec3 xyz;
	};

	static_assert(glt::is_equivalent_v<glm::vec3, Coords>);

	return 0;
}