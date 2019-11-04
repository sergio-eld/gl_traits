#include "gl_traits.hpp"

#include "gltEnums.hpp"

struct vertex
{
	glm::vec3 posCoords;
	glm::vec2 textureCoords;

};

constexpr const char aPos[] = "aPos";
constexpr const char aTex[] = "aTex";

using attr_aPos = typename glslt<glm::vec3, aPos>;
using attr_aTex = typename glslt<glm::vec2, aTex>;

void assert_all()
{
    // asserting c to gl types convertions
    static_assert(c_to_gl_v<GLbyte> == (glType)GL_BYTE);
    static_assert(c_to_gl_v<GLubyte> == (glType)GL_UNSIGNED_BYTE);
    static_assert(c_to_gl_v<GLshort> == (glType)GL_SHORT);
    static_assert(c_to_gl_v<GLushort> == (glType)GL_UNSIGNED_SHORT);
    static_assert(c_to_gl_v<GLint> == (glType)GL_INT);
    static_assert(c_to_gl_v<GLuint> == (glType)GL_UNSIGNED_INT);
    //static_assert(c_to_gl_v<GLfixed> == GL_FIXED);
    //static_assert(c_to_gl_v<GLhalf> == GL_HALF_FLOAT);
    static_assert(c_to_gl_v<GLfloat> == (glType)GL_FLOAT);
    static_assert(c_to_gl_v<GLdouble> == (glType)GL_DOUBLE);

    std::is_same_v<GLint, GLfixed>;
    std::is_same_v<GLushort, GLhalf>;

    gl_deleter2<&glDeleteBuffers>();
    gl_deleter2<&glDeleteTextures> {};
    gl_deleter1<&glDeleteShader> {};
    gl_deleter1<&glDeleteProgram> {};
    gl_deleter2<&glDeleteVertexArrays> {};

	valid_user_attribclass<vertex>();
	valid_user_attribclass<glm::vec2>();

	using vertex_types = refl_traits<vertex>::field_types;

	static_assert(std::is_same_v<vertex_types, std::tuple<glm::vec3, glm::vec2>>);
	//static_assert(all_glm_v<glm::vec3, glm::vec2>);
	static_assert(all_glm_v<std::tuple<glm::vec3, glm::vec2>>);
	static_assert(!all_glm_v<std::tuple<glm::vec3, float>>);
	static_assert(all_glm_v<std::tuple<glm::vec3, glm::mat3>>);

    /*
	glenum_from_type<float>::value;
	glenum_from_type<glm::vec4>::value;
	glenum_from_type<glm::vec4>::nComponents;
	glenum_from_type<glm::mat4>::value;
	glenum_from_type<glm::mat4>::nComponents;

	glenum_from_type<glm::uvec4>::value;
	glenum_from_type<glm::uvec4>::nComponents;
	glenum_from_type<glm::fmat3>::value;
	glenum_from_type<glm::fmat4>::nComponents;
    */

	sizeof(glm::vec4);

	is_compound_attr_v<glm::vec4>;
	//attrib_traits<glm::vec4>::stride;

	is_compound_attr_v<comp_attr<glm::vec3, glm::vec2>>;
	//attrib_traits<comp_attr<glm::vec3, glm::vec2>>::stride;

	refl_traits<glm::vec4>::fields_count();
	refl_traits<glm::mat4>::fields_count();
	refl_traits<glm::vec4>::field_types;

	static_assert(!all_equivalent_v<glm::vec4, float>);
	static_assert(all_equivalent_v<comp_attr<glm::vec3, glm::vec2>,
		refl_traits<vertex>::field_types>);

	static_assert(is_equivalent_v<glm::vec4, glm::vec4>);
	static_assert(!is_equivalent_v<glm::vec4, glm::vec3>);

	// check compound class against user-defined compount class
	static_assert(is_equivalent_v<comp_attr<glm::vec3, glm::vec2>, vertex>);
	static_assert(!is_equivalent_v<comp_attr<glm::vec3, glm::vec2>,
		comp_attr<glm::vec3>>);

	static_assert(is_named_attr_v<attr_aPos>);
	static_assert(!is_named_attr_v<glm::vec3>);
	constexpr const char* name = attr_aPos::name;

	using compound_attr = comp_attr<glm::vec3, 
		glm::vec2, glm::vec2>;

	using tuple2 = std::tuple<bool, size_t, char, char, glm::vec3>;

	static_assert(tuple_size_v<compound_attr> == sizeof(compound_attr));
	static_assert(tuple_size_v<tuple2> != sizeof(tuple2));

	static_assert(get_offset<0, compound_attr>::value == 0);
	static_assert(get_offset<1, compound_attr>::value == sizeof(glm::vec3));
	static_assert(get_offset<2, compound_attr>::value == sizeof(glm::vec3) + sizeof(glm::vec2));

//	valid_attr_collection<compound_attr, compound_attr>::has_compounds;
	/*
	static_assert(dh_conjunction<true, true, true>);
	static_assert(!dh_conjunction<true, false, true>);
	static_assert(dh_disjunction<false, false, true>);
	static_assert(!dh_disjunction<false, false, false>);

	static_assert(!valid_attr_collection_v<>);
	static_assert(valid_attr_collection_v<compound_attr>);
	static_assert(!valid_attr_collection_v<compound_attr, compound_attr>);
	static_assert(!valid_attr_collection_v<glm::vec3, compound_attr>);
	static_assert(valid_attr_collection_v<glm::vec3>);
	static_assert(valid_attr_collection_v<compound_attr, glm::vec3>);*/

//	static_assert(!str_equal(aPos, aTex));

}


