
#include "basic_types.hpp"

constexpr const char xyz[] = "xyz";

using NamedSimple = glt::glslt<glm::vec3, xyz>;

int main()
{

    static_assert(glt::has_name_v<NamedSimple>);
    static_assert(!glt::has_name_v<glm::vec3>);

    static_assert(glt::is_compound_attr_v<glt::compound<glm::vec3, glm::vec2>>);
    static_assert(!glt::is_compound_attr_v<glt::compound<glm::vec3>>);
    static_assert(!glt::is_compound_attr_v<glt::compound<>>);
    static_assert(!glt::is_compound_attr_v<glm::vec3>);

    // named attributes
    static_assert(glt::is_compound_attr_v<glt::compound<glt::glslt<glm::vec3, xyz>, glt::glslt<glm::vec2, xyz>>>);
    static_assert(!glt::is_compound_attr_v<glt::compound<glt::glslt<glm::vec3, xyz>>>);
    static_assert(!glt::is_compound_attr_v<glt::compound<>>);
    static_assert(!glt::is_compound_attr_v<glt::glslt<glm::vec3, xyz>>);
    
    using Compound4 = glt::compound<float, char, glt::glslt<glm::vec3, xyz>, glt::glslt<glm::vec2, xyz>>;
    static_assert(glt::compound_attr_count_v<Compound4> == 4);
    static_assert(glt::compound_attr_count_v<Compound4> != 3);
    static_assert(glt::compound_attr_count_v<glt::compound<float>> == 1);
    static_assert(glt::compound_attr_count_v<glt::compound<>> == 0);


    return 0;
}