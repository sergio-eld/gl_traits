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
    using wrap_attr_t = typename wrap_attr<T>::type;

	template <size_t indx, class AttrCompound, bool first = !indx>
	struct sequence_indexed;

	template <size_t indx, class ... Attr>
	struct sequence_indexed<indx, compound<Attr...>, false>
	{
		Sequence<Attr...> seq;

        constexpr static size_t elem_size = Sequence<Attr...>::elem_size;

		constexpr sequence_indexed(buffer_base& buf,
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
		using seq_base = sequence_indexed<0, compound<Attr...>, false>;

		constexpr sequence_indexed(buffer_base& buf,
			const std::ptrdiff_t &bytes_lbound,
			const std::ptrdiff_t &bytes_rbound)
			: seq_base(buf, bytes_lbound, bytes_rbound)
		{}

		using seq_base::SeqN;
        using seq_base::elem_size;

		constexpr const Sequence<Attr...>& SeqN() const
		{
			return seq_base::SeqN(tag_s<0>());
		}

        Sequence<Attr...>& SeqN()
		{
			return seq_base::SeqN(tag_s<0>());
		}

		constexpr const Sequence<Attr...>& operator()() const
		{
			return seq_base::SeqN(tag_s<0>());
		}

        Sequence<Attr...>& operator()()
		{
			return seq_base::SeqN(tag_s<0>());
		}

		constexpr operator const Sequence<Attr...>&() const
		{
            return SeqN();
		}

		operator Sequence<Attr...>&()
		{
            return SeqN();
        }


	};

	template <class TupleSeqAttr,
		class = decltype(std::make_index_sequence<std::tuple_size_v<TupleSeqAttr>>())>
		struct aggregated_sequences;

	template <class ... seq_attribs, size_t ... indx>
	struct aggregated_sequences<std::tuple<seq_attribs...>, std::index_sequence<indx...>> :
		public sequence_indexed<indx, wrap_attr_t<seq_attribs>> ...
	{
        constexpr static size_t seq_count = sizeof...(seq_attribs);

        std::array<std::ptrdiff_t, seq_count + 1> offsets_{ 0 };

        template <size_t i>
        using sequence_i =
            sequence_indexed<i, wrap_attr_t<std::tuple_element_t<i, std::tuple<seq_attribs...>>>>;

        constexpr aggregated_sequences(buffer_base& buf)
			: sequence_i<indx>(buf, offsets_[indx], offsets_[indx + 1]) ...
		{}

        constexpr std::ptrdiff_t assign_offsets(convert_to<size_t, seq_attribs> ... instances)
        {
            ((offsets_[indx + 1] += offsets_[indx] +
                sequence_i<indx>::elem_size * instances), ...);
            return offsets_[seq_count];
        }

        constexpr aggregated_sequences(buffer_base& buf,
            const std::array<std::ptrdiff_t, seq_count + 1>& offsets)
            : sequence_i<indx>(buf, offsets[indx], offsets[indx + 1]) ...
        {}

        aggregated_sequences(const aggregated_sequences&) = delete;
        aggregated_sequences& operator=(const aggregated_sequences&) = delete;

        using sequence_i<indx>::SeqN...;
        using sequence_i<indx>::operator()...;

//		using sequence_indexed<0, wrap_attr_t<seq_attribs>, true>::operator sequence_i<0>&;
//		using sequence_indexed<0, wrap_attr_t<seq_attribs>, true>::operator const sequence_i<0>&;

	};


	// Batched buffer
	template <class ... attribs>
	class Buffer : public buffer_base,
		public aggregated_sequences<std::tuple<attribs...>> // TODO: remove public
	{

		using aggr_sequences = aggregated_sequences<std::tuple<attribs...>>;

		using aggr_sequences::seq_count;


	public:
        constexpr Buffer(HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
			: buffer_base(std::move(handle)),
            aggr_sequences(static_cast<buffer_base&>(*this))
		{}

#pragma message("Need to check implementation for Buffer move constructor!")
		Buffer(Buffer&& other)
			: buffer_base(std::move(static_cast<buffer_base&&>(other))),
            aggr_sequences(static_cast<buffer_base&>(*this))
		{
			aggr_sequences::offsets_ = std::move(other.offsets_);
		}        
        // TODO: add allocating constructor

        void Bind(BufferTarget target)
        {
            using A = std::tuple_element_t<0, std::tuple<attribs...>>;
            using TA = typename sequence_traits<A>::first_type;

            if constexpr (seq_count != 1 ||
                is_compound_seq_v<A> ||
                !std::is_same_v<TA, GLubyte> &&
                !std::is_same_v<TA, GLushort> &&
                !std::is_same_v<TA, GLuint>)
                assert(target != BufferTarget::element_array &&
                    "Invalid buffer type for element array target!");


            buffer_base::Bind(target);
        }

        using buffer_base::IsBound;
		using buffer_base::UnBind;
		using buffer_base::IsMapped;

        using aggr_sequences::SeqN ...;
        using aggr_sequences::operator() ...;

        // Sequence<SingleSeq>::Allocated;
        // using Sequence<SingleSeq>::SubData;

		void AllocateMemory(convert_to<size_t, attribs> ...  instances,
			BufUsage usage)
		{
			assert(IsBound() && "Allocating memory for non-bound buffer!");

			ptrdiff_t totalSize =
                aggr_sequences::assign_offsets(instances...);

			glBufferData((GLenum)Bound(),
				(GLsizei)totalSize,
				nullptr,
				(GLenum)usage);

			// TODO: debug check for opengl errors
            assert(AssertGL());

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


}