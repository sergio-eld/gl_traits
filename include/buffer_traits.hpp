#pragma once

#include "basic_types.hpp"

namespace glt
{

    class Buffer_base
    {
        // this may be optimized out in release?
        static std::map<BufferTarget, Buffer_base*> targets_;

        static void Register(BufferTarget target, Buffer_base* ptr = nullptr)
        {
            Buffer_base *&current = targets_.at(target);
            if (current)
                current->bound_ = BufferTarget::none;

            current = ptr;
            if (ptr)
                ptr->bound_ = target;
        }

        BufferTarget bound_ = BufferTarget::none;
        bool mapped_ = false;

    protected:

        HandleBuffer handle_;

        BufUsage currentUsage_ = BufUsage::none;

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

        ~Buffer_base()
        {
            if (IsBound() != BufferTarget::none)
                Register(IsBound());
        }
    public:

        // Will register current buffer for the given target and unregister previous if any
        void Bind(BufferTarget target)
        {
            glBindBuffer((GLenum)target, handle_accessor(handle_));
            Register(target, this);
        }

        BufferTarget IsBound() const
        {
            return bound_;
        }

        void UnBind()
        {
            assert(IsBound() != BufferTarget::none && "Buffer is alraedy not bound");
            if (IsBound() == BufferTarget::none)
                throw std::exception("Trying to unbind non-bound buffer!");
            Register(bound_);
        }

    };


    // A - is a raw glsl type or named glsl type (wrapped)
    template <typename A>
    struct FetchedAttrib
    {
        static_assert(!is_compound_attr_v<A>, "Compound attributes are not allowed!");

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

    template <class A, bool = is_compound_attr_v<A>>
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


    // buffer attribute must know its offset (depends on its position via buffer)
    // buffer attribute must provide its stride
    // SubData
    // MapBufferRange
    // Attrib may be compound. If compound can't be named
    // Case for non-compound
    template <size_t indx, class A, bool compound = is_compound_attr_v<A>>
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
            assert(IsBound() != BufferTarget::none && "Fetching attribute of non-bound buffer!");
            if constexpr (is_compound_attr_v<A>)
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
        using Buffer_base::IsBound;
        using Buffer_base::UnBind;
        using buffer_attr<indx>::Fetch...;

        Buffer_packed(HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
            : Buffer_base(std::move(handle)),
            Buffer_attrib<indx, Attribs>(offsets_[indx])...
        {}

        void AllocateMemory(convert_to<size_t, Attribs> ... instances, BufUsage usage)
        {
            assert(IsBound() != BufferTarget::none && "Allocating memory for non-bound buffer!");
            size_t total_sz = ((sizeof(Attribs) * instances) + ...);

            // TODO: get name of the target this buffer is bound to (if bound)
            (assign<indx>(instances), ...);
            glBufferData((GLenum)IsBound(), offsets_[sizeof...(indx)], nullptr, (GLenum)usage);

            currentUsage_ = usage;
        }
    };

    template <class ... Attribs>
    using Buffer2 = Buffer_packed<std::tuple<Attribs...>>;
}