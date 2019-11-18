#pragma once

#include "gltEnums.hpp"

#include <map>

// constructs handles
template <typename>
struct gltAllocator;

template <typename eTargetType>
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

	// handle is not aware if it is or can be bound to a target
    ~gltHandle()
    {
        if (handle_)
        {
            // TODO: do i need to check?
            assert(*ppDeleteFunc && "Pointers to OpenGL deleter functions have not been initialiized!");

			// TODO: Unbind from gl_bound_handle, OpenGL ubinds automaticallu after deleting

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

using gltBufferHandle = gltHandle<gltBufferTarget>;

// TODO: add responsibility to delete handle?
// TODO: unbind handle when deleting?
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

// base class for every object, that has glBind(target, handle) function
template <typename eTargetType>
struct IBindable
{
	virtual const gltHandle<eTargetType>& GetHandle() const = 0;
};

template <auto>
struct tag
{};

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
class gl_bound_handle_base
{
protected:
    inline static GLuint raw_handle_ = 0;

    static_assert(has_func_bind_v<eTargetType>, "Typename doesn't have a glBind function");

    // property cannot be retrieved using glGet(..._BINDING)
    static_assert(has_gl_binding_v<target>, "Target can not be bound");
public:

    constexpr static auto ppBindFunc = pp_gl_binder_v<eTargetType>;
    constexpr static auto binding = get_binding_v<target>;

    static void Bind(tag<target>, const IBindable<eTargetType>& obj)
    {
		const gltHandle<eTargetType>& handle = obj.GetHandle();
        assert(*ppBindFunc && "OpenGL Bind function has not been initialized!");

        (*ppBindFunc)((GLenum)target, handle_accessor<eTargetType>()(handle));
        raw_handle_ = handle_accessor<eTargetType>()(handle);
    }

    static void UnBind(tag<target>, const IBindable<eTargetType>& obj)
    {
		const gltHandle<eTargetType>& handle = obj.GetHandle();
		if (handle != raw_handle_)
		{
			assert(false && "Unbinding handle that is not bound!");
			throw("Unbinding handle that is not bound!");
		}
        assert(*ppBindFunc && "OpenGL Bind function has not been initialized!");

        (*ppBindFunc)((GLenum)target, 0);
        raw_handle_ = 0;
    }

    static bool IsBound(tag<target>, const IBindable<eTargetType>& obj)
    {
		const gltHandle<eTargetType>& handle = obj.GetHandle();
        // TODO: assert that returned by glGet object is equal to raw_handle_;
        // TODO: make an independent wrapper
        GLint res = 0;
        glGetIntegerv((GLenum)binding, &res);
        assert(res == raw_handle_);

        return handle == raw_handle_;
    }
};




template <typename ... gl_cur_obj>
struct gl_bound_handle_collection : gl_cur_obj...
{
    using gl_cur_obj::Bind...;
    using gl_cur_obj::UnBind...;
    using gl_cur_obj::IsBound...;

};

template <typename>
class gl_bound_handle;

// Target enum collection template parameter as integer sequence
template <typename eTargetType, eTargetType ... vals>
class gl_bound_handle<std::integer_sequence<eTargetType, vals...>> :
    public gl_bound_handle_collection<gl_bound_handle_base<eTargetType, vals> ...>
{
	using this_t = gl_bound_handle<std::integer_sequence<eTargetType, vals...>>;

	using pBindFunc = void(*)(tag<(eTargetType)0>, const IBindable<eTargetType>&);
	using pIsBoundFunc = bool(*)(tag<(eTargetType)0>, const IBindable<eTargetType>&);
	using pUnbindFunc = void(*)(tag<(eTargetType)0>, const IBindable<eTargetType>&);

	inline static std::map<eTargetType, void(*)()> pBindFuncsMap_
	{
		std::pair(vals,
		reinterpret_cast<void(*)()>(&gl_bound_handle_base<eTargetType, vals>::Bind)) ...
	},
		pUnbindFuncsMap_
	{
		std::pair(vals,
		reinterpret_cast<void(*)()>(&gl_bound_handle_base<eTargetType, vals>::UnBind)) ...
	},
		pIsBoundFuncsMap_
	{
		std::pair(vals,
		reinterpret_cast<void(*)()>(&gl_bound_handle_base<eTargetType, vals>::IsBound)) ...
	};

public:
	// TODO: add runtime versions 
	static void BindRT(eTargetType target, const IBindable<eTargetType>& handle)
	{
		auto found = pBindFuncsMap_.find(target);

		assert(found != pBindFuncsMap_.cend() &&
			"Target is not registered!");

		pBindFunc pBind = reinterpret_cast<pBindFunc>(found->second);
		(pBind)(tag<(eTargetType)0>(), handle);

	}

	static bool IsBoundRT(eTargetType target, const IBindable<eTargetType>& handle)
	{
		auto found = pIsBoundFuncsMap_.find(target);

		assert(found != pIsBoundFuncsMap_.cend() &&
			"Target is not registered!");

		pIsBoundFunc pIsBound = reinterpret_cast<pIsBoundFunc>(found->second);
		return (pIsBound)(tag<(eTargetType)0>(), handle);
	}

	static void UnBindRT(eTargetType target, const IBindable<eTargetType>& handle)
	{
		auto found = pUnbindFuncsMap_.find(target);

		assert(found != pUnbindFuncsMap_.cend() &&
			"Target is not registered!");

		pUnbindFunc pUnBind = reinterpret_cast<pUnbindFunc>(found->second);
		(pUnBind)(tag<(eTargetType)0>(), handle);
	}

};

// forcing generation
// template class gl_bound_handle<glBufferTargetList>; // will throw

using gltActiveBufferTargets = gl_bound_handle<glBufferTargetList>;





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
////////////////////////////////////////////////////

template <class>
class gltMapStatus_;

// this can be acquired via OpenGL function
template <gltBufferTarget ... targets>
class gltMapStatus_<std::integer_sequence<gltBufferTarget, targets...>>
{
	inline static std::map<gltBufferTarget, gltBufferMapStatus>
		mapStatus_{ std::pair(targets, (gltBufferMapStatus)0) ... };

public:

	static gltBufferMapStatus MappedStatus(gltBufferTarget target)
	{
		auto found = mapStatus_.find(target);
		assert(found != mapStatus_.cend() && "Target is not registered!");
		// or use glGet ?

		return found->second;
	}

	static bool IsMapped(gltBufferTarget target)
	{
		return (bool)MappedStatus(target);
	}

	static void SetMapStatus(gltBufferTarget target, gltBufferMapStatus type)
	{
		auto found = mapStatus_.find(target);
		assert(found != mapStatus_.cend() && "Target is not registered!");
		found->second = type;
	}
};

using gltMapStatus = gltMapStatus_<glBufferTargetList>;

// TODO: unite with gltBuffer and replace with gltMapGuard?
// or use the same collection of template attribute params:
// 
template <typename T, gltBufferMapStatus S = gltBufferMapStatus::read_write>
class gltBufferMap
{
	//const IBindable<gltBufferTarget> *handle_;
	gltBufferTarget target_;
	T *start_,
		*end_;
	bool unmapped_ = false;

public:
	gltBufferMap(T* start,
		T* end,
		gltBufferTarget target)
		: start_(start),
		end_(end),
		target_(target)
	{
		// maybe this checks should do gltBuffer class? 
		// gltBuffer also has to check for mapping when allocating memory or buffering data!
		//if (!gl_bound_handle<glBufferTargetList>::IsBoundRT(target_, handle_))
			//throw("Attempting to Map a buffer which has not been bound!");

		assert(end_ > start_ && "Invalid range!");
	}

	gltBufferMap(const gltBufferMap<T, S>&) = delete;
	gltBufferMap& operator=(const gltBufferMap<T, S>&) = delete;

	template <gltBufferMapStatus S2>
	gltBufferMap(gltBufferMap<T, S2>&& other)
		: target_(other.target_),
		start_(other.start_),
		end_(other.end_)
	{
		other.start_ = nullptr;
		other.end_ = nullptr;
		other.unmapped_ = true;
	}

	gltBufferMap& operator=(gltBufferMap<T, S>&& other)
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
		gltMapStatus::SetMapStatus(target_, (gltBufferMapStatus)0);
		unmapped_ = true;
		start_ = nullptr;
		end_ = nullptr;
	}

	~gltBufferMap()
	{
		if (!unmapped_)
			UnMap();
	}
};

template <typename T, typename ... N>
class gltBuffer : public IBindable<gltBufferTarget>
{
	static_assert(!std::disjunction_v<is_compound_attr<N>...>, 
		"Only first type may be compound!");

	constexpr static size_t n_arrays_ = sizeof...(N) + 1;

	gltHandle<gltBufferTarget> handle_;
	gltBufferTarget last_bound_ = gltBufferTarget::none;
	std::array<size_t, n_arrays_> inst_allocated_ =
		init_array<size_t, n_arrays_>()(0);

	gltBufferMapStatus mapped_ = gltBufferMapStatus(0);


	// helper methods
	// get total size of N elements of type C
	template <typename C>
	constexpr static size_t size_of_N(size_t Num)
	{
		return sizeof(C) * Num;
	}

	template <size_t n>
	constexpr static auto* NthType_()
	{
		using rawT = std::tuple_element_t<n, std::tuple<T, N...>>;

		// check here for specialized types (for instance, compound and named)

		return (rawT*)nullptr;
	}

	template <size_t N>
	using NthType_t = std::remove_pointer_t<decltype(NthType_<N>())>;


public:
	gltBuffer(gltHandle<gltBufferTarget>&& handle = gltAllocator<gltBufferTarget>::Allocate())
		: handle_(std::move(handle))
	{}

	template <gltBufferTarget target>
	void Bind(tag<target> t)
	{
		last_bound_ = target;
		gltActiveBufferTargets::Bind(t, *this);
	}

	// RunTime wrapper
	void Bind(gltBufferTarget target)
	{
		last_bound_ = target;
		return gltActiveBufferTargets::BindRT(target, *this);
	}

	template <gltBufferTarget target>
	bool IsBound(tag<target>) const
	{
		return gltActiveBufferTargets::IsBound(tag<target>(), *this);
	}

	// RunTime wrapper
	bool IsBound(gltBufferTarget target) const
	{
		return gltActiveBufferTargets::IsBoundRT(target, *this);
	}

	template <gltBufferTarget target>
	void UnBind(tag<target> t)
	{
		last_bound_ = gltBufferTarget::none;
		gltActiveBufferTargets::UnBind(t, *this);
	}

	// RunTime wrapper
	void UnBind(gltBufferTarget target)
	{
		last_bound_ = gltBufferTarget::none;
		return gltActiveBufferTargets::UnBindRT(target, *this);
	}

	const gltHandle<gltBufferTarget>& GetHandle() const
	{
		return handle_;
	}

	operator const gltHandle<gltBufferTarget>&() const
	{
		return GetHandle();
	}

	gltBufferTarget LastBound() const
	{
		return last_bound_;
	}

	// TODO: case with named attributes
	void AllocateMemory(to_type<size_t, T> inst0, to_type<size_t, N> ... instN, glBufUse usage)
	{
		ValidateBind();

		size_t total_sz = sizeof(T) * inst0; //size_of_N<T>(inst0);
		if constexpr (sizeof...(N))
			total_sz += ((sizeof(N) * instN) + ...); //(size_of_N<N>(instN) + ...);

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

	// default case for Buffer with one template argument
	template <size_t indx = 0, typename T = typename NthType_t<indx>>
	void BufferData(T* data, size_t size, size_t offset = 0)
	{
		size_t total = size + offset;
		if (total > inst_allocated_[indx])
			throw std::range_error("Allocated range exceeded");


		// TODO: choose Named or default
		ValidateBind();
		using DataType = T;
		GLintptr offset_bytes = sizeof(DataType) * offset,
			size_bytes = sizeof(DataType) * size;
		glBufferSubData((GLenum)last_bound_, offset_bytes, size_bytes, data);

	}

	~gltBuffer()
	{
		// check if is still bound
		if ((bool)last_bound_ && IsBound(last_bound_))
			UnBind(last_bound_);
	}

	// TODO: change read_write to Access
	// N = 0 is default for a Buffer specialization with 1 attribute 
	template <size_t N = 0, gltBufferMapStatus S = gltBufferMapStatus::read_write,
		typename T = typename NthType_t<N>>
	gltBufferMap<T, S> MapBuffer()
	{
		// TODO: choose Named or default function
		ValidateBind();

		if (gltMapStatus::IsMapped(last_bound_))
			throw("Target is already mapped!");

		// TODO: change to glMapBufferRange?
		T* start = (T*)glMapBuffer((GLenum)last_bound_, (GLenum)S);
		if (!start)
			throw("glMapBuffer returned nullptr!");

		T* end = std::next(start, inst_allocated_[N]);

		gltMapStatus::SetMapStatus(last_bound_, S);

		return gltBufferMap<T, S>(start, end, last_bound_);
	}

private:
	void ValidateBind() const
	{
		if (!(bool)last_bound_ || !IsBound(last_bound_))
		{
			assert(false && "Allocating memory for non-active buffer!");
			throw("Allocating memory for non-active buffer!");
		}
	}
};


