
#include "basic_types.hpp"

template <class ... >
struct BufferTest {};

using Compound = glt::compound<float, glm::vec3, glm::vec2, char, char>;
using Buffer = BufferTest<glm::vec3, glm::vec2, glt::compound<glm::vec4, glm::vec2>, glt::compound<glt::glslt<glm::vec2, nullptr>>>;

int main()
{
    static_assert(std::is_same_v<float, glt::nth_parameter_t<0, Compound>>);
    static_assert(std::is_same_v<glm::vec3, glt::nth_parameter_t<1, Compound>>);
    static_assert(std::is_same_v<glm::vec2, glt::nth_parameter_t<2, Compound>>);
    static_assert(std::is_same_v<char, glt::nth_parameter_t<3, Compound>>);
    static_assert(std::is_same_v<char, glt::nth_parameter_t<4, Compound>>);

    static_assert(std::is_same_v<glm::vec3, glt::nth_parameter_t<0, Buffer>>);
    static_assert(std::is_same_v<glm::vec2, glt::nth_parameter_t<1, Buffer>>);
    static_assert(std::is_same_v<glt::compound<glm::vec4, glm::vec2>, glt::nth_parameter_t<2, Buffer>>);
    static_assert(std::is_same_v<glt::compound<glt::glslt<glm::vec2, nullptr>>, glt::nth_parameter_t<3, Buffer>>);
    // static_assert(!std::is_same_v<char, glt::nth_parameter_t<4, Buffer>>); // index out of bounds

	return 0;
}
