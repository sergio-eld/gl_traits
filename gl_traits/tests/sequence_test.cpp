#include "sequence_layout.hpp"

#include <glm/glm.hpp>

template <typename ... Attribs>
class DummySeq : public glt::aggregate_attribs<glt::compound<Attribs...>>
{
	using aggr_attribs = glt::aggregate_attribs<glt::compound<Attribs...>>;

	glt::seq_layout_info<Attribs...> layout_;


	size_t allocated_;
	std::ptrdiff_t lbound_,
		rbound_ = lbound_ + glt::seq_layout_info<Attribs...>::elem_size * allocated_;

public:

	constexpr DummySeq(size_t alloc, std::ptrdiff_t lbound = 0)
		: aggr_attribs(layout_),
		layout_(lbound_, rbound_),
		allocated_(alloc),
		lbound_(lbound)
	{}

	constexpr size_t Stride() const
	{
		return layout_.Stride();
	}

	constexpr size_t Allocated() const
	{
		return layout_.Allocated();
	}

	constexpr std::ptrdiff_t BufferOffset() const
	{
		return layout_.BufferOffset();
	}

	using aggr_attribs::AttribPointer;

};

int main()
{
	constexpr DummySeq<glm::vec3> dVec3{ 10, 16 };
	
	// testing Batched sequence

	static_assert(DummySeq<glm::vec3>(10).Allocated() == 10);
	static_assert(DummySeq<glm::vec3>(10).Stride() == 0);
	static_assert(DummySeq<glm::vec3>(10, 16).BufferOffset() == 16);

	static_assert(DummySeq<glm::vec3>(10).AttribPointer(glt::tag_s<0>()).stride == 0);
	static_assert(DummySeq<glm::vec3>(10, 16).AttribPointer().offset == 16);
	static_assert(DummySeq<glm::vec3>(10, 16)(glt::tag_s<0>()).offset == 16);
	static_assert(DummySeq<glm::vec3>(10, 16)().offset == 16);

	// testing Compound sequence

	static_assert(DummySeq<glm::vec3, glm::vec2>(10).Allocated() == 10);
	static_assert(DummySeq<glm::vec3, glm::vec2>(10).Stride() == 
		glt::get_class_size_v<glm::vec3, glm::vec2>);
	static_assert(DummySeq<glm::vec3, glm::vec2>(10, 128).BufferOffset() == 128);

	static_assert(DummySeq<glm::vec3, glm::vec2>(10).AttribPointer(glt::tag_s<0>()).stride ==
		glt::get_class_size_v<glm::vec3, glm::vec2>);
	static_assert(DummySeq<glm::vec3, glm::vec2>(10, 16).AttribPointer().offset == 16);
	static_assert(DummySeq<glm::vec3, glm::vec2>(10, 16)(glt::tag_s<0>()).offset == 16);
	static_assert(DummySeq<glm::vec3, glm::vec2>(10, 16)().offset == 16);

	static_assert(DummySeq<glm::vec3, glm::vec2>(10, 16)(glt::tag_s<1>()).offset == 
		16 + glt::get_member_offset_v<1, glm::vec3, glm::vec2>);



	return 0;
}