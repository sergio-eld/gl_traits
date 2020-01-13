#pragma once


#include "traits.hpp"
#include "enums.hpp"

#include <map>

namespace glt
{
    // constructs handles
    template <typename>
    struct AllocatorSpecific;

    template <typename eTargetType>
    class handle_accessor;

	/*
	Handle is unique. Frees OpenGL resources on destruction.

	Need to implement shared handle?
	*/
    template <typename eTargetType>
    class Handle
    {
        //TODO: add Glsync handle type
        GLuint handle_;// = 0;

        friend struct AllocatorSpecific<eTargetType>;
        friend class handle_accessor<eTargetType>;


    public:

        // recover pointer to deleter function pointer
        constexpr static auto ppDeleteFunc = pp_gl_deleter_v<eTargetType>;

        Handle(const Handle<eTargetType>& other) = delete;
        Handle<eTargetType>& operator=(const Handle<eTargetType>& other) = delete;

		constexpr Handle(Handle<eTargetType>&& other)
            : handle_(other.handle_)
        {
            other.handle_ = 0;
        }
		Handle<eTargetType>& operator=(Handle<eTargetType>&& other)
        {
            if (handle_)
                DestroyHandle();
            handle_ = other.handle_;
            other.handle_ = 0;
            return *this;
        }

		constexpr bool operator==(GLuint raw_handle) const
        {
            return handle_ == raw_handle;
        }

		constexpr bool operator!=(GLuint raw_handle) const
        {
            return !operator==(raw_handle);
        }

		constexpr bool operator==(const Handle<eTargetType>& other) const
        {
            return handle_ == other.handle_;
        }

		constexpr bool operator!=(const Handle<eTargetType>& other) const
        {
            return !operator==(other);
        }

		constexpr bool IsValid() const
        {
            return (bool)handle_;
        }

		constexpr operator bool() const
		{
			return IsValid();
		}

        // handle is not aware if it is or can be bound to a target
        ~Handle()
        {
            if (handle_)
            {
                // TODO: do i need to check?
                assert(*ppDeleteFunc && "Pointers to OpenGL deleter functions have not been initialiized!");

                // TODO: Unbind from bound_handle, OpenGL ubinds automaticallu after deleting

                // check function signature for arguments
                if constexpr (std::is_same_v<void(APIENTRYP *)(GLsizei, const GLuint*), pp_gl_deleter<eTargetType>::value_type>)
                    (*ppDeleteFunc)(1, &handle_);
                else if constexpr (std::is_same_v<void(APIENTRYP *)(GLuint), pp_gl_deleter<eTargetType>::value_type>)
                    (*ppDeleteFunc)(handle_);
                else
                    static_assert(false, "Unhandled case!");
            }
        }


    protected:

		constexpr Handle() = default;
        constexpr Handle(GLuint handle) noexcept
            : handle_(handle)
        {
			static_assert(sizeof(decltype(*this)) == sizeof(GLuint));
		}

        constexpr operator GLuint() const
        {
            return handle_;
        }

    };

	// traits for classes that has handle accessor functions
	template <class T, typename = std::void_t<>>
	struct has_GetHandle : std::false_type {};

	template <template <typename>  class T, typename TargetType>
	struct has_GetHandle<T<TargetType>,
		std::void_t<decltype(	// get type from function pointer
		(const Handle<TargetType>&(T<TargetType>::*)() const)	// cast function pointer to type
			&T<TargetType>::GetHandle)>> : std::true_type {};	// function pointer

    using HandleBuffer = Handle<BufferTarget>;
    using HandleVAO = Handle<VAOTarget>;
	using HandleShader = Handle<ShaderTarget>;
	using HandleProg = Handle<ProgramTarget>;

	using TargetTypes = std::tuple<BufferTarget,
		FrameBufferTarget,
		TextureTarget,
		VAOTarget,
		TransformFeedBackTarget,
		QueryTarget,
		ProgramPipeLineTarget,
		RenderBufferTarget,
		SamplerTarget,
		ShaderTarget,
		ProgramTarget>;


    // TODO: add responsibility to delete handle?
    // TODO: unbind handle when deleting?
    template <typename eTargetType>
    struct AllocatorSpecific
    {
        // retrieve pointer to allocator function pointer
        constexpr static auto ppAllocFunc = pp_gl_allocator<eTargetType>::value;

        //template <typename = std::enable_if_t<!std::is_same_v<ShaderTarget, decltype(target)>>>
        static Handle<eTargetType> Allocate(eTargetType)
        {
            // TODO: also check for deleter function pointers to be loaded
            // TODO: throw exception inside the function to which ppAllocFunc points by default
            /*if (!*ppAllocFunc)
                throw std::exception("Pointers to OpenGL allocator "
                    "functions have not been initialiized!");
                    */
            GLuint h = 0;
            if constexpr (std::is_same_v<void(APIENTRYP *)(GLsizei, GLuint*), pp_gl_allocator<eTargetType>::value_type>)
            {
                (*ppAllocFunc)(1, &h);
            }
            else if constexpr (std::is_same_v<GLuint(APIENTRYP *)(void), pp_gl_allocator<eTargetType>::value_type>)
                h = (*ppAllocFunc)();
            //else if constexpr (std::is_same_v<GLuint(APIENTRYP *)(GLenum), pp_gl_allocator<eTargetType>::value_type>)
            //    return Handle<eTargetType>((*ppAllocFunc)((GLenum)target));
            else
                static_assert(false, "Allocator::Unhandled case!");

            /* TODO: check handle at Buffers' (or other objects') constructors
            if (!h)
            {
                assert(false && "Failed to allocate handle!");
                throw("Failed to allocate handle");
            }*/
            return Handle<eTargetType>(h);

        }

        //template <typename = std::enable_if_t<!std::is_same_v<ShaderTarget, decltype(target)>>>
        Handle<eTargetType> operator()() const
        {
            return Allocate();
        }

    };

    template <>
    struct AllocatorSpecific<ShaderTarget>
    {
        constexpr static auto ppAllocFunc = pp_gl_allocator_v<ShaderTarget>;

        static Handle<ShaderTarget> Allocate(ShaderTarget target)
        {
            // TODO: also check for deleter function pointers to be loaded
            assert(*ppAllocFunc && "Pointers to OpenGL allocator functions have not been initialiized!");
            
            return Handle<ShaderTarget>((*ppAllocFunc)((GLenum)target));
        }

        Handle<ShaderTarget> operator()(ShaderTarget target) const
        {
            return Allocate(target);
        }
    };

	// TODO: add specialization for glProgram


	template <class TupleTargets,
		class = decltype(std::make_index_sequence<std::tuple_size_v<TupleTargets>>())>
		class AllocatorCommon;

	template <typename ... TargetTypes, size_t ... indx>
	class AllocatorCommon<std::tuple<TargetTypes...>, std::index_sequence<indx...>>
		: AllocatorSpecific<TargetTypes>...
	{
		template <size_t i>
		using AllocBase = AllocatorSpecific<std::tuple_element_t<i, std::tuple<TargetTypes...>>>;

	public:

		using AllocBase<indx>::Allocate...;
		using AllocBase<indx>::operator()...;
	};

	using Allocator = AllocatorCommon<TargetTypes>;

    template <typename eTargetType>
    class handle_accessor
    {
		GLuint raw_handle_ = 0;
	public:
		constexpr handle_accessor() = default;
		constexpr handle_accessor(const Handle<eTargetType>& handle) noexcept
			: raw_handle_(handle)
		{}

		constexpr operator GLint() const noexcept
		{
			assert(raw_handle_ && "handle_accessor::Invalid handle!");
			return raw_handle_;
		}
    };

	// TODO: replace with traits? (checking for "is_bindable" and "has_GetHandle")
    // base class for every object, that has glBind(target, handle) function
    template <typename eTargetType>
    struct IBindable
    {
        virtual const Handle<eTargetType>& GetHandle() const = 0;
    };


    /*
    Depending on OpenGL spec version and extensions, Named methods for GL objects
    may be accessible. Default versions of such functions (glMapBuffer, glBufferData, etc.)
    require these objects to be bound to a particular target.
    Need to implement compile-time (or maybe run-time) algorithm selection to choose between those
    functions.
    Default pipeline involves:
    1. Check that target is free
    2. Bind
    3. Do stuff
    4. Unbind

    Need one class template to check Bindable GL objects.
    - Use guards?

    */

    // TODO: replace with map?
    template <typename eTargetType, eTargetType target>
    class bound_handle_base
    {
    protected:
        inline static GLuint raw_handle_ = 0;

        static_assert(has_func_bind_v<eTargetType>, "Typename doesn't have a glBind function");

        // property cannot be retrieved using glGet(..._BINDING)
        static_assert(has_gl_binding_v<target>, "Target can not be bound");
    public:

        constexpr static auto ppBindFunc = pp_gl_binder_v<eTargetType>;
        constexpr static auto binding = get_binding_v<target>;

        static void Bind(tag_v<target>, const IBindable<eTargetType>& obj)
        {
            const Handle<eTargetType>& handle = obj.GetHandle();

			// TODO: remove check from here. Assign function pointers to default function
            assert(*ppBindFunc && "OpenGL Bind function has not been initialized!");

            (*ppBindFunc)((GLenum)target, handle_accessor(handle));
            raw_handle_ = handle_accessor(handle);
        }

        static void UnBind(tag_v<target>, const IBindable<eTargetType>& obj)
        {
            const Handle<eTargetType>& handle = obj.GetHandle();
            if (handle != raw_handle_)
            {
                assert(false && "Unbinding handle that is not bound!");
                throw("Unbinding handle that is not bound!");
            }
            assert(*ppBindFunc && "OpenGL Bind function has not been initialized!");

            (*ppBindFunc)((GLenum)target, 0);
            raw_handle_ = 0;
        }

        static bool IsBound(tag_v<target>, const IBindable<eTargetType>& obj)
        {
            const Handle<eTargetType>& handle = obj.GetHandle();
            // TODO: assert that returned by glGet object is equal to raw_handle_;
            // TODO: make an independent wrapper
            GLint res = 0;
            glGetIntegerv((GLenum)binding, &res);
            assert(res == raw_handle_);

            return bool(handle == raw_handle_);
        }
    };

    template <typename ... gl_cur_obj>
    struct bound_handle_collection : gl_cur_obj...
    {
        using gl_cur_obj::Bind...;
        using gl_cur_obj::UnBind...;
        using gl_cur_obj::IsBound...;

    };

    template <typename>
    class bound_handle;

    // Target enum collection template parameter as integer sequence
    template <typename eTargetType, eTargetType ... vals>
    class bound_handle<std::integer_sequence<eTargetType, vals...>> :
        public bound_handle_collection<bound_handle_base<eTargetType, vals> ...>
    {
        using this_t = bound_handle<std::integer_sequence<eTargetType, vals...>>;

        using pBindFunc = void(*)(tag_v<(eTargetType)0>, const IBindable<eTargetType>&);
        using pIsBoundFunc = bool(*)(tag_v<(eTargetType)0>, const IBindable<eTargetType>&);
        using pUnbindFunc = void(*)(tag_v<(eTargetType)0>, const IBindable<eTargetType>&);

        inline static std::map<eTargetType, void(*)()> pBindFuncsMap_
        {
            std::pair(vals,
            reinterpret_cast<void(*)()>(&bound_handle_base<eTargetType, vals>::Bind)) ...
        },
            pUnbindFuncsMap_
        {
            std::pair(vals,
            reinterpret_cast<void(*)()>(&bound_handle_base<eTargetType, vals>::UnBind)) ...
        },
            pIsBoundFuncsMap_
        {
            std::pair(vals,
            reinterpret_cast<void(*)()>(&bound_handle_base<eTargetType, vals>::IsBound)) ...
        };

    public:
        // TODO: add runtime versions 
        static void BindRT(eTargetType target, const IBindable<eTargetType>& handle)
        {
            auto found = pBindFuncsMap_.find(target);

            assert(found != pBindFuncsMap_.cend() &&
                "Target is not registered!");

            pBindFunc pBind = reinterpret_cast<pBindFunc>(found->second);
            (pBind)(tag_v<(eTargetType)0>(), handle);

        }

        static bool IsBoundRT(eTargetType target, const IBindable<eTargetType>& handle)
        {
            auto found = pIsBoundFuncsMap_.find(target);

            assert(found != pIsBoundFuncsMap_.cend() &&
                "Target is not registered!");

            pIsBoundFunc pIsBound = reinterpret_cast<pIsBoundFunc>(found->second);
            return (pIsBound)(tag_v<(eTargetType)0>(), handle);
        }

        static void UnBindRT(eTargetType target, const IBindable<eTargetType>& handle)
        {
            auto found = pUnbindFuncsMap_.find(target);

            assert(found != pUnbindFuncsMap_.cend() &&
                "Target is not registered!");

            pUnbindFunc pUnBind = reinterpret_cast<pUnbindFunc>(found->second);
            (pUnBind)(tag_v<(eTargetType)0>(), handle);
        }

    };

    // forcing generation
    // template class bound_handle<BufferTargetList>; // will throw

    using gltActiveBufferTargets = bound_handle<BufferTargetList>;





    /*
    OpenGL:

    Shader source contains:
    - layout of named attributes with (not necessarily) defined locations
    - named uniforms

    Buffers can store arrays of attributes:

    - one batched attribute array in one VBO (one attribute per array element)
    - array of compounds (several attributes per array element)
    - several batched arrays of attributes  in one VBO
    - first - compound, followed by multiple batched atribute arrays

    Each attribute/variable has a glsl Type and a name;

    Goal:
    1. Provide type info about Buffer Object's data:
        a. types of attributes stored (Type, Batched/Compound)
        b. order of attributes stored
        c. if the first array is compound
        d. (maybe) specific info about the attribute variable name
        e. (maybe) specific info about the attribute location number

    Comment on "1.d.". Name is needed to check the attribute location when
    VBO is passed to a Shader object to upload data. Would be completely typesafe
    in case Shader classes will be generated at precompile step from shader source file.

    Comment of "1.e". Not typesafe

    2. Accept containers with continious data to buffers:
        a. having the same type as template arguments
            glt_VBO<glm::Vec3> vbo;
            ...
            std::vector<glm::Vec3> data;
            vbo.Load(data);

        b. having type equivalent to that of template arguments
        (check using pod_reflection lib)

            struct Vertex
            {
                glm::Vec3 pos,
                    texture;
            }
            ...
            std::vector<Vertex> data;
            glt_VBO<glm::Vec3, glm::Vec2> vbo;
            vbo.Load(data);

        c. for buffers with multiple attributes provide location index
        or deduce:
            - explicitly passing the index number
            - if attribute type is unique
            - for named may use tag_v dispatching

    Cases:
    - VBO of unnamed attributes
        glVBO<glm::Vec3, glm::Vec3, int, int>

    - VBO with first compound maybe followed by several unnamed attributes
        glVBO<compound<glm::Vec3, glm::Vec2>, int, int>

    - VBO with named attributes
        glVBO<glslt<glm::Vec3, gl_pos>, glslt<glm::Vec2, gl_tex>> (gl_pos = "pos", gl_tex = "tex")
        glVBO<compound<glslt<glm::Vec3, gl_pos>, glslt<glm::Vec2, gl_tex>>>

    TODO: define all attribute validations here (at the beginning)
    */




    ////////////////////////////////////////////////////
    // helpers. Move to another Module?
    ////////////////////////////////////////////////////



    template <typename T, size_t sz, class = decltype(std::make_index_sequence<sz>())>
    struct gen_tuple;

    template <typename T, size_t sz, size_t ... indx>
    struct gen_tuple<T, sz, std::index_sequence<indx...>>
    {
        using type = std::tuple<convert_v_to<T, indx> ...>;
    };

    template <typename T, size_t sz>
    using gen_tuple_t = typename gen_tuple<T, sz>::type;

    template <typename T, size_t sz, class = decltype(std::make_index_sequence<sz>())>
    struct init_array;

    template <typename T, size_t sz, size_t ... Indx>
    struct init_array<T, sz, std::index_sequence<Indx...>>
    {
        constexpr static std::array<T, sz> init(T def_value)
        {
            return std::array<T, sz>{ convert_v_to<T, Indx>(def_value) ... };
        }

        constexpr std::array<T, sz> operator()(T def_value) const
        {
            return init(def_value);
        }
    };

    template <typename T, size_t sz, class = decltype(std::make_index_sequence<sz>())>
    struct modify_array;

    template <typename T, size_t sz, size_t ... indx>
    struct modify_array<T, sz, std::index_sequence<indx...>>
    {
        template<size_t i>
        constexpr static void set(std::array<T, sz>& arr, const T& val)
        {
            arr[i] = val;
        }

        constexpr static void modify(std::array<T, sz>& arr, const convert_v_to<T, indx>& ... vals)
        {
            (set<indx>(arr, vals), ...);
        }
    };

    /*
    To check if Attribute is equivalent to Input type.

    Decay Attribute and input classes to tuples and compare them.

    Defined cases for input:
    - glm class
    - user defined class, equivalent to Attribute (same size and layout)
    - std array (not implemented yet) i.e. glm::vec3 is same as std::array<3, float>
    - other fixed sized containers??

    Defined cases for Attribute:
    - glm type
    - compound type (several attributes)
    - named type
    - custom shader-defined type: are all of them PODs?

    Shader-Defined custom structs must be able to be expanded to tuple/compound
    via template function

    Equivalence Table
    ---------------------------------
    |	| Attribute Type			| Input Type		|	Eval	|
    |---|---------------------------|-----------		|-----------|
    | 0	|		Simple				|	Simple			|	??		|
    | 1 |		Simple				| Compound<I>		|	??(0)	|
    | 2 |		Simple				| Compound<I...>	|	-> 0	|
    | 3*|		Compound<T>			|	Simple			|	??(0)	|
    | 4 |		Compound<T...>		|	Simple			|	-> 0	|
    | 5	|		Compound<T...>		|	Compund<T...>	|	-> 0	|

    "*": Cases unlikely to occur
    "-> n": resolves to case n;
    "??(n)": specialization of case n
    */

    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////

    template <class>
    class MapStatus_;

    // this can be acquired via OpenGL function
    template <BufferTarget ... targets>
    class MapStatus_<std::integer_sequence<BufferTarget, targets...>>
    {
        inline static std::map<BufferTarget, MapAccess>
            mapStatus_{ std::pair(targets, (MapAccess)0) ... };

    public:

        static MapAccess MappedStatus(BufferTarget target)
        {
            auto found = mapStatus_.find(target);
            assert(found != mapStatus_.cend() && "Target is not registered!");
            // or use glGet ?

            return found->second;
        }

        static bool IsMapped(BufferTarget target)
        {
            return (bool)MappedStatus(target);
        }

        static void SetMapStatus(BufferTarget target, MapAccess type)
        {
            auto found = mapStatus_.find(target);
            assert(found != mapStatus_.cend() && "Target is not registered!");
            found->second = type;
        }
    };

    using MapStatus = MapStatus_<BufferTargetList>;

    // TODO: unite with Buffer and replace with gltMapGuard?
    // or use the same collection of template attribute params:
    // 
    template <typename T, MapAccess S = MapAccess::read_write>
    class BufferMap
    {
        //const IBindable<BufferTarget> *handle_;
        BufferTarget target_;
        T *start_,
            *end_;
        bool unmapped_ = false;

    public:
        BufferMap(T* start,
            T* end,
            BufferTarget target)
            : start_(start),
            end_(end),
            target_(target)
        {
            // maybe this checks should do Buffer class? 
            // Buffer also has to check for mapping when allocating memory or buffering data!
            //if (!bound_handle<BufferTargetList>::IsBoundRT(target_, handle_))
                //throw("Attempting to Map a buffer which has not been bound!");

            assert(end_ > start_ && "Invalid range!");
        }

        BufferMap(const BufferMap<T, S>&) = delete;
        BufferMap& operator=(const BufferMap<T, S>&) = delete;

        //template <MapAccess S2>
        BufferMap(BufferMap<T, S>&& other)
            : target_(other.target_),
            start_(other.start_),
            end_(other.end_),
            unmapped_(other.unmapped_)
        {
            other.start_ = nullptr;
            other.end_ = nullptr;
            other.unmapped_ = true;
        }

        BufferMap& operator=(BufferMap<T, S>&& other)
        {
            // two Maps may have different targets, current one must be unmapped
            if (!unmapped_)
            {
                assert(other.unmapped_ || target_ == other.target_ &&
                    "Invalid scenario! Both maps are mapped to the same target!");
                UnMap();
            }

            target_ = other.target_;
            start_ = other.start_;
            end_ = other.end_;
            unmapped_ = other.unmapped_;

            other.start_ = nullptr;
            other.end_ = nullptr;
            other.unmapped_ = true;

            return *this;
        }

        size_t Size() const
        {
            return std::distance(start_, end);
        }

        class Iterator
        {
            T *ptr_;

        public:
            Iterator(T *const ptr)
                : ptr_(ptr)
            {}

            bool operator!=(const Iterator& other) const
            {
                return ptr_ != other.ptr_;
            }

            Iterator& operator++()
            {
                ++ptr_;
                return *this;
            }

            Iterator operator++(int)
            {
                Iterator tmp{ *this };
                operator++();
                return tmp;
            }

            T& operator*()
            {
                return *ptr_;
            }
            T operator*() const
            {
                return *ptr_;
            }
        };

        class ConstIterator
        {
            T *ptr_;

        public:
            ConstIterator(T *const ptr)
                : ptr_(ptr)
            {}

            bool operator!=(const ConstIterator& other) const
            {
                return ptr_ != other.ptr_;
            }
            ConstIterator& operator++()
            {
                ++ptr_;
                return *this;
            }

            ConstIterator operator++(int)
            {
                ConstIterator tmp{ *this };
                operator++();
                return tmp;
            }

            const T& operator*()
            {
                return *ptr_;
            }
            T operator*() const
            {
                return *ptr_;
            }

        };



        Iterator begin()
        {
            return Iterator(start_);
        }

        ConstIterator begin() const
        {
            return Iterator(start_);
        }

        Iterator end()
        {
            return Iterator(end_);
        }
        ConstIterator end() const
        {
            return ConstIterator(end_);
        }

        void UnMap()
        {
            glUnmapBuffer((GLenum)target_);
            MapStatus::SetMapStatus(target_, (MapAccess)0);
            unmapped_ = true;
            start_ = nullptr;
            end_ = nullptr;
        }

        ~BufferMap()
        {
            if (!unmapped_)
                UnMap();
        }
    };


    class Buffer_base
    {
    protected:
        HandleBuffer handle_;

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

		constexpr FetchedAttrib(GLsizei stride = 0, std::ptrdiff_t offset = 0)
			: stride(stride),
			offset(offset)
		{}

		template <typename E = std::enable_if_t<std::is_same_v<
			variable_traits_type<A>, variable_traits_type<E>>>>
		constexpr FetchedAttrib(FetchedAttrib<E>&& attrib)
			: stride(attrib.stride),
			offset(attrib.offset)
		{}
	};

    // buffer attribute must know its offset (depends on its position via buffer)
    // buffer attribute must provide its stride
	// SubData
	// MapBufferRange
    // Attrib may be compound
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

		constexpr static GLsizei stride = 0;

		constexpr FetchedAttrib<variable_traits_type<A>> Fetch(tag_s<indx>) const
		{
			return offset_;
		}
    };

	template <class tupleAttribs, class =
		decltype(std::make_index_sequence<std::tuple_size_v<tupleAttribs>>())>
		class Buffer_packed;

	template <class ... Attribs, size_t ... indx>
	class Buffer_packed<std::tuple<Attribs...>, std::index_sequence<indx...>> :
		Buffer_attrib<indx, Attribs> ...
	{
		// first offset must be zero!!!
		std::array<std::ptrdiff_t, sizeof...(Attribs)> offsets_{0};

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
			if constexpr (i)
				offsets_[i] = inst * type_s + offsets_[i - 1];
			else
				offsets_[i] = inst * type_s;
		}

	public:
		Buffer_packed(HandleBuffer&& handle = Allocator::Allocate(BufferTarget()))
			: Buffer_base(std::move(handle)),
			Buffer_attrib<indx, Attribs>(offsets_[indx])...
		{}

		void AllocateMemory(convert_to<size_t, Attribs> ... instances)
		{
			size_t total_sz = ((sizeof(Attribs) * instances) + ...);

			(assign<indx>(instances),...);

		}

	};

	template <class ... Attribs>
	using Buffer2 = Buffer_packed<std::tuple<Attribs...>>;


    /*
    This class is used to Set VertexAttributePointer.
    To set properties of a particular vertex attribute, one must fetch
    an instance of VertexAttrib class from a Buffer object, using
    VertexAttrib<A> Buffer::Attribute(tag_s<indx>) or
    VertexAttrib<A> Buffer::Attribute(tag_s<indx>, tag_s<subIndx>) for accessing
    Attributes withing Compounds:
    A - typename of an attribute,
    indx - Attribute's index (index of a Buffer's template parameter),
    subIndx - Attribute's index within the Compound Attribute
    */
    template <class Attrib>
    class VertexAttrib
    {

        std::ptrdiff_t offset_;

        size_t stride_;

        // TODO: stride // stride = sizeof(Atrrib)

    public:

        // TODO: move to private
        constexpr VertexAttrib(std::ptrdiff_t offset, size_t stride = 0)
            : offset_(offset),
            stride_(stride)
        {}

        VertexAttrib(const VertexAttrib<Attrib>&) = delete;
        VertexAttrib& operator=(const VertexAttrib<Attrib>&) = delete;

        constexpr VertexAttrib(VertexAttrib<Attrib>&& other)
            : offset_(other.offset_),
            stride_(other.stride_)
        {}

        VertexAttrib& operator=(VertexAttrib<Attrib>&& other)
        {
            offset_ = other.offset_;
            stride_ = other.stride_;
        }

        std::ptrdiff_t Offset() const
        {
            return offset_;
        }
        size_t Stride() const
        {
            return stride_;
        }

        ~VertexAttrib()
        {}
    private:


    };


    // TODO: add function to return the offset for nth type
    template <typename ... Attr>
    class Buffer : public IBindable<BufferTarget>
    {
        /*
        static_assert(!std::disjunction_v<is_compound_attr<Attr>...>,
            "Only first type may be compound!");*/

        constexpr static size_t num_attribs_ = sizeof...(Attr);

        Handle<BufferTarget> handle_;
        BufferTarget last_bound_ = BufferTarget::none;
        std::array<size_t, num_attribs_> inst_allocated_ =
            init_array<size_t, num_attribs_>()(0);

        // remove this? Another class is responsible for mapping state check
        MapAccess mapped_ = MapAccess(0);

    public:

        Buffer(Handle<BufferTarget>&& handle = Allocator::Allocate(BufferTarget()))
            : handle_(std::move(handle))
        {
            if (!handle_)
                throw std::exception("glt::Buffer::Invalid handle");
        }

        template <BufferTarget target>
        void Bind(tag_v<target> t)
        {
            last_bound_ = target;
            gltActiveBufferTargets::Bind(t, *this);
        }

        // RunTime wrapper
        void Bind(BufferTarget target)
        {
            last_bound_ = target;
            return gltActiveBufferTargets::BindRT(target, *this);
        }

        template <BufferTarget target>
        bool IsBound(tag_v<target>) const
        {
            return gltActiveBufferTargets::IsBound(tag_v<target>(), *this);
        }

        // RunTime wrapper
        bool IsBound(BufferTarget target) const
        {
            return gltActiveBufferTargets::IsBoundRT(target, *this);
        }

        template <BufferTarget target>
        void UnBind(tag_v<target> t)
        {
            last_bound_ = BufferTarget::none;
            gltActiveBufferTargets::UnBind(t, *this);
        }

        // RunTime wrapper
        void UnBind(BufferTarget target)
        {
            last_bound_ = BufferTarget::none;
            return gltActiveBufferTargets::UnBindRT(target, *this);
        }

        const Handle<BufferTarget>& GetHandle() const
        {
            return handle_;
        }

        operator const Handle<BufferTarget>&() const
        {
            return GetHandle();
        }

        BufferTarget LastBound() const
        {
            return last_bound_;
        }



        /*
        What to choose? glBufferData or glBufferStorage?
        BufferStorage size is immutable and memory cannot be reallocated
        - In case of glBufferData was used, would buffer resizing lead to invalidating
        the data contained withing? --> Yes
        - In case of glBufferStorage is used, it is safer to restrict access to AllocateMemory
        function and call it inside the constructor.
        - For glBufferStorage does it make sence to make a size-aware class similar to
        std::array?
        */
        // TODO: case with named attributes
        void AllocateMemory(/* convert_to<size_t, First> inst0,*/ convert_to<size_t, Attr> ... instN, BufferUse usage)
        {
            ValidateBind();

            size_t total_sz = ((sizeof(Attr) * instN) + ...);// sizeof(First) * inst0; //size_of_N<First>(inst0);
            // if constexpr (sizeof...(Attr))
            //	total_sz += ((sizeof(Attr) * instN) + ...); //(size_of_N<Attr>(instN) + ...);

            glBufferData((GLenum)last_bound_, total_sz, nullptr, (GLenum)usage);
            // TODO: check for OpenGL errors
            modify_array<size_t, num_attribs_>::modify(inst_allocated_, /*inst0,*/ instN...);
        }

        size_t InstancesAllocated(size_t indx) const
        {
            if (indx >= num_attribs_)
                throw std::out_of_range("Buffer's array index is out of range!");
            return inst_allocated_[indx];
        }

        // default case for Buffer with one template argument
        template <size_t indx = 0,
            typename UT = nth_element_t<indx, /*First,*/ Attr...>>
            void BufferData(UT* data, size_t size, size_t offset = 0)
        {
            static_assert(is_equivalent_v<nth_element_t<indx, /*First,*/ Attr...>, UT>,
                "Input typename is not equivalent to the buffer's one!");
            size_t total = size + offset;
            if (total > inst_allocated_[indx])
                throw std::range_error("Allocated range exceeded");

            // TODO: choose Named or default
            ValidateBind();
            using DataType = UT;

			GLintptr offset_bytes = GetOffset(tag_s<indx>()) + 
				sizeof(UT) * offset,
				size_bytes = sizeof(UT) * size;

            glBufferSubData((GLenum)last_bound_, offset_bytes, size_bytes, data);

        }




        /*
        BufferMap:
        - when using glMapBuffer, the whole buffer is mapped.
        But in case the Buffer is batched (contains several arrays of different types),
        a single pointer can't be used. User must be able to gain access to the
        starting pointer of each array within the buffer (or its mapped range).
        0. How to ensure that the particular array's range has not been exceeded?
        - functions that take data or expose pointers need to take size arguments
        and check against range
        - do not expose pointers at all?
        1. What gl function to use when mapping buffer or its range?
        - glMapBuffer always?
        - glMapBuffer when accessing the whole buffer and glMapBufferRange for a part?
        - Have 2 different functions for glMapBuffer and glMapBufferRange?
        2. Who is responsible for exposing mapped pointers or recieving input data?
        - BufferMap handles both data input and unmapping
        - BufferMap serves as guard for unmapping data, Buffer handles data input
        3. Make a separated function that expose pointers to be held during subsequent
        drawing operations (glMapBufferRange with persistent bit)?
        */

        // TODO: take user-defined type and check for equivalence?
        // indx = 0 is default for a Buffer specialization with 1 attribute 
        template <size_t indx = 0, MapAccess S = MapAccess::read_write,
            typename UT = nth_element_t<indx, /*First,*/ Attr...>>
            BufferMap<UT, S> MapBuffer(tag_t<UT> = tag_t<UT>())
        {
            static_assert(is_equivalent_v<nth_element_t<indx, /*First,*/ Attr...>, UT>,
                "User-defined type is not equivalent to the buffer's one!");

            // TODO: choose Named or default function
            ValidateBind();

            if (MapStatus::IsMapped(last_bound_))
                throw std::exception("Target is already mapped!");

            // TODO: change to glMapBufferRange?
            UT* start = (UT*)glMapBuffer((GLenum)last_bound_, (GLenum)S);
            if (!start)
                throw std::exception("glMapBuffer returned nullptr!");

            UT* end = std::next(start, inst_allocated_[indx]);

            MapStatus::SetMapStatus(last_bound_, S);

            return BufferMap<UT, S>(start, end, last_bound_);
        }


        // TODO: refactor assertions. Ambiguous errors in wrong order
        template <size_t indx, size_t indx_compound = 0>
        std::ptrdiff_t GetOffset(tag_s<indx>, tag_s<indx_compound> = tag_s<indx_compound>()) const
        {
            using AttrType = nth_element_t<indx, /*First,*/ Attr...>;
            static_assert(indx < num_attribs_, "Index is out of range!");
            constexpr size_t type_sizes[sizeof...(Attr) + 1]{ /*sizeof(First),*/ sizeof(Attr)... };

            std::ptrdiff_t res = 0;

            for (size_t i = 0; i != indx; ++i)
                res += type_sizes[i] * inst_allocated_[i];

            if constexpr (indx_compound)
            {
                static_assert(is_compound_attr_v<AttrType>,
                    "Non-compound elements do not support sub-indexes");
                static_assert(indx_compound < compound_attr_count<AttrType>(),
                    "Sub-index is out of range!");
                res += get_tuple_member_offset<indx_compound, AttrType>();
            }
            return res;
        }

        /* Fetch Vertex Attribute (Simple, Named or Compound) by index. */
        template <size_t indx, class A =
            std::conditional_t<(indx < num_attribs_), nth_element_t<indx, Attr...>, void>>
            VertexAttrib<A> Attribute(tag_s<indx>) const
        {
            static_assert(indx < num_attribs_, "Index is out of range!");

            if constexpr (is_compound_attr_v<A>)
                return VertexAttrib<A>(GetOffset(tag_s<indx>()), sizeof(A));
            else
                return VertexAttrib<A>(GetOffset(tag_s<indx>()));
        }

		// TODO: remove tag_s with tag taking 2 indices
        /* Fetch a subIndex's Attribute from a compound attribute at index */
        template <size_t indx, size_t subIndx, class Compound =
            std::conditional_t<(indx < num_attribs_), nth_element_t<indx, Attr...>, void>,
            class A = std::conditional_t<std::is_void_v<Compound>, void, nth_parameter_t<subIndx, Compound>>>
            VertexAttrib<A> Attribute(tag_indx<indx, subIndx>) const
        {
            static_assert(indx < num_attribs_, "Index is out of range!");

            return VertexAttrib<A>(GetOffset(tag_s<indx>(), tag_s<subIndx>()), sizeof(Compound));
        }

        ~Buffer()
        {
            // check if is still bound
            if ((bool)last_bound_ && IsBound(last_bound_))
                UnBind(last_bound_);
        }

    private:
        void ValidateBind() const
        {
            if (!(bool)last_bound_ || !IsBound(last_bound_))
                throw std::exception("Allocating memory for non-active buffer!");
        }
    };

    template <class AttrList>
    struct BufferListWrapper
    {
        static_assert(is_tuple_v<AttrList>, "Template argument is not an Attribute List");
    };

    template <class ... Attrs>
    struct BufferListWrapper<AttrList<Attrs...>>
    {
        using type = Buffer<Attrs...>;
    };

    template <class AttrList>
    using BufferL = typename BufferListWrapper<AttrList>::type;
    

	
}