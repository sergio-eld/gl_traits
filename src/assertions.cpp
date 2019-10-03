#include "gl_traits.hpp"

struct vertex
{
	glm::vec3 posCoords;
	glm::vec2 textureCoords;

};

constexpr const char aPos[] = "aPos";

using attr_aPos = typename glslt<glm::vec3, aPos>;


void assert_all()
{
	valid_user_attribclass<vertex>();
	valid_user_attribclass<glm::vec2>();

	using vertex_types = refl_traits<vertex>::field_types;

	static_assert(std::is_same_v<vertex_types, std::tuple<glm::vec3, glm::vec2>>);
	//static_assert(all_glm_v<glm::vec3, glm::vec2>);
	static_assert(all_glm_v<std::tuple<glm::vec3, glm::vec2>>);
	static_assert(!all_glm_v<std::tuple<glm::vec3, float>>);
	static_assert(all_glm_v<std::tuple<glm::vec3, glm::mat3>>);

	glenum_from_type<float>::value;
	glenum_from_type<glm::vec4>::value;
	glenum_from_type<glm::vec4>::nComponents;
	glenum_from_type<glm::mat4>::value;
	glenum_from_type<glm::mat4>::nComponents;

	glenum_from_type<glm::uvec4>::value;
	glenum_from_type<glm::uvec4>::nComponents;
	glenum_from_type<glm::fmat3>::value;
	glenum_from_type<glm::fmat4>::nComponents;

	sizeof(glm::vec4);

	attrib_traits<glm::vec4>::is_compound;
	attrib_traits<glm::vec4>::stride;

	attrib_traits<attr_compound<glm::vec3, glm::vec2>>::is_compound;
	attrib_traits<attr_compound<glm::vec3, glm::vec2>>::stride;

	refl_traits<glm::vec4>::fields_count();
	refl_traits<glm::mat4>::fields_count();
	refl_traits<glm::vec4>::field_types;

	static_assert(!all_equivalent_v<glm::vec4, float>);
	static_assert(all_equivalent_v<attr_compound<glm::vec3, glm::vec2>,
		refl_traits<vertex>::field_types>);

	static_assert(is_equivalent_v<glm::vec4, glm::vec4>);
	static_assert(!is_equivalent_v<glm::vec4, glm::vec3>);

	// check compound class against user-defined compount class
	static_assert(is_equivalent_v<attr_compound<glm::vec3, glm::vec2>, vertex>);
	static_assert(!is_equivalent_v<attr_compound<glm::vec3, glm::vec2>,
		attr_compound<glm::vec3>>);

	static_assert(is_named_attr_v<attr_aPos>);
	static_assert(!is_named_attr_v<glm::vec3>);
	constexpr const char* name = attr_aPos::name;

	using compound_attr = attr_compound<glm::vec3, 
		glm::vec2, glm::vec2>;

	get_offset<0, compound_attr>::sub_tuple;

}


