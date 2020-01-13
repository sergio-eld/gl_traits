
#include "basic_types.hpp"

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

struct Layout2
{
    char s0;
    glm::vec3 s1;
    char s2;
};



int main()
{
    
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

    // static_assert(!glt::is_tuple_equivalent_v<LayoutTuple, LayoutTuple1>); // must fail assertion
    static_assert(!glt::is_tuple_equivalent_v<Layout1, LayoutTuple>);
    static_assert(glt::is_tuple_equivalent_v<Layout, LayoutTuple>);
    static_assert(!glt::is_tuple_equivalent_v<Layout, glt::compound<char, char>>);

    static_assert(!glt::recover_tuple<Layout>());
    static_assert(!glt::recover_tuple<Layout, std::tuple<char>>());
    static_assert(glt::recover_tuple<Layout,
        std::tuple<char, char, float, glm::vec2, char, char, glm::vec3>>());

    static_assert(std::is_same_v<std::tuple<Layout>, glt::recover_tuple_t<Layout>>); // failed to recover
    static_assert(std::is_same_v<std::tuple<char, char, float, glm::vec2, char, char, glm::vec3>,
        glt::recover_tuple_t<std::tuple<char, char, float, glm::vec2, char, char, glm::vec3>>>); 

    static_assert(std::is_same_v<std::tuple<char, char, float, glm::vec2, char, char, glm::vec3>,
        glt::recover_tuple_t<Layout, std::tuple<char, char, float, glm::vec2, char, char, glm::vec3>>>);

    static_assert(glt::is_equivalent_v<Layout, Layout>);
    static_assert(!glt::is_equivalent_v<Layout, Layout1>);
    static_assert(glt::is_equivalent_v<LayoutTuple, Layout>);
    static_assert(glt::is_equivalent_v<LayoutTuple, LayoutTuple>);  
    static_assert(glt::is_equivalent_v<Layout1, Layout2, LayoutTuple1>);
    static_assert(!glt::is_equivalent_v<Layout, Layout2, LayoutTuple1>);


	static_assert(glt::FetchedAttrib<glm::vec3>::size == sizeof(glm::vec3));

	glt::FetchedAttrib<glm::vec3> a{ 2, 13 };

    return 0;
}