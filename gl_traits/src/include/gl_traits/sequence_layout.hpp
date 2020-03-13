#pragma once

#include "equivalence.hpp"
#include <cassert>

namespace glt
{
	// seq layout info - rename?
	template <typename ... Attribs>
	class seq_layout_info
	{
		// sequence boundaries
		const std::ptrdiff_t &bytes_lbound_,
			&bytes_rbound_;

	public:

		constexpr static size_t elem_size =
			get_class_size_v<Attribs...>;

		template <size_t indx>
		constexpr static std::ptrdiff_t AttrOffset(tag_s<indx>)
		{
			static_assert(indx < sizeof...(Attribs), "Attribute of index is out of range!");
			return get_member_offset_v<indx, Attribs...>;
		}

		constexpr static size_t Stride()
		{
			if constexpr (sizeof...(Attribs) > 1)
				return elem_size;
			else
				return 0;
		}

		constexpr size_t Allocated() const
		{
			return (bytes_rbound_ - bytes_lbound_)
				/ elem_size;
		}

		constexpr std::ptrdiff_t BufferOffset() const
		{
			return bytes_lbound_;
		}

		// use ambassador if want protected constructor
	//protected:

		constexpr seq_layout_info(const std::ptrdiff_t &bytes_lbound,
			const std::ptrdiff_t &bytes_rbound)
			: bytes_lbound_(bytes_lbound),
			bytes_rbound_(bytes_rbound)
		{}
	};

	// A - is a raw glsl type or named glsl type (wrapped)
	template <typename A>
	struct AttribPtr
	{
		// static_assert(!is_compound_seq_v<A>, "Compound attributes are not allowed!");

		// these parameters may be deduced inside vao class methods,
		// though storing all the attribute's info in one class is more intuitive
		// constexpr static GLint size = sizeof(variable_traits_type<A>);
		// constexpr static GLenum glType = (GLenum)c_to_gl_v<variable_traits_type<A>>;

		// stride only depends on neighbor attributes with compound allignment
		std::ptrdiff_t offset;
		size_t stride;

		constexpr AttribPtr(std::ptrdiff_t offset = 0, size_t stride = 0)
			: stride(stride),
			offset(offset)
		{}

	};

	/* Private implementation class
Responsibility:
- generate "AttribPtr<Attrib> operator()(size_t instOffset = 0) const"
for first/single Attribute in a Buffer Sequence;
- generate "constexpr operator AttribPtr<Attrib>() const"
for first/single Attribute in a Buffer Sequence;
- generate "AttribPtr<Attrib> operator()(tag_s<N>, size_t instOffset = 0) const"
for Nth Attribute in a Buffer Sequence (if compound)

(?)- generate "AttribPtr<AttrNamed> operator()(tag_t<AttrNamed>,
size_t instOffset = 0) const"
for Named Attribute in a Buffer Sequence (if compound)

Depends on:
- the offset within the containing buffer (provided via CRTP::BufferOffsetBytes());
- the attribute's offset within the containing sequence (provided via CRTPSeq::AttrOffsetBytes(tag_s<indx>)); // PROBLEM HERE
- the stride (for Batched = 0) of the given sequence's element (provided via CRTPSeq::Stride());
- the size of the sequence element (provided via CRTPSeq::elem_size);
- amount of allocated instances/elements (provided via CRTP::Allocated());
- the index within the sequence (provided as non-template parameter);

Template arguments:
Attr - is a Type of the Nth attribute within the sequence.
indx - is an attribute position within the compound sequence
first - true if indx = 0.
*/
	template <class seq_layout, class Attr, size_t indx, bool first = !indx> // not first case
	class get_attrib_ptr_i
	{
		const seq_layout& layout_;

	public:

		constexpr get_attrib_ptr_i(const seq_layout& seq)
			: layout_(seq)
		{}

		constexpr AttribPtr<Attr> AttribPointer(tag_s<indx>, size_t instOffset = 0) const
		{
			assert(layout_.Allocated() >= instOffset && "Pointer exceeds sequence's bounds!");
			return AttribPtr<Attr>(seq_layout::AttrOffset(tag_s<indx>()) + layout_.BufferOffset() +
				seq_layout::elem_size * instOffset, seq_layout::Stride());
		}

		constexpr AttribPtr<Attr> operator()(tag_s<indx>, size_t instOffset = 0) const
		{
			return AttribPointer(tag_s<indx>(), instOffset);
		}

	};

	template <class seq_layout, class Attr>
	class get_attrib_ptr_i<seq_layout, Attr, 0, true> :
		public get_attrib_ptr_i<seq_layout, Attr, 0, false>
	{
		using base = get_attrib_ptr_i<seq_layout, Attr, 0, false>;

	public:

		constexpr get_attrib_ptr_i(const seq_layout& seq)
			: base(seq)
		{}

		constexpr AttribPtr<Attr> AttribPointer() const
		{
			return AttribPointer(tag_s<0>());
		}

		constexpr AttribPtr<Attr> operator()(size_t instOffset = 0) const
		{
			return (*this)(tag_s<0>(), instOffset);
		}

		// need this?
		constexpr operator AttribPtr<Attr>() const
		{
			return (*this)(tag_s<0>());
		}

		using base::operator();
		using base::AttribPointer;

	};

	// TODO: independent from seq_layout_info
	template <class AttrCompound, 
		class = decltype(std::make_index_sequence<std::tuple_size_v<AttrCompound>>())>
		class aggregate_attribs;

	template <class ... Attr, size_t ... indx>
	struct aggregate_attribs<compound<Attr...>, std::index_sequence<indx...>> :
		public get_attrib_ptr_i<seq_layout_info<Attr...>, Attr, indx>...
	{
		template <size_t i>
		using get_attrib_i = get_attrib_ptr_i<seq_layout_info<Attr...>,
			std::tuple_element_t<i, std::tuple<Attr...>>, i>;

		using get_attrib_i<indx>::AttribPointer ...;
		using get_attrib_i<indx>::operator() ...;

		constexpr aggregate_attribs(const seq_layout_info<Attr...>& seq)
			: get_attrib_i<indx>(seq) ...
		{}
	};



	
	// data_impl provides implementation for SubData and MapRange, etc.
	template <class AttrCompound, class data_impl,
		class = decltype(std::make_index_sequence<std::tuple_size_v<AttrCompound>>())>
		class sequence_aggregate;

	template <class data_impl, class ... Attr, size_t ... indx>
	class sequence_aggregate<compound<Attr...>, data_impl, std::index_sequence<indx...>> :
		public get_attrib_ptr_i<seq_layout_info<Attr...>, Attr, indx>...
	{
		seq_layout_info<Attr...> seq_;

		template <size_t i>
		using get_attrib_i = get_attrib_ptr_i<seq_layout_info<Attr...>,
			std::tuple_element_t<i, std::tuple<Attr...>>, i>;

	protected:
		constexpr sequence_aggregate(const std::ptrdiff_t &bytes_lbound,
			const std::ptrdiff_t &bytes_rbound)
			: get_attrib_i<indx>(seq_) ...,
			seq_(bytes_lbound, bytes_rbound)
		{}

	public:

		using get_attrib_i<indx>::AttribPointer ...;
		using get_attrib_i<indx>::operator() ...;


		constexpr static size_t elem_size = seq_layout_info<Attr...>::elem_size;

		constexpr static size_t Stride()
		{
			return seq_layout_info<Attr...>::Stride();
		}

		constexpr size_t Allocated() const
		{
			return seq_.Allocated();
		}

		constexpr std::ptrdiff_t BufferOffsetBytes() const
		{
			return seq_.BufferOffsetBytes();
		}


	};

}