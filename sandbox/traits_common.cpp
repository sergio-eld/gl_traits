
#include "traits.hpp"

struct Vertex
{
    float p0;
    glm::vec4 p1;
    glm::vec2 p2;
};

using attribs = glt::AttrList<float, float, float>;

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


	return 0;
}
