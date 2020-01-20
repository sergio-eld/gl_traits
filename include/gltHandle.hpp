#pragma once

#include "glslt_traits.hpp"
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

		void DestroyHandle()
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
				DestroyHandle();
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

   
}