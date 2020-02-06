
#include <vector>

#include "equivalence.hpp" //"basic_types.hpp"
#include "glm/glm.hpp"

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

    static_assert(glt::is_aggregate_initializable_v<glt::compound<int>, int>);
    static_assert(glt::is_aggregate_initializable_v<glt::compound<glm::vec3>, glm::vec3>);
    static_assert(glt::is_equivalent_v<int, glt::compound<int>>);
    static_assert(glt::is_equivalent_v<glm::vec3, glt::compound<glm::vec3>>);
    static_assert(glt::is_equivalent_v<glt::compound<int>, int>);
    static_assert(glt::is_equivalent_v<glt::compound<glm::vec3>, glm::vec3>);
    static_assert(glt::is_equivalent_v<glt::compound<int>, glt::compound<int>>);

   
    using LayoutCompound = glt::compound<char, char, float, glm::vec2, char, char, glm::vec3>;
    using LayoutCompound1 = glt::compound<char, glm::vec3, char>;

    constexpr size_t s0 = offsetof(Layout, c0),
        s1 = offsetof(Layout, c1),
        s2 = offsetof(Layout, f),
        s3 = offsetof(Layout, a),
        s4 = offsetof(Layout, c2),
        s5 = offsetof(Layout, c3),
        s6 = offsetof(Layout, b),
        sLayout = sizeof(Layout);

    static_assert(s0 == glt::get_compound_member_offset_v<0, LayoutCompound>);
    static_assert(s1 == glt::get_compound_member_offset_v<1, LayoutCompound>);
    static_assert(s2 == glt::get_compound_member_offset_v<2, LayoutCompound>);
    static_assert(s3 == glt::get_compound_member_offset_v<3, LayoutCompound>);
    static_assert(s4 == glt::get_compound_member_offset_v<4, LayoutCompound>);
    static_assert(s5 == glt::get_compound_member_offset_v<5, LayoutCompound>);
    static_assert(s6 == glt::get_compound_member_offset_v<6, LayoutCompound>);   
    
    
    static_assert(!glt::is_equivalent_v<LayoutCompound, LayoutCompound1>);
    static_assert(!glt::is_equivalent_v<Layout1, LayoutCompound>);
    static_assert(glt::is_equivalent_v<Layout, LayoutCompound>);
    static_assert(!glt::is_equivalent_v<Layout, glt::compound<char, char>>);

    
    static_assert(glt::is_equivalent_v<Layout, Layout>);
    static_assert(!glt::is_equivalent_v<Layout, Layout1>);
    static_assert(glt::is_equivalent_v<Layout, LayoutCompound>);
    static_assert(glt::is_equivalent_v<LayoutCompound, Layout>);
    static_assert(glt::is_equivalent_v<LayoutCompound, LayoutCompound>);  
    static_assert(glt::is_equivalent_v<Layout1, Layout2, char, glm::vec3, char>);
    static_assert(!glt::is_equivalent_v<Layout, Layout2, char, glm::vec3, char>);
    

	std::vector<Layout> testLayout{ 42, 
		Layout{'a', 'b',
		4.0f,
		glm::vec2(8, 15), 
		'c', 'd', 
		glm::vec3(16, 23, 42)} };

	size_t errors = 0;

	for (const Layout& l : testLayout)
	{
		const LayoutCompound& lcomp = reinterpret_cast<const LayoutCompound&>(l);

		const char& a = lcomp.Get(glt::tag_s<0>()),
			b = lcomp.Get(glt::tag_s<1>());

		const float& f = lcomp.Get(glt::tag_s<2>());
		const glm::vec2& v2 = lcomp.Get(glt::tag_s<3>());

		const char& c = lcomp.Get(glt::tag_s<4>()),
			d = lcomp.Get(glt::tag_s<5>());

		const glm::vec3& v3 = lcomp.Get(glt::tag_s<6>());

		errors += a != 'a' ||
			b != 'b' ||
			c != 'c' ||
			d != 'd' ||
			f != 4.0f ||
			v2 != glm::vec2(8, 15) ||
			v3 != glm::vec3(16, 23, 42);

	}

    return errors;
}