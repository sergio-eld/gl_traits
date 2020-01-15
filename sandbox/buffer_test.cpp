/* buffer_test.cpp
This module test buffer objects for:
- Memory allocation: flag 1;
- Buffering Data (SubData): flag 2;
- Mapping Data for Write: flag 4;
- Mapping Data for Read: flag 8;
- Mapping Ranged Data for Read and Write: flag 16;

return code is a bitmask of flags set for each failed case;
*/

#include "helpers.hpp"

int test_allocation(int& mask);
int test_SubData_MapRead(int& mask);

template <typename ... Attribs>
struct DummySeq
{
	constexpr static size_t elem_size = 
		glt::seq_elem_size<glt::compound<Attribs...>>;

	size_t allocated;
	GLsizei offset;

	constexpr DummySeq(size_t allocated = 0, 
		GLsizei offset = 0)
		: allocated(allocated),
		offset(offset)
	{}

	constexpr size_t Allocated() const
	{
		return 100;
	}

	constexpr GLsizei OffsetBytes() const
	{
		return offset;
	}
};



int main()
{
	SmartGLFW glfw{ 3, 3 };
	SmartGLFWwindow window{ SCR_HEIGHT, SCR_WIDTH, "buffer test" };
	glfw.MakeContextCurrent(window);
	glfw.LoadOpenGL();

	int retMask = 0;
	test_allocation(retMask);

	glt::is_compound_seq_v<glt::compound<glm::vec3>>;

	static_assert(!glt::is_compound_seq_v<glt::compound<glm::vec3>>);

	DummySeq<glm::vec3, glm::vec2>::elem_size;

	static_assert(glt::seq_elem_count<glm::vec3> == 1);

	using SeqBatched = DummySeq<glm::vec3>;
	using SeqCompound = DummySeq<glm::vec3, glm::vec2, float>;
	using Compound = glt::compound<glm::vec3, glm::vec2, float>;

	constexpr glt::get_attrib_ptr_i<SeqBatched,
		glm::vec3> base_vec3{ SeqBatched() }; // intellisence wtf???

	constexpr glt::get_attrib_ptr<SeqCompound, Compound> 
		inh_base{ SeqCompound() }; // intellisence wtf???

	inh_base.AttribPointer(glt::tag_s<2>());

	//inh_base(glt::tag_s<0>());

	// batched sequence
	constexpr glt::AttribPtr<glm::vec3> attr1 = glt::get_attrib_ptr_i<SeqBatched,
		glm::vec3>(SeqBatched(100))(glt::tag_s<0>()),

		attr1a = glt::get_attrib_ptr_i<SeqBatched,
		glm::vec3>(SeqBatched(100, 64))(),

		atr1b = glt::get_attrib_ptr_i<SeqBatched,
		glm::vec3>(SeqBatched(100, 64));

		// compound sequence

	/*
	// batched sequence
	constexpr glt::AttribPtr<glm::vec3> 
		attr = glt::get_attrib_ptr_i<DummySeq<>, glm::vec3>(DummySeq<>())(),
		attr1 = glt::get_attrib_ptr_i<DummySeq<64>, glt::compound<glm::vec3>>(DummySeq<64>()),

		// compound sequence first attribute pointer
		attrComp = glt::get_attrib_ptr_i<DummySeq<64>,
		glt::compound<glm::vec3, glm::vec2>>(DummySeq<64>())(),

		attrComp2 = glt::get_attrib_ptr_i<DummySeq<64>, 
		glt::compound<glm::vec3, glm::vec2>>(DummySeq<64>());
		*/



	return retMask;
}

int test_allocation(int & mask)
{
	glt::BufferSingle<glm::vec3> bVec3;
	glt::BufferSingle<glt::compound<glm::vec3, glm::vec2, float>> bVecCmp;

	return mask;
}

int test_SubData_MapRead(int & mask)
{
	glt::BufferSingle<glm::vec3> bVec3;

	std::vector<glm::vec3> positions = glm_cube_positions();
	bVec3.AllocateMemory(positions.size(), glt::BufUsage::static_draw);


	return 0;
}
