#pragma once

#include "Sequence.hpp"

namespace glt
{

    template <class attr, bool = is_compound_seq_v<attr>>
    struct wrap_attr
    {
        using type = compound<attr>;
    };

    template <class ... attr>
    struct wrap_attr<compound<attr...>, true>
    {
        using type = compound<attr...>;
    };

    template <typename T>
    using wrap_attr_t = typename wrap_attr<T>::sequence;

	template <size_t indx, class AttrCompound, bool first = !indx>
	struct sequence_indexed;

	template <size_t indx, class ... Attr>
	struct sequence_indexed<indx, compound<Attr...>, false>
	{
		Sequence<Attr...> seq;

		constexpr sequence_indexed(Buffer_base& buf,
			const std::ptrdiff_t &bytes_lbound,
			const std::ptrdiff_t &bytes_rbound)
			: seq(buf, bytes_lbound, bytes_rbound)
		{}

		constexpr const Sequence<Attr...>& SeqN(tag_s<indx>) const
		{
			return seq;
		}

		Sequence<Attr...>& SeqN(tag_s<indx>)
		{
			return seq;
		}

        constexpr const Sequence<Attr...>& operator()(tag_s<indx>) const
        {
            return SeqN(tag_s<indx>());
        }

		Sequence<Attr...>& operator()(tag_s<indx>)
        {
            return SeqN(tag_s<indx>());
        }
	};

	template <class ... Attr>
	struct sequence_indexed<0, compound<Attr...>, true> :
		public sequence_indexed<0, compound<Attr...>, false>
	{
		using seq_base = sequence_indexed<0, Seq, false>;

		constexpr sequence_indexed(Buffer_base& buf,
			const std::ptrdiff_t &bytes_lbound,
			const std::ptrdiff_t &bytes_rbound)
			: seq_base(buf, bytes_lbound, bytes_rbound)
		{}

		using seq_base::SeqN;

		constexpr const Seq& SeqN() const
		{
			return seq_base::SeqN(tag_s<0>());
		}

		Seq& SeqN()
		{
			return seq_base::SeqN(tag_s<0>());
		}

		constexpr const Seq& operator()() const
		{
			return seq_base::SeqN(tag_s<0>());
		}

		Seq& operator()()
		{
			return seq_base::SeqN(tag_s<0>());
		}

		constexpr operator const Seq&() const
		{
            return SeqN();
		}

		operator Seq&()
		{
            return SeqN();
        }


	};

	template <class TupleSeqAttr,
		class = decltype(std::make_index_sequence<std::tuple_size_v<TupleSeqAttr>>())>
		struct aggregated_sequences;

	template <class ... seq_attribs, size_t ... indx>
	struct aggregated_sequences<compound<seq_attribs...>, std::index_sequence<indx...>> :
		public sequence_indexed<indx, wrap_attr_t<seq_attribs>> ...
	{
		template <size_t i>
		using sequence_i =
			sequence_indexed<i, wrap_attr_t<std::tuple_element_t<i, std::tuple<seq_attribs>>>>;

		constexpr static size_t seq_count = sizeof...(attribs);

		std::array<std::ptrdiff_t, seq_count + 1> offsets_{ 0 };


		constexpr std::ptrdiff_t assign_offsets(convert_to<size_t, attribs> ... instances)
		{
			((offsets_[indx + 1] += offsets_[indx] +
				sequence_i<indx>::elem_size * instances), ...);
			return offsets_[seq_count];
		}

		constexpr aggregated_sequences(Buffer_base& buf, 
			const std::array<std::ptrdiff_t, seq_count + 1>& offsets)
			: sequence_i<indx>(buf, offsets[indx], offsets[indx + 1]) ...
		{}

		using sequence_i<indx>::SeqN...;
		using sequence_i<indx>::operator()...;

		using sequence_i<0>::operator sequence_i<0>&;
		using sequence_i<0>::operator const sequence_i<0>&;

	};


	// Batched buffer
	template <class ... attribs>
	class Buffer : public Buffer_base,
		public aggregated_sequences<std::tuple<attribs...>> // TODO: remove public
	{
		using aggr_sequences = aggregated_sequences<std::tuple<attribs...>>;

		using aggr_sequences::seq_count;


		

	public:
        constexpr BufferPacked(HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
			: Buffer_base(std::move(handle)),
			seq_base<indx>(unwrap_seq_t<attribs>(*this,
				offsets_[indx], offsets_[indx + 1]))...
		{}

        // TODO: add allocating constructor

		using Buffer_base::Bind;
		using Buffer_base::IsBound;
		using Buffer_base::UnBind;
		using Buffer_base::IsMapped;

        using seq_base<indx>::SeqN ...;
        using seq_base<indx>::operator() ...;

        // Sequence<SingleSeq>::Allocated;
        // using Sequence<SingleSeq>::SubData;

		void AllocateMemory(convert_to<size_t, attribs> ...  instances,
			BufUsage usage)
		{
			assert(IsBound() && "Allocating memory for non-bound buffer!");

			ptrdiff_t totalSize = assign_offsets(instances..., 
				std::make_index_sequence<seq_count>());

			glBufferData((GLenum)Bound(),
				(GLsizei)totalSize,
				nullptr,
				(GLenum)usage);

			// TODO: debug check for opengl errors

			currentUsage_ = usage;
		}

        // TODO:
        // - MapData
        // - SeqN
        // - MapBufferRange from the first sequence
        // - SubData function from the first sequence

        // - AttribPointer function from the first sequence 
        // - user-defined conversion to the first sequence
        // - user-defined conversion to the first attribute of the first sequence
	};

    /*
	// TODO 
	template <size_t indx, class SingleSeq>
	class Buffer<sub_tag<indx, SingleSeq>> : virtual protected Buffer_base
	{
		static_assert(!is_compound_seq_v<SingleSeq> &&
			"Compound sequences are not allowed!");

		const std::ptrdiff_t &bytes_lbound_,
			&bytes_rbound_;

	protected:
		Buffer(const std::ptrdiff_t &bytes_lbound,
			const std::ptrdiff_t &bytes_rbound)
			: bytes_lbound_(bytes_lbound),
			bytes_rbound_(bytes_rbound)
		{}

	public:
		using Buffer_base::Bound;
		using Buffer_base::UnBind;
		using Buffer_base::IsMapped;

		

		void SubData(variable_traits_type<SingleSeq> *data, size_t sz, size_t inst_offset = 0)
		{
			assert(IsBound() && "Copying data to non-bound buffer!");
			assert(Allocated() >= sz + inst_offset && "Data exceeds buffer's bounds!");

			glBufferSubData((GLenum)Bound(),
				sizeof(variable_traits_type<SingleSeq>) * inst_offset,
				sizeof(variable_traits_type<SingleSeq>) * sz,
				data);
		}
	};
    */

}