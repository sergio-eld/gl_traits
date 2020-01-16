#pragma once

#include "basic_types.hpp"

namespace glt
{

	// store instances info here?
	class Buffer_base
	{
		// this may be optimized out in release?
		static std::map<BufferTarget, Buffer_base*> targets_;

		// will unregister previous buffer and register new ptr
		static void Register(BufferTarget target, Buffer_base* ptr = nullptr)
		{
			Buffer_base *&current = targets_[target];
			if (current)
				current->target_ = BufferTarget::none;

			current = ptr;
		}


	protected:

		HandleBuffer handle_;

		// one buffer can be mapped to multiple targets (?)
		BufferTarget target_ = BufferTarget::none;
		BufUsage currentUsage_ = BufUsage::none;

		// replace bool by map type (?)
		bool mapped_ = false;


		constexpr Buffer_base(HandleBuffer&& handle)
			: handle_(std::move(handle))
		{
			assert(handle_ && "Invalid Handle!");
		}

		Buffer_base(const Buffer_base&) = delete;
		Buffer_base& operator=(const Buffer_base& other) = delete;

		constexpr Buffer_base(Buffer_base&& other)
			: handle_(std::move(other.handle_)),
			mapped_(other.mapped_)
		{
			if (other.Bound() != BufferTarget::none)
				Bind(other.Bound());
			other.mapped_ = false;
		}

		Buffer_base& operator=(Buffer_base&& other)
		{
			handle_ = std::move(other.handle_);
			mapped_ = other.mapped_;

			if (other.Bound() != BufferTarget::none)
				Bind(other.Bound());
			other.mapped_ = false;

			return *this;
		}


		~Buffer_base()
		{
			if (Bound() != BufferTarget::none)
				Register(Bound());
		}
	public:

		// Will register current buffer for the given target and unregister previous if any
		void Bind(BufferTarget target)
		{
			glBindBuffer((GLenum)target, handle_accessor(handle_));
			Register(target, this);
			target_ = target;
		}

		constexpr BufferTarget Bound() const
		{
			return target_;
		}

		constexpr bool IsBound() const
		{
			return Bound() != BufferTarget::none;
		}

		void UnBind()
		{
			assert(IsBound() && "Buffer is alraedy not bound");
			if (Bound() == BufferTarget::none)
				throw std::exception("Trying to unbind non-bound buffer!");
			Register(target_);
		}

		constexpr bool IsMapped() const
		{
			return mapped_;
		}

	};


	// A - is a raw glsl type or named glsl type (wrapped)
	template <typename A>
	struct AttribPtr
	{
		static_assert(!is_compound_seq_v<A>, "Compound attributes are not allowed!");

		constexpr static GLint size = sizeof(variable_traits_type<A>);
		constexpr static GLenum glType = (GLenum)c_to_gl_v<variable_traits_type<A>>;

		// stride only depends on neighbor attributes with compound allignment
		std::ptrdiff_t offset;
		GLsizei stride;

        /*
		template <typename T>
		using ConvType = std::conditional_t<std::is_same_v<variable_traits_type<A>,
			variable_traits_type<T>>,
			AttribPtr<T>, void>;

		template <typename T>
		constexpr AttribPtr(AttribPtr<T>&& attrib)
			: stride(attrib.stride),
			offset(attrib.offset)
		{
			static_assert(std::is_same_v<variable_traits_type<A>, variable_traits_type<T>>,
				"Attributes' types must be the same!");
		}

		template <typename T>
		constexpr operator ConvType<T>() const
		{
			return AttribPtr<T>(stride, offset);
		}

		// private:
		template <class SingleSeq, bool>
		friend class Sequence;

		//template <class, class, size_t, size_t, bool, class>
		//friend class get_attrib_ptr_i;
        */

		constexpr AttribPtr(std::ptrdiff_t offset = 0, GLsizei stride = 0)
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
	template <class CRTPSeq, class Attr, size_t indx, bool first = !indx> // false case
    class get_attrib_ptr_i
    {
        const CRTPSeq& seq_;

    public:

        constexpr get_attrib_ptr_i(const CRTPSeq& seq)
            : seq_(seq)
        {}

        constexpr AttribPtr<Attr> AttribPointer(tag_s<indx>, size_t instOffset = 0) const
        {
            assert(seq_.Allocated() >= instOffset && "Pointer exceeds sequence's bounds!");
            return AttribPtr<Attr>(CRTPSeq::AttrOffsetBytes(tag_s<indx>()) + seq_.BufferOffsetBytes() +
                CRTPSeq::elem_size * instOffset, CRTPSeq::Stride());
        }

        constexpr AttribPtr<Attr> operator()(tag_s<indx>, size_t instOffset = 0) const
        {
            return AttribPointer(tag_s<indx>(), instOffset);
        }

    };

    template <class CRTPSeq, class Attr>
    class get_attrib_ptr_i<CRTPSeq, Attr, 0, true> : 
        public get_attrib_ptr_i<CRTPSeq, Attr, 0, false>
	{
        using base = get_attrib_ptr_i<CRTPSeq, Attr, 0, false>;

    public:

        constexpr get_attrib_ptr_i(const CRTPSeq& seq)
            : get_attrib_ptr_i<CRTPSeq, Attr, 0, false>(seq)
        {}

        constexpr AttribPtr<Attr> operator()(size_t instOffset = 0) const
        {
            return (*this)(tag_s<0>(), instOffset);
        }

        constexpr operator AttribPtr<Attr>() const
        {
            return (*this)(tag_s<0>());
        }

        using base::operator();
        using base::AttribPointer;

	};



	template <class CRTPSeq, class CompCollect,
		class = decltype(std::make_index_sequence<seq_elem_count<CompCollect>>())>
		class get_attrib_ptr;

	template <class CRTPSeq, class ... Attr, size_t ... indx>
	class get_attrib_ptr<CRTPSeq, compound<Attr...>, std::index_sequence<indx...>>
		: public get_attrib_ptr_i<CRTPSeq, Attr, indx> ...
	{
		template <size_t i>
		using ith_type = std::tuple_element_t<i, std::tuple<Attr...>>;

		template <size_t i>
		using get_attrib_i = get_attrib_ptr_i<CRTPSeq, ith_type<i>, i>;

        //template <size_t i>
        //using attrib_operator_i = get_attrib_i<i>::operator();

	public:
		constexpr get_attrib_ptr(const CRTPSeq& seq)
			: get_attrib_ptr_i<CRTPSeq, Attr, indx>(seq) ...
		{}

		using get_attrib_i<indx>::AttribPointer ...;
        using get_attrib_i<indx>::operator() ...;
        using get_attrib_i<0>::operator AttribPtr<ith_type<0>>;
	};

	/* Template "API" class for Sequence:

	constexpr static size_t elem_size;

	template <size_t indx>
	constexpr static std::ptrdiff_t AttrOffsetBytes(glt::tag_s<indx>);

	constexpr static GLsizei Stride();
	constexpr size_t Allocated() const;

	constexpr std::ptrdiff_t BufferOffsetBytes() const;
	constexpr operator glt::AttribPtr
	constexpr glt::AttribPtr operator()
	*/

    template <typename ... Attribs>
    class SequenceCRTP
    {
        const std::ptrdiff_t &bytes_lbound_,
            &bytes_rbound_;

    public:

        constexpr static size_t elem_size =
            seq_elem_size<compound<Attribs...>>;

        template <size_t indx>
        constexpr static std::ptrdiff_t AttrOffsetBytes(tag_s<indx>)
        {
            return get_tuple_member_offset_v<indx, std::tuple<Attribs...>>;
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
        constexpr SequenceCRTP(const std::ptrdiff_t &bytes_lbound,
            const std::ptrdiff_t &bytes_rbound)
            : bytes_lbound_(bytes_lbound),
            bytes_rbound_(bytes_rbound)
        {}
    };

    /* Responsibilities:
    - AttribPtr
    - SubData
    - Map Buffer Range
    */
    template <typename ... Attribs>
    class Sequence : public SequenceCRTP<Attribs...>,
        glt::get_attrib_ptr<SequenceCRTP<Attribs...>, glt::compound<Attribs...>>
    {
        using get_attrib = glt::get_attrib_ptr<SequenceCRTP<Attribs...>, 
			glt::compound<Attribs...>>;

        const Buffer_base& buf_;

    // protected: // compoistion won't work
	public:
        Sequence(const Buffer_base& buf,
            const std::ptrdiff_t &bytes_lbound,
            const std::ptrdiff_t &bytes_rbound)
            : SequenceCRTP<Attribs...>(bytes_lbound, bytes_rbound),
            get_attrib(static_cast<const SequenceCRTP<Attribs...>&>(*this)),
            buf_(buf)
        {}

    public:

        using get_attrib::AttribPointer;
        using get_attrib::operator();
        using get_attrib::operator glt::AttribPtr<std::tuple_element_t<0,
            std::tuple<Attribs...>>>;

        // TODO: add template function with equivalence check
        template <class AttrT, 
            class = std::enable_if_t<is_tuple_equivalent_v<AttrT, std::tuple<Attribs...>>>>
        void SubData(AttrT *data, size_t sz, size_t inst_offset = 0)
        {
            assert(!IsMapped() && "Copying data to mapped buffer!");
            assert(IsBound() && "Copying data to non-bound buffer!");
            assert(Allocated() >= sz + inst_offset && "Data exceeds buffer's bounds!");

            glBufferSubData((GLenum)Bound(),
                elem_size * inst_offset,
                elem_size * sz,
                data);
        }

    };

    template <class attr, bool = is_compound_seq_v<attr>>
    struct unwrap_seq
    {
        using sequence = Sequence<attr>;
    };

    template <class ... attr>
    struct unwrap_seq<compound<attr...>, true>
    {
        using sequence = Sequence<attr...>;
    };

    template <typename T>
    using unwrap_seq_t = typename unwrap_seq<T>::sequence;

	// need to use this generic wrapper?
	template <size_t indx, class T>
	struct indexed_wrapper
	{
		T wrapped;

		constexpr indexed_wrapper(T&& wrapped)
			: wrapped(std::forward<T>(wrapped))
		{}

		constexpr const T& GetIndx(tag_s<indx>) const
		{
			return wrapped;
		}

		T& GetIndx(tag_s<indx>)
		{
			return wrapped;
		}
	};

	template <size_t indx, class Seq, bool first = !indx>
	struct sequence_indexed
	{
		Seq seq;

		constexpr sequence_indexed(Seq&& seq)
			: seq(std::forward<Seq>(seq))
		{}

		constexpr const Seq& SeqN(tag_s<indx>) const
		{
			return seq;
		}

		Seq& SeqN(tag_s<indx>)
		{
			return seq;
		}
	};

	template <class Seq>
	struct sequence_indexed<0, Seq, true> :
		public sequence_indexed<0, Seq, false>
	{
		using seq_base = sequence_indexed<0, Seq, false>;

		constexpr sequence_indexed(Seq&& seq)
			: seq_base(std::forward<Seq>(seq))
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

		constexpr operator const Seq&() const
		{
			return seq_base::SeqN(tag_s<0>());
		}

		operator Seq&()
		{
			return seq_base::SeqN(tag_s<0>());
		}
	};



	/* Wrapper class to allow access to the Sequence by index within a 
	Containing Buffer
	*/
	/*
	template <size_t indx, class ... Attr>
	struct indexed_wrapper<indx, Sequence<Attr...>> : public Sequence<Attr...>
	{
		constexpr const Sequence<Attr...>& Seq(tag_s<indx>) const
		{
			return static_cast<const Sequence<Attr...>&>(*this);
		}
	};*/

	/*
	template <class ... Attr>
	struct indexed_wrapper<0, Sequence<Attr...>> : public Sequence<Attr...>
	{
		constexpr const Sequence<Attr...>& Seq() const
		{
			return static_cast<const Sequence<Attr...>&>(*this);
		}
	};*/

	template <class Packed,
		class = decltype(std::make_index_sequence<std::tuple_size_v<Packed>>())>
		class BufferPacked;


	// Batched buffer
	template <class ... attribs, size_t ... indx>
	class BufferPacked<compound<attribs...>, std::index_sequence<indx...>>
		: public Buffer_base, public sequence_indexed<indx, unwrap_seq_t<attribs>>...
	{
		template <size_t i>
		using nth_attrib = std::tuple_element_t<i, std::tuple<attribs...>>;

		template <size_t i>
		using seq_base = sequence_indexed<i, unwrap_seq_t<nth_attrib<i>>>;

		constexpr static size_t seq_count = sizeof...(attribs);

        std::array<std::ptrdiff_t, seq_count + 1> offsets_{ 0 };

		template <size_t ... indx>
		constexpr std::ptrdiff_t assign_offsets(convert_to<size_t, attribs> ... instances,
			std::index_sequence<indx...>)
		{
			((offsets_[indx + 1] += offsets_[indx] +
				unwrap_seq_t<attribs>::elem_size * instances), ...);
			return offsets_[seq_count];
		}

	public:
        constexpr BufferPacked(HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
			: Buffer_base(std::move(handle)),
			//sequence_indexed<indx, unwrap_seq_t<attribs>>
			seq_base<indx>(unwrap_seq_t<attribs>(static_cast<const Buffer_base&>(*this),
				offsets_[indx], offsets_[indx + 1]))...
		{}

        /*
        Buffer(size_t inst, HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
			: Buffer_base(std::move(handle)),
            Sequence<SingleSeq>(offsets_[0], offsets_[1])
		{}*/

		using Buffer_base::Bind;
		using Buffer_base::IsBound;
		using Buffer_base::UnBind;
		using Buffer_base::IsMapped;

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

			// TODO: debug check if data has been buffered

			currentUsage_ = usage;
		}

	};

	template <class ... Sequences>
	using Buffer = BufferPacked<compound<Sequences...>>;

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