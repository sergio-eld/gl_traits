/* buffer_test.cpp
This module runs compile-time tests for 
test Template Sequence class "DummySeq".
Get and check offset and stride values for 
received AttribPtr<T>:

Batched Sequence = DummySeq<glm::vec3>

- First attribute for Batched Sequence, buffer offset = 0, 
instances offset = 0;
- First attribute for Batched Sequence, buffer offset = 256, 
instances offset = 0;
- First attribute for Batched Sequence, buffer offset = 256,
instances offset = 10;

Compound Sequence = DummySeq<glm::vec3, glm::vec2, float, glm::vec4> 

- First attribute for Compound Sequence, buffer offset = 0,
instances offset = 0;
- First attribute for Compound Sequence, buffer offset = 256,
instances offset = 0;
- First attribute for Compound Sequence, buffer offset = 256,
instances offset = 10;

- 3th and 4th attributes for Compound Sequence, buffer offset = 0,
instances offset = 0;
- 3th and 4th attributes for Compound Sequence, buffer offset = 256,
instances offset = 0;
- 3th and 4th attributes for Compound Sequence, buffer offset = 256,
instances offset = 10;

This module test buffer objects for:

- Memory allocation: flag 1;
- Buffering Data (SubData): flag 2;
- Mapping Data for Write: flag 4;
- Mapping Data for Read: flag 8;
- Mapping Ranged Data for Read and Write: flag 16;

return code is a bitmask of flags set for each failed case;

// TODO: implement Named AttribPtr:
- user-defined conversion from Sequence class
*/

#include "helpers.hpp"

int test_allocation(int& mask);
int test_SubData_MapRead(int& mask);


template <typename ... Attribs>
class CRTPSeq
{
    const std::ptrdiff_t &bytes_lbound_,
        &bytes_rbound_;

public:

    constexpr static size_t elem_size =
        glt::seq_elem_size<glt::compound<Attribs...>>;

    template <size_t indx>
    constexpr static std::ptrdiff_t AttrOffsetBytes(glt::tag_s<indx>)
    {
        return glt::get_tuple_member_offset_v<indx, std::tuple<Attribs...>>;
    }

    constexpr static GLsizei Stride()
    {
        if constexpr (sizeof...(Attribs) > 1)
            return (GLsizei)elem_size;
        else
            return 0;
    }

    constexpr size_t Allocated() const
    {
        return (bytes_rbound_ - bytes_lbound_)
            / elem_size;
    }

    constexpr std::ptrdiff_t BufferOffsetBytes() const
    {
        return bytes_lbound_;
    }

protected:
    constexpr CRTPSeq(const std::ptrdiff_t &bytes_lbound,
        const std::ptrdiff_t &bytes_rbound)
        : bytes_lbound_(bytes_lbound),
        bytes_rbound_(bytes_rbound)
    {}
};

// Test Sequence class
template <typename ... Attribs>
class DummySeq : public CRTPSeq<Attribs...>,
    glt::get_attrib_ptr<CRTPSeq<Attribs...>, glt::compound<Attribs...>>
{

    size_t allocated;
    std::ptrdiff_t lbound,
        rbound = lbound + elem_size * allocated;

    using get_attrib = glt::get_attrib_ptr<CRTPSeq<Attribs...>, glt::compound<Attribs...>>;

public:
	constexpr DummySeq(size_t allocated = 0, 
		GLsizei offset = 0)
		: CRTPSeq<Attribs...>(lbound, rbound),
        get_attrib(static_cast<const CRTPSeq<Attribs...>&>(*this)),
        allocated(allocated),
        lbound(offset)
	{}

    using get_attrib::AttribPointer;
    using get_attrib::operator();
    using get_attrib::operator glt::AttribPtr<std::tuple_element_t<0, 
        std::tuple<Attribs...>>>;

};


int main()
{
    // compile-time tests
    using Batched = DummySeq<glm::vec3>;
    using Compound = DummySeq<glm::vec3, glm::vec2, float, glm::vec4>;

    constexpr Batched testBatched{ 10, 64 },
        batched{ 10, 0 };

    static_assert(testBatched.Allocated() == 10);
    static_assert(testBatched.AttrOffsetBytes(glt::tag_s<0>()) == 0);
    static_assert(testBatched.BufferOffsetBytes() == 64);
    static_assert(testBatched.Stride() == 0);
    static_assert(testBatched.elem_size == sizeof(glm::vec3));

    constexpr Compound testCompound{ 10, 256 };

    static_assert(testCompound.Allocated() == 10);
    static_assert(testCompound.AttrOffsetBytes(glt::tag_s<0>()) == 0);
    static_assert(testCompound.AttrOffsetBytes(glt::tag_s<1>()) == sizeof(glm::vec3));
    static_assert(testCompound.AttrOffsetBytes(glt::tag_s<2>()) == sizeof(glm::vec3) +
        sizeof(glm::vec2));
    static_assert(testCompound.AttrOffsetBytes(glt::tag_s<3>()) == sizeof(glm::vec3) +
        sizeof(glm::vec2) + sizeof(float));

    static_assert(testCompound.BufferOffsetBytes() == 256);
    static_assert(testCompound.Stride() == glt::class_size_from_tuple_v<std::tuple<glm::vec3, glm::vec2, float, glm::vec4>>);
    static_assert(testCompound.elem_size == glt::class_size_from_tuple_v<std::tuple<glm::vec3, glm::vec2, float, glm::vec4>>);


	glt::sequence_indexed<1, Compound>(Compound()).SeqN(glt::tag_s<1>());

	constexpr glt::sequence_indexed<0, Compound> cmp_indx{ Compound() };
	constexpr const Compound& cmp = cmp_indx;

    // TODO: static check all attributes are the same
    // - First attribute for Batched Sequence, buffer offset = 0, 
    // instances offset = 0;
    
    constexpr glt::AttribPtr<glm::vec3> attr1 = batched,
        attr1a = batched(),
        attr1b = batched(glt::tag_s<0>()),
        attr1c = batched.AttribPointer(glt::tag_s<0>());

    // -First attribute for Batched Sequence, buffer offset = 256,
    // instances offset = 0;
    constexpr glt::AttribPtr<glm::vec3> attr2 = Batched(10, 256),
        attr2a = Batched(10, 256)(),
        attr2b = Batched(10, 256)(glt::tag_s<0>()),
        attr2c = Batched(10, 256).AttribPointer(glt::tag_s<0>());

    // -First attribute for Batched Sequence, buffer offset = 256,
    // instances offset = 10;
    constexpr glt::AttribPtr<glm::vec3> attr3 = Batched(10, 256)(10),
        attr3a = Batched(10, 256)(glt::tag_s<0>(), 10),
        attr3b = Batched(10, 256).AttribPointer(glt::tag_s<0>(), 10);

    // -First attribute for Compound Sequence, buffer offset = 0,
    // instances offset = 0;
    constexpr glt::AttribPtr<glm::vec3> attr4 = Compound(10, 0),
        attr4a = Compound(10, 0)(),
        attr4b = Compound(10, 0)(glt::tag_s<0>()),
        attr4c = Compound(10, 0).AttribPointer(glt::tag_s<0>());

    // -First attribute for Compound Sequence, buffer offset = 256,
    // instances offset = 0;
    constexpr glt::AttribPtr<glm::vec3> attr5 = Compound(10, 256),
        attr5a = Compound(10, 256)(),
        attr5b = Compound(10, 256)(glt::tag_s<0>()),
        attr5c = Compound(10, 256).AttribPointer(glt::tag_s<0>());

    // -First attribute for Compound Sequence, buffer offset = 256,
    // instances offset = 10;
    constexpr glt::AttribPtr<glm::vec3> attr6 = Compound(10, 256)(10),
        attr6a = Compound(10, 256)(glt::tag_s<0>(), 10),
        attr6b = Compound(10, 256).AttribPointer(glt::tag_s<0>(), 10);

    // -3th and 4th attributes for Compound Sequence, buffer offset = 0,
    // instances offset = 0;
    constexpr glt::AttribPtr<float> attr7 = Compound(10, 0)(glt::tag_s<2>()),
        attr7a = Compound(10, 0).AttribPointer(glt::tag_s<2>());

    constexpr glt::AttribPtr<glm::vec4> attr7b = Compound(10, 0)(glt::tag_s<3>()),
        attr7c = Compound(10, 0).AttribPointer(glt::tag_s<3>());

    // -3th and 4th attributes for Compound Sequence, buffer offset = 256,
    // instances offset = 0;
    constexpr glt::AttribPtr<float> attr8 = Compound(10, 256)(glt::tag_s<2>()),
        attr8a = Compound(10, 256).AttribPointer(glt::tag_s<2>());

    constexpr glt::AttribPtr<glm::vec4> attr8b = Compound(10, 256)(glt::tag_s<3>()),
        attr8c = Compound(10, 256).AttribPointer(glt::tag_s<3>());

    // -3th and 4th attributes for Compound Sequence, buffer offset = 256,
    // instances offset = 10;
    constexpr glt::AttribPtr<float> attr9 = Compound(10, 256)(glt::tag_s<2>(), 10),
        attr9a = Compound(10, 256).AttribPointer(glt::tag_s<2>(), 10);

    constexpr glt::AttribPtr<glm::vec4> attr9b = Compound(10, 256)(glt::tag_s<3>(), 10),
        attr9c = Compound(10, 256).AttribPointer(glt::tag_s<3>(), 10);



    // run-time tests
	SmartGLFW glfw{ 3, 3 };
	SmartGLFWwindow window{ SCR_HEIGHT, SCR_WIDTH, "buffer test" };
	glfw.MakeContextCurrent(window);
	glfw.LoadOpenGL();

	int retMask = 0;
	test_SubData_MapRead(retMask);

	return retMask;
}


int test_SubData_MapRead(int & mask)
{
	glt::Buffer<glm::vec3> bVec3;
	glt::Buffer<int> bElements;

	sizeof(bVec3);

	bVec3.Bind(glt::BufferTarget::array);
	bElements.Bind(glt::BufferTarget::element_array);

	std::vector<glm::vec3> positions = glm_cube_positions();

	bVec3.AllocateMemory(positions.size(), glt::BufUsage::static_draw);
	bElements.AllocateMemory(positions.size(), glt::BufUsage::static_draw);

	return 0;
}
