#pragma once

#include "basic_types.hpp"

namespace glt
{

	// store instances info here?
    class Buffer_base
    {
        // this may be optimized out in release?
		// map of pointers is senceless if Buffer is move-constructible,
		// unless it reregisteres on move-construction
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


        Buffer_base(HandleBuffer&& handle)
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

		Buffer_base(Buffer_base&& other)
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

        BufferTarget Bound() const
        {
            return target_;
        }

		bool IsBound() const
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

		bool IsMapped() const
		{
			return mapped_;
		}

    };


    // A - is a raw glsl type or named glsl type (wrapped)
    template <typename A>
    struct FetchedAttrib
    {
        static_assert(!is_compound_seq_v<A>, "Compound attributes are not allowed!");

        constexpr static GLint size = sizeof(variable_traits_type<A>);
        constexpr static GLenum glType = (GLenum)c_to_gl_v<variable_traits_type<A>>;

        // stride only depends on neighbor attributes with compound allignment
        GLsizei stride;
        std::ptrdiff_t offset;

        
        template <typename T>
        using ConvType = std::conditional_t<std::is_same_v<variable_traits_type<A>,
            variable_traits_type<T>>,
            FetchedAttrib<T>, void>;

        template <typename T>
        constexpr FetchedAttrib(FetchedAttrib<T>&& attrib)
            : stride(attrib.stride),
            offset(attrib.offset)
        {
            static_assert(std::is_same_v<variable_traits_type<A>, variable_traits_type<T>>,
                "Attributes' types must be the same!");
        }

        template <typename T>
        constexpr operator ConvType<T>() const
        {
            return FetchedAttrib<T>(stride, offset);
        }

    // private:
        template <size_t indx, class A, bool>
        class Buffer_attrib;

        constexpr FetchedAttrib(GLsizei stride = 0, std::ptrdiff_t offset = 0)
            : stride(stride),
            offset(offset)
        {}

    };

    template <class A, bool = is_compound_seq_v<A>>
    struct get_fetch_type
    {
        using type = std::tuple_element_t<0, A>;
    };

    template <class A>
    struct get_fetch_type<A, false>
    {
        using type = variable_traits_type<A>;
    };

    template <class A>
    using fetch_type = typename get_fetch_type<A>::type;

	template <size_t indx, class Attr>
	struct sub_tag {};

	// Batched buffer
	template <class SingleSeq>
	class Buffer : protected Buffer_base
	{
		size_t instanses_ = 0;

	public:
		Buffer(HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
			: Buffer_base(std::move(handle))
		{}

		Buffer(size_t inst, HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
			: Buffer_base(std::move(handle))
			// TODO: allocate inst
		{}

		using Buffer_base::Bind;
		using Buffer_base::IsBound;
		using Buffer_base::UnBind;
		using Buffer_base::IsMapped;

		void AllocateMemory(size_t instances, BufUsage usage)
		{
			assert(IsBound() && "Allocating memory for non-bound buffer!");

			glBufferData((GLenum)Bound(), 
				sizeof(variable_traits_type<SingleSeq>) * instances,
				nullptr,
				(GLenum)usage);

			instanses_ = instances;
			currentUsage_ = usage;
		}

		size_t Allocated() const
		{
			return instanses_;
		}

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

		size_t Allocated() const
		{
			return (bytes_rbound_- bytes_lbound_) / 
				sizeof(variable_traits_type<SingleSeq>);
		}

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







    // buffer attribute must know its offset (depends on its position via buffer)
    // buffer attribute must provide its stride
    // SubData
    // MapBufferRange
    // Attrib may be compound. If compound can't be named
    // Case for non-compound
    template <size_t indx, class A, bool compound = is_compound_seq_v<A>>
    class Buffer_attrib : protected virtual Buffer_base
    {
        // in bytes
        const std::ptrdiff_t& offset_;

    protected:
        Buffer_attrib(const std::ptrdiff_t& offset)
            : offset_(offset)
        {}

    public:

        // compound buffer by default will return its first attribute
        constexpr FetchedAttrib<fetch_type<A>> Fetch(tag_s<indx>) const
        {
            assert(Bound() != BufferTarget::none && "Fetching attribute of non-bound buffer!");
            if constexpr (is_compound_seq_v<A>)
                return { (GLsizei)class_size_from_tuple_v<A>, offset_ };
            else
                return { 0, offset_ };
        }
    };

    


    template <class tupleAttribs, class =
        decltype(std::make_index_sequence<std::tuple_size_v<tupleAttribs>>())>
        class Buffer_packed;

    template <class ... Attribs, size_t ... indx>
    class Buffer_packed<std::tuple<Attribs...>, std::index_sequence<indx...>> :
        Buffer_attrib<indx, Attribs> ...
    {
        // n + 1 to use array to deduce number of elements allocated
        std::array<std::ptrdiff_t, sizeof...(Attribs) + 1> offsets_{ 0 };

        template <size_t i>
        constexpr static size_t get_attrib_size()
        {
            // TODO: compound case

            return sizeof(std::tuple_element_t<i, std::tuple<Attribs...>>);
        }

        template <size_t i>
        constexpr void assign(size_t inst)
        {
            constexpr size_t type_s = get_attrib_size<i>();
            offsets_[i + 1] = offsets_[i] + inst * type_s;
        }

        template <size_t indx>
        using buffer_attr = Buffer_attrib<indx, std::tuple_element_t<indx, std::tuple<Attribs...>>>;

    public:

        using Buffer_base::Bind;
        using Buffer_base::Bound;
        using Buffer_base::UnBind;
        using buffer_attr<indx>::Fetch...;

        Buffer_packed(HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
            : Buffer_base(std::move(handle)),
            Buffer_attrib<indx, Attribs>(offsets_[indx])...
        {}

        void AllocateMemory(convert_to<size_t, Attribs> ... instances, BufUsage usage)
        {
            assert(Bound() != BufferTarget::none && "Allocating memory for non-bound buffer!");
            size_t total_sz = ((sizeof(Attribs) * instances) + ...);

            // TODO: get name of the target this buffer is bound to (if bound)
            (assign<indx>(instances), ...);
            glBufferData((GLenum)Bound(), offsets_[sizeof...(indx)], nullptr, (GLenum)usage);

            currentUsage_ = usage;
        }
    };

    template <class ... Attribs>
    using Buffer2 = Buffer_packed<std::tuple<Attribs...>>;
}