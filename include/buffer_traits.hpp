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

        Buffer_base()
            : handle_(Allocator::Allocate(BufferTarget()))
        {
            assert(false && "Buffer_base default constructor called!");
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

    private:
        template <class SingleSeq, bool>
        friend class Sequence;

        constexpr AttribPtr(std::ptrdiff_t offset = 0, GLsizei stride = 0)
            : stride(stride),
            offset(offset)
        {}

    };

    template <class CompoundSeq, bool = is_compound_seq_v<CompoundSeq>>
    struct create_indx_seq
    {
        using type = decltype(std::make_index_sequence<std::tuple_size_v<CompoundSeq>>());
    };

    template <class BatchedSeq>
    struct create_indx_seq<BatchedSeq, false>
    {
        using type = std::index_sequence<0>;
    };

    template <class Seq>
    using create_indx_seq_t = typename create_indx_seq<Seq>::type;

    /* Responsibilities:
    - SubData
    - Fetch Attribute
    - Map Buffer Range
    */
    template <class SingleSeq, bool = is_compound_seq_v<SingleSeq>>//,
        //class = create_indx_seq_t<SingleSeq>>
    class Sequence : protected virtual Buffer_base
    {
        const std::ptrdiff_t &bytes_lbound_,
            &bytes_rbound_;

        template <class T, bool = is_compound_seq_v<T>>
        struct seq_attr_type
        {
            using type = T;
        };

        template <class T>
        struct seq_attr_type<T, false>
        {
            using type = variable_traits_type<T>;
        };

        template <class T, bool = is_compound_seq_v<T>>
        struct seq_elem_size
        {
            constexpr static size_t size = class_size_from_tuple_v<T>;
        };

        template <class T>
        struct seq_elem_size<T, false>
        {
            constexpr static size_t size = sizeof(variable_traits_type<SingleSeq>);
        };

    protected:
        Sequence(const std::ptrdiff_t &bytes_lbound,
            const std::ptrdiff_t &bytes_rbound)
            : bytes_lbound_(bytes_lbound),
            bytes_rbound_(bytes_rbound)
        {}

    public:

        using AttrT = typename seq_attr_type<SingleSeq>::type;
        constexpr static size_t elem_size = seq_elem_size<AttrT>::size;

        // TODO: add template function with equivalence check
        void SubData(AttrT *data, size_t sz, size_t inst_offset = 0)
        {
            // TODO: add check if memory has been allocated
            assert(!IsMapped() && "Copying data to mapped buffer!");
            assert(IsBound() && "Copying data to non-bound buffer!");
            assert(Allocated() >= sz + inst_offset && "Data exceeds buffer's bounds!");

            glBufferSubData((GLenum)Bound(),
                elem_size * inst_offset,
                elem_size * sz,
                data);
        }

        size_t Allocated() const
        {
            return (bytes_rbound_ - bytes_lbound_) /
                elem_size;
        }

        GLsizei OffsetBytes() const
        {
            return bytes_lbound_;
        }

        // not accessible for compound specialization
        constexpr AttribPtr<SingleSeq> operator()(size_t instOffset = 0) const
        {
            assert(Allocated() >= instOffset && "Pointer exceeds sequence's bounds!");
            return AttribPtr<SingleSeq>(OffsetBytes() + sizeof(SingleSeq) * instOffset);
        }

        constexpr operator AttribPtr<SingleSeq>() const
        {
            return (*this)();
        }

    };

    template <size_t indx, class Attr, class ... AllAttr>
    struct seq_attrib_pointer //: protected virtual Sequence<std::tuple<AllAttr...>, false>
    {

    };

    template <class ... Attr>//, size_t ... indx>
    class Sequence<compound<Attr...>, true>//,
        //std::index_sequence<indx...>> 
        : protected Sequence<compound<Attr...>, false>
    {

        using seq_base = Sequence<compound<Attr...>, false>;

    protected:

        Sequence(const std::ptrdiff_t &bytes_lbound,
            const std::ptrdiff_t &bytes_rbound)
            : seq_base(bytes_lbound, bytes_rbound)
        {}

    public:
        using seq_base::Allocated;
        using seq_base::OffsetBytes;

        /*
        constexpr AttribPtr<SingleSeq> operator()( size_t instOffset = 0) const
        {
            assert(Allocated() >= instOffset && "Pointer exceeds sequence's bounds!");
            return AttribPtr<SingleSeq>(OffsetBytes() + sizeof(SingleSeq) * instOffset);
        }*/


    };


	// Batched buffer
	template <class SingleSeq>
	class BufferSingle : public Sequence<SingleSeq>
	{
        std::array<std::ptrdiff_t, 2> offsets_{ 0 };

	public:
        BufferSingle(HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
			: Buffer_base(std::move(handle)),
            Sequence<SingleSeq>(offsets_[0], offsets_[1])
		{}

        BufferSingle(size_t inst, HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
			: Buffer_base(std::move(handle)),
            Sequence<SingleSeq>(offsets_[0], offsets_[1])
		{}

		using Buffer_base::Bind;
		using Buffer_base::IsBound;
		using Buffer_base::UnBind;
		using Buffer_base::IsMapped;

        using Sequence<SingleSeq>::Allocated;
        using Sequence<SingleSeq>::SubData;

        // constexpr static size_t elem_size = seq_elem_size<SingleSeq>::size;

		void AllocateMemory(size_t instances, BufUsage usage)
		{
			assert(IsBound() && "Allocating memory for non-bound buffer!");

			glBufferData((GLenum)Bound(), 
                elem_size * instances,
				nullptr,
				(GLenum)usage);

            offsets_[1] = elem_size * instances;
			currentUsage_ = usage;
		}

        operator const Sequence<SingleSeq>&() const
        {
            return static_cast<const Sequence<SingleSeq>&>(*this);
        }

        operator Sequence<SingleSeq>&()
        {
            return static_cast<Sequence<SingleSeq>&>(*this);
        }

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