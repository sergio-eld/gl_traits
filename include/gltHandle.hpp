#pragma once

#include "gltEnums.hpp"

template <typename>//auto target, typename = decltype(target)>
struct gltAllocator;

template <typename eTargetType>//auto target>
struct handle_accessor;

template <typename eTargetType>
class gltHandle//<target, typename decltype(target)>
{
    //TODO: add Glsync handle type
    GLuint handle_;// = 0;

    friend struct gltAllocator<eTargetType>;
    friend struct handle_accessor<eTargetType>;

public:

    // recover pointer to deleter function pointer
    constexpr static auto ppDeleteFunc = pp_gl_deleter_v<eTargetType>;

    gltHandle(const gltHandle<eTargetType>& other) = delete;
    gltHandle<eTargetType>& operator=(const gltHandle<eTargetType>& other) = delete;

    gltHandle(gltHandle<eTargetType>&& other)
        : handle_(other.handle_)
    {
        other.handle_ = 0;
    }
    gltHandle<eTargetType>& operator=(gltHandle<eTargetType>&& other)
    {
        if (handle_)
            DestroyHandle();
        handle_ = other.handle_;
        other.handle_ = 0;
        return *this;
    }

    bool operator==(GLuint raw_handle) const
    {
        return handle_ == raw_handle;
    }

    bool operator!=(GLuint raw_handle) const
    {
        return !operator==(raw_handle);
    }

    bool operator==(const gltHandle<eTargetType>& other) const
    {
        return handle_ == other.handle_;
    }

    bool operator!=(const gltHandle<eTargetType>& other) const
    {
        return !operator==(other);
    }

    bool IsValid() const
    {
        return (bool)handle_;
    }

    bool operator!() const
    {
        return !IsValid();
    }


    ~gltHandle()
    {
        if (handle_)
        {
            // TODO: do i need to check?
            assert(*ppDeleteFunc && "Pointers to OpenGL deleter functions have not been initialiized!");

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

    gltHandle() = default;
    gltHandle(GLuint handle)
        : handle_(handle)
    {
        //ownership?
        //handle = 0;
    }

    constexpr operator GLuint() const
    {
        return handle_;
    }

};

template <typename eTargetType>
struct gltAllocator
{
    // retrieve pointer to allocator function pointer
    constexpr static auto ppAllocFunc = pp_gl_allocator<eTargetType>::value;

    //template <typename = std::enable_if_t<!std::is_same_v<glShaderTarget, decltype(target)>>>
    static gltHandle<eTargetType> Allocate()
    {
        // TODO: also check for deleter function pointers to be loaded
        assert(*ppAllocFunc && "Pointers to OpenGL allocator functions have not been initialiized!");
        if (!*ppAllocFunc)
            throw("Pointers to OpenGL allocator functions have not been initialiized!");

        GLuint h = 0;
        if constexpr (std::is_same_v<void(APIENTRYP *)(GLsizei, GLuint*), pp_gl_allocator<eTargetType>::value_type>)
        {
            (*ppAllocFunc)(1, &h);
        }
        else if constexpr (std::is_same_v<GLuint(APIENTRYP *)(void), pp_gl_allocator<eTargetType>::value_type>)
            h = (*ppAllocFunc)();
        //else if constexpr (std::is_same_v<GLuint(APIENTRYP *)(GLenum), pp_gl_allocator<eTargetType>::value_type>)
        //    return gltHandle<eTargetType>((*ppAllocFunc)((GLenum)target));
        else
            static_assert(false, "Unhandled case!");   

        if (!h)
        {
            assert(false && "Failed to allocate handle!");
            throw("Failed to allocate handle");
        }
        return gltHandle<eTargetType>(h);

    }

    //template <typename = std::enable_if_t<!std::is_same_v<glShaderTarget, decltype(target)>>>
    gltHandle<eTargetType> operator()() const
    {
        return Allocate();
    }

};

template <>
struct gltAllocator<glShaderTarget>
{
    constexpr static auto ppAllocFunc = pp_gl_allocator_v<glShaderTarget>;

    static gltHandle<glShaderTarget> Allocate(glShaderTarget target)
    {
        // TODO: also check for deleter function pointers to be loaded
        assert(*ppAllocFunc && "Pointers to OpenGL allocator functions have not been initialiized!");
        if (!*ppAllocFunc)
            throw("Pointers to OpenGL allocator functions have not been initialiized!");

        return gltHandle<glShaderTarget>((*ppAllocFunc)((GLenum)target));
    }

    gltHandle<glShaderTarget> operator()(glShaderTarget target) const
    {
        return Allocate(target);
    }

};

template <typename eTargetType>
struct handle_accessor
{
    GLuint operator()(const gltHandle<eTargetType>& handle) const
    {
        return handle;
    }
};

template <auto>
struct tag
{};

template <typename eTargetType, eTargetType target>
class gl_current_object_base
{
    static GLuint raw_handle_;

    static_assert(has_func_bind_v<eTargetType>, "Typename doesn't have a glBind function");

    // property cannot be retrieved using glGet(..._BINDING)
    static_assert(has_gl_binding_v<target>, "Target can not be bound");
public:

    constexpr static auto ppBindFunc = pp_gl_binder_v<eTargetType>;
    constexpr static auto binding = get_binding_v<target>;

    static void Bind(tag<target>, const gltHandle<eTargetType>& handle)
    {
        assert(*ppBindFunc && "OpenGL Bind function has not been initialized!");

        (*ppBindFunc)((GLenum)target, handle_accessor<eTargetType>()(handle));
        raw_handle_ = handle_accessor<eTargetType>()(handle);
    }

    static void UnBind(tag<target>)
    {
        assert(*ppBindFunc && "OpenGL Bind function has not been initialized!");

        (*ppBindFunc)((GLenum)target, handle_accessor<eTargetType>()(handle));
        raw_handle_ = handle_accessor<eTargetType>()(handle);
    }

    static bool IsCurrent(tag<target>, const gltHandle<eTargetType>& handle)
    {
        // TODO: assert that returned by glGet object is equal to raw_handle_;
        // TODO: make an independent wrapper
        GLint res = 0;
        glGetIntegerv((GLenum)binding, &res);
        assert(res == raw_handle_);

        return handle == raw_handle_;
    }
};

template <typename eTargetType, eTargetType target>
GLuint gl_current_object_base<eTargetType, target>::raw_handle_ = 0;

template <typename ... gl_cur_obj>
struct gl_current_object_collection : gl_cur_obj...
{
    using gl_cur_obj::Bind...;
    using gl_cur_obj::UnBind...;
    using gl_cur_obj::IsCurrent...;

};

template <typename>
class gl_current_object;

template <typename eTargetType, eTargetType ... vals>
class gl_current_object<std::integer_sequence<eTargetType, vals...>> :
    public gl_current_object_collection<gl_current_object_base<eTargetType, vals> ...>
{
	using this_t = gl_current_object<std::integer_sequence<eTargetType, vals...>>;

	template <size_t n>
	constexpr static eTargetType arg_n = std::tuple_element_t<n, std::tuple<std::integral_constant<eTargetType, vals>...>>::value;

	template <size_t iter = 0>
	static void bind_or_next(eTargetType target, const gltHandle<eTargetType>& handle)
	{
		if constexpr (iter != sizeof...(vals))
		{
			constexpr eTargetType cur_target = arg_n<iter>;
			if (target == cur_target)
				return Bind(tag<cur_target>(), handle);
			return bind_or_next<iter + 1>(target, handle);
		}

		assert(false && "Invalid input! Unrecognized target");
		throw("Invalid input! Unrecognized target");
	}

	template <size_t iter = 0>
	static bool iscurrent_or_next(eTargetType target, const gltHandle<eTargetType>& handle)
	{
		if constexpr (iter != sizeof...(vals))
		{
			constexpr eTargetType cur_target = arg_n<iter>;
			if (target == cur_target)
				return IsCurrent(tag<cur_target>(), handle);
			return iscurrent_or_next<iter + 1>(target, handle);
		}

		assert(false && "Invalid input! Unrecognized target");
		throw("Invalid input! Unrecognized target");
	}

	template <size_t iter = 0>
	static void unbind_or_next(eTargetType target)
	{
		if constexpr (iter != sizeof...(vals))
		{
			constexpr eTargetType cur_target = arg_n<iter>;
			if (target == cur_target)
				return UnBind(tag<cur_target>());
			return unbind_or_next<iter + 1>(target);
		}

		assert(false && "Invalid input! Unrecognized target");
		throw("Invalid input! Unrecognized target");
	}

public:
	// TODO: add runtime versions 
	static void BindRT(eTargetType target, const gltHandle<eTargetType>& handle)
	{
		return bind_or_next(target, handle);
	}

	static bool IsCurrentRT(eTargetType target, const gltHandle<eTargetType>& handle)
	{
		return iscurrent_or_next(target, handle);
	}

	static void UnBindRT(eTargetType target)
	{
		return unbind_or_next(target);
	}

};

using gltActiveBufferTargets = gl_current_object<glBufferTargetList>;

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
        - for named may use tag dispatching

Cases:
- VBO of unnamed attributes
    glVBO<glm::Vec3, glm::Vec3, int, int>

- VBO with first compound maybe followed by several unnamed attributes
    glVBO<comp_attr<glm::Vec3, glm::Vec2>, int, int>

- VBO with named attributes
    glVBO<glslt<glm::Vec3, gl_pos>, glslt<glm::Vec2, gl_tex>> (gl_pos = "pos", gl_tex = "tex")
    glVBO<comp_attr<glslt<glm::Vec3, gl_pos>, glslt<glm::Vec2, gl_tex>>>

TODO: define all attribute validations here (at the beginning)
*/


// attribute traits
template <class T, const char * glslName>
struct glslt
{
    using type = typename T;
    constexpr static const char * name = glslName;
};

template <class ... vAttribs>
using comp_attr = std::tuple<vAttribs...>;

template <class T>
struct is_named_attr : std::bool_constant<false> {};

template <class T, const char * glslName>
struct is_named_attr<glslt<T, glslName>> : std::bool_constant<true> {};

template <class T>
constexpr inline bool is_named_attr_v = is_named_attr<T>();

template <class T>
struct is_compound_attr : std::bool_constant<false> {};

template <class ... T>
struct is_compound_attr<comp_attr<T...>> : std::bool_constant<(sizeof...(T) > 1)> {};

// matrices are compound
template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct is_compound_attr<glm::mat<C, R, T, Q>> : std::bool_constant<true> {};

template <class T>
constexpr inline bool is_compound_attr_v = is_compound_attr<T>();

////////////////////////////////////////////////////
// helpers
////////////////////////////////////////////////////

template <typename To, typename From>
using to_type = To;

template <typename To, size_t indx>
using type_from_indx = To;

template <typename T, size_t sz, class = decltype(std::make_index_sequence<sz>())>
struct gen_tuple;

template <typename T, size_t sz, size_t ... indx>
struct gen_tuple<T, sz, std::index_sequence<indx...>>
{
	using type = std::tuple<type_from_indx<T, indx> ...>;
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
		return std::array<T, sz>{ type_from_indx<T, Indx>(def_value) ... };
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

	constexpr static void modify(std::array<T, sz>& arr, const type_from_indx<T, indx>& ... vals)
	{
		(set<indx>(arr, vals), ...);
	}
};


////////////////////////////////////////////////////

template <typename T, typename ... N>
class gltBuffer
{

	constexpr static size_t n_arrays_ = sizeof...(N) + 1;

	gltHandle<glBufferTarget> handle_;
	glBufferTarget last_bound_ = glBufferTarget::none;
	std::array<size_t, n_arrays_> inst_allocated_ =
		init_array<size_t, n_arrays_>()(0);

	void validate()
	{
		if constexpr (sizeof...(N))
			static_assert(!std::disjunction_v<is_compound_attr<N>...>, "Only first type may be compound!");
	}

	template <typename C>
	constexpr static size_t size_of_N(size_t Num)
	{
		return sizeof(C) * Num;
	}

public:
	gltBuffer(gltHandle<glBufferTarget>&& handle = gltAllocator<glBufferTarget>::Allocate())
		: handle_(std::move(handle))
	{
		validate();
	}

	template <glBufferTarget target>
	void Bind(tag<target> t)
	{
		last_bound_ = target;
		gltActiveBufferTargets::Bind(t, *this);
	}

	// RunTime wrapper
	void Bind(glBufferTarget target)
	{
		last_bound_ = target;
		return gltActiveBufferTargets::BindRT(target, *this);
	}

	template <glBufferTarget target>
	bool IsCurrent(tag<target>) const
	{
		return gltActiveBufferTargets::IsCurrent(tag<target>(), *this);
	}

	bool IsCurrent(glBufferTarget target) const
	{
		return gltActiveBufferTargets::IsCurrentRT(target, *this);
	}

	template <glBufferTarget target>
	void UnBind(tag<target> t)
	{
		last_bound_ = glBufferTarget::none;
		gltActiveBufferTargets::UnBind(t, *this);
	}

	// RunTime wrapper
	void UnBind(glBufferTarget target)
	{
		last_bound_ = glBufferTarget::none;
		return gltActiveBufferTargets::UnBindRT(target, *this);
	}

	operator const gltHandle<glBufferTarget>&() const
	{
		return handle_;
	}

	glBufferTarget LastBound() const
	{
		return last_bound_;
	}

	// TODO: case with named attributes
	void AllocateMemory(to_type<size_t, T> inst0, to_type<size_t, N> ... instN, glBufUse usage)
	{
		if (!(bool)last_bound_ || !IsCurrent(last_bound_))
		{
			assert(false && "Allocating memory for non-active buffer!");
			throw("Allocating memory for non-active buffer!");
		}

		size_t total_sz = size_of_N<T>(inst0) + (size_of_N<N>(instN) + ...);// sizeof(N) * instN + ...);
		glBufferData((GLenum)last_bound_, total_sz, nullptr, (GLenum)usage);
		// TODO: check for OpenGL errors
		modify_array<size_t, n_arrays_>::modify(inst_allocated_, inst0, instN...);
	}

	size_t InstancesAllocated(size_t indx) const
	{
		if (indx >= n_arrays_)
			throw std::out_of_range("Buffer's array index is out of range!");
 		return inst_allocated_[indx];
	}
};

