
#include "traits.hpp"

struct Vertex
{
    float p0;
    glm::vec4 p1;
    glm::vec2 p2;
};

using attribs = glt::AttrList<float, float, float>;

constexpr inline const char uname_model[] = "model",
uname_view[] = "view",
uname_projection[] = "projection",
uname_texture1[] = "texture1",
uname_texture2[] = "texture2";

using utype_model = glt::glslt<glm::mat4, uname_model>;
using utype_view = glt::glslt<glm::mat4, uname_view>;
using utype_projection = glt::glslt<glm::mat4, uname_projection>;
using utype_texture1 = glt::glslt<int, uname_texture1>;
using utype_texture2 = glt::glslt<int, uname_texture2>;

int main()
{
    static_assert(glt::templ_params_count_v<attribs> == 3);

    static_assert(glt::is_tuple_v<std::tuple<>>);
    static_assert(glt::is_tuple_v<glt::compound<>>);
    static_assert(glt::is_tuple_v<glt::compound<>>);
    static_assert(!glt::is_tuple_v<Vertex>);

    static_assert(glt::is_initializable_from_v<Vertex, float>);
    static_assert(glt::is_initializable_from_v<Vertex, float, glm::vec4>);
    static_assert(glt::is_initializable_from_v<Vertex, float, glm::vec4, glm::vec2>);
    static_assert(!glt::is_initializable_from_v<Vertex, glm::vec4, glm::vec2>);

    static_assert(std::is_same_v<float, glt::nth_element_t<0, float, glm::vec3, glm::vec2, Vertex>>);
    static_assert(std::is_same_v<glm::vec3, glt::nth_element_t<1, float, glm::vec3, glm::vec2, Vertex>>);
    static_assert(std::is_same_v<glm::vec2, glt::nth_element_t<2, float, glm::vec3, glm::vec2, Vertex>>);
    static_assert(std::is_same_v<Vertex, glt::nth_element_t<3, float, glm::vec3, glm::vec2, Vertex>>);

	static_assert(std::is_same_v<int, glt::unwrap_glslt_t<int>>);
	static_assert(std::is_same_v<int, glt::unwrap_glslt_t<glt::glslt<int, 0>>>);

	static_assert(std::is_same_v<glm::vec4, glt::unwrap_glslt_t<glm::vec4>>);
	static_assert(std::is_same_v<glm::vec4, 
		glt::unwrap_glslt_t<glt::glslt<glm::vec4, 0>>>);

	static_assert(std::is_same_v<glm::mat4, glt::unwrap_glslt_t<glm::mat4>>);
	static_assert(std::is_same_v<glm::mat4,
		glt::unwrap_glslt_t<glt::glslt<glm::mat4, 0>>>);


	static_assert(uname_model == glt::get_glslt_name_v<utype_model>);

	return 0;
}
