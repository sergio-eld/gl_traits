#pragma once

// 
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
1. Provide typesafe info about Vertex Buffer Object:
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
	glVBO<compound<glm::Vec3, glm::Vec2>, int, int>

- VBO with named attributes
	glVBO<glslt<glm::Vec3, gl_pos>, glslt<glm::Vec2, gl_tex>> (gl_pos = "pos", gl_tex = "tex")
	glVBO<compound<glslt<glm::Vec3, gl_pos>, glslt<glm::Vec2, gl_tex>>>

TODO: define all attribute validations here (at the beginning)
*/

#include "gltEnums.hpp"
#include "gltHandle.hpp"

template <class T, const char * glslName>
struct glslt
{
	using type = typename T;
	constexpr static const char * name = glslName;
};

template <class ... vAttribs>
using compound = std::tuple<vAttribs...>;

template <class T>
struct is_named_attr : std::bool_constant<false> {};

template <class T, const char * glslName>
struct is_named_attr<glslt<T, glslName>> : std::bool_constant<true> {};

template <class T>
constexpr inline bool is_named_attr_v = is_named_attr<T>();

template <class T>
struct is_compound_attr : std::bool_constant<false> {};

template <class ... T>
struct is_compound_attr<compound<T...>> : std::bool_constant<true> {};

// matrices are compound
template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct is_compound_attr<glm::mat<C, R, T, Q>> : std::bool_constant<true> {};

template <class T>
constexpr inline bool is_compound_attr_v = is_compound_attr<T>();

///////////////////////////
//What is this for?
///////////////////////////

// this does not consider padding for types less than 4 bytes
template <size_t indx, class tuple, class seq = typename indx_seq<indx>>
struct get_offset;

template <typename ... attribs>
struct get_offset<0, std::tuple<attribs...>, std::index_sequence<>>
{
    constexpr static size_t value = 0;
};

template <class tuple>
struct tuple_size;

template <class ... types>
struct tuple_size<std::tuple<types...>>
{
    constexpr static size_t value = sum<sizeof(types)...>;
};

template <class tuple>
constexpr inline size_t tuple_size_v = tuple_size<tuple>::value;

template <size_t indx, class ... attribs, size_t ... i>
struct get_offset<indx, std::tuple<attribs...>, std::index_sequence<i...>>
{
    constexpr static size_t value = sum<sizeof(nth_type<i, attribs...>) ...>;
};

template <size_t indx, class tuple>
constexpr inline size_t get_offset_v = get_offset<indx, tuple>::value;

///////////////////////////

// TODO: specialization for glm matrix?
template <class vAttrib>
struct attrib_traits
{
	// TODO: check if template parameter is valid  
	//constexpr static bool is_compound = false;
	constexpr static size_t stride = sizeof(vAttrib);
	constexpr static size_t numAttribs = 1;
	using base_attribs = typename std::tuple<vAttrib>;
	using type = typename is_named_attr<vAttrib>::type;
};

template <class ... vAttribs>
struct attrib_traits<compound<vAttribs...>>
{
	//constexpr static bool is_compound = true;
	constexpr static size_t stride = sum<sizeof(vAttribs)...>;
	constexpr static size_t numAttribs = sizeof...(vAttribs);
	using base_attribs = typename std::tuple<vAttribs...>;
	using type = typename void;
};

// TODO:
// class for runtime check the currently active buffer for each target

class GLT_API glt_buffers
{
	template <BufferTarget target>
	static const Handle<target> *currentBuffer_;

    // separate to another class ?
    static const Handle<glVertexArrayTarget::vao> *currentVAO_;

public:

	template <BufferTarget target>
	static Handle<target> GenBuffer()
	{
		GLuint handle = 0;
		glGenBuffers(1, &handle);
		assert(handle && "Failed to generate buffer!");
        return Allocator<target>::Allocate();// Handle<target>(handle);
	}

	template <BufferTarget target>
	static bool IsCurrentBuffer(const Handle<target>& handle)
	{
		return currentBuffer_<target> ? *currentBuffer_<target> == handle : false;
	}

	template <BufferTarget target>
	static void BindBuffer(const Handle<target>& handle)
	{
		glBindBuffer((int)target, handle_accessor<target>()(handle));
		currentBuffer_<target> = &handle;
	}

	template <BufferTarget target>
	static void AllocateBuffer(const Handle<target>& handle, size_t sizeBytes, glBufUse usage)
	{
        if (!IsCurrentBuffer(handle))
        {
            assert(false && "Allocating memory for non-active buffer!");
            throw("Allocating memory for non-active buffer!");
        }
		glBufferData((GLenum)target, sizeBytes, nullptr, (GLenum)usage);
	}


    // raw, type-independent members, doesnt track allocated memory ranges
	template <BufferTarget target, typename dataType>
	static void BufferSubData(const Handle<target>& handle, size_t offset, const std::vector<dataType>& data)
	{
        if (!IsCurrentBuffer(handle))
        {
            assert(false && "Buffering data to non-active buffer!");
            throw("Buffering data to non-active buffer!");
        }
		glBufferSubData((GLenum)target, (GLintptr)offset, sizeof(dataType) * data.size(), data.data());
	}

	//TODO: restrict usage for array_buffer
	template<size_t sz, typename dataType>
	static void BufferData(BufferTarget target, const dataType(&data)[sz], glBufUse usage)
	{
		glBufferData(target, sizeof(data), data, usage);
	}

	template <class dataType>
	static void BufferData(BufferTarget target, const std::vector<dataType>& data, glBufUse usage)
	{
		glBufferData((GLenum)target, data.size() * sizeof(dataType), data.data(), (GLenum)usage);
	}


    //////////////////////////////////////////////////////
    //VAO move to VAO class
    //////////////////////////////////////////////////////

	static Handle<glVertexArrayTarget::vao> GenVAO()
	{
		GLuint vao;
		glGenVertexArrays(1, &vao);
		assert(vao && "Failed to generate VAO");
        return Allocator<glVertexArrayTarget::vao>()();// Handle<glVertexArrayTarget::vao>(vao);
	}

    static void BindVAO(const Handle<glVertexArrayTarget::vao>& handle)
    {
        assert(handle.IsValid() && "Binding invalid VAO");
        glBindVertexArray(handle_accessor<glVertexArrayTarget::vao>()(handle));
        currentVAO_ = &handle;
    }

	static bool IsCurrentVAO(const Handle<glVertexArrayTarget::vao>& handle)
	{
		return currentVAO_ ? *currentVAO_ == handle : false;
	}


	// TODO: add shader prog handle to VAO class
	// TODO: specialization for glm::vec and glm::mat?
	// TODO: specializtion for compound?
	// TODO: check currently bound VBO?
	// use tag dispatching?
	// not compound, do I need stride as parameter?
	// store normalize parameter in a attrib_class (create new)
	template <typename Attrib>
	static void VertexAttribPointer(Attrib&& tag, const Handle<glVertexArrayTarget::vao>& handle,
		size_t indx,
		size_t offsetBytes,
		bool normalize = false)
	{
		bool is_current = IsCurrentVAO(handle);
		assert(is_current && "Setting attribute pointer for non-active VAO");

        /*
		glVertexAttribPointer((GLuint)indx,				// attribute index
			(GLint)glenum_from_type<Attrib>::nComponents,	// attribute's components
			glenum_from_type_v<Attrib>,						// OpenGL type
			normalize,
			0,							// stride. 0 if Attrib is not compound (tightly packed)
			(void*)offsetBytes);
            */
	}

	template <class compound, class = indx_seq<attrib_traits<compound>::numAttribs>>
	struct compound_attrib_pointer_;

	template <class ... Attribs, size_t ... indxs>
	struct compound_attrib_pointer_<compound<Attribs...>, std::index_sequence<indxs...>>
	{
		static void VertexAttribPointer(const Handle<glVertexArrayTarget::vao>& handle,
			//size_t firstIndx,		// compounds starts from 0 by default
			//size_t offsetBytes,	// offset is calculated from compound types
			const std::tuple<to_type<bool, Attribs>...>& normalize)
		{
			constexpr size_t stride = attrib_traits<compound<Attribs...>>::stride;
			using tuple = typename std::tuple<Attribs...>;

            /*
			(glVertexAttribPointer((GLuint)indxs,
				(GLint)glenum_from_type<Attribs>::nComponents,
				glenum_from_type_v<Attribs>,
				std::get<indxs>(normalize),
				stride,
				(void*)get_offset_v<indxs, tuple>), ...);
                */
		}
	};



	// TODO: think of a better way to pass normalized parameters
	template <typename ... Attribs>
	static void VertexAttribPointer(compound<Attribs...>&& tag, const Handle<glVertexArrayTarget::vao>& handle,
		//size_t firstIndx,		// compounds starts from 0 by default
		//size_t offsetBytes,	// offset is calculated from compound types
		const std::tuple<to_type<bool, Attribs>...>& normalize)
	{
		bool is_current = IsCurrentVAO(handle);
		assert(is_current && "Setting attribute pointer for non-active VAO");

		compound_attrib_pointer_<compound<Attribs...>>::VertexAttribPointer(handle,
			//indx, offsetBytes, 
			normalize);
	}

	static void EnableVertexAttribArray(const Handle<glVertexArrayTarget::vao>& handle, size_t indx)
	{
		bool is_current = IsCurrentVAO(handle);
		assert(is_current && "Setting attribute pointer for non-active VAO");

		glEnableVertexAttribArray((GLuint)indx);

	}

};

template <BufferTarget target>
const Handle<target> *glt_buffers::currentBuffer_ = nullptr;


template <BufferTarget, class ... params>
class Buffer;

template <BufferTarget target>
class Buffer<target>
{
	static_assert(target != BufferTarget::array_buffer, "Array buffer target is not allowed, use corresponding gltBuffer specialization");
	static_assert(target != BufferTarget::element_array_buffer, "Element Array buffer target is not allowed, use corresponding gltBuffer specialization");

	//TODO: add support for multiple threads. Use static map or something
	//static const Handle<target> *currentBuffer_;

    Handle<target> handle_;// = glt_buffers::GenBuffer<target>();

public:
	//TODO: move semantics!
    Buffer() = default;

    Buffer(Handle<target>&& handle)
        : handle_(std::move(handle))
    {}

	void Bind()
	{
		glt_buffers::BindBuffer(handle_);
		//currentBuffer_ = &handle_;
	}

	void UnBind()
	{
		assert(IsBound() && "Trying to unbind non-active buffer object!");
		glt_buffers::BindBuffer<target>(0);
		currentBuffer_ = nullptr;
	}

	bool IsBound() const
	{
		if (!handle_)
			return false;
		return glt_buffers::IsCurrentBuffer(handle);
	}

	operator const Handle<target>&() const
	{
		return handle_;
	}

	~Buffer()
	{
		if (IsBound())
			UnBind();
	}
};

//TODO: move to glt_buffers?
//template <glt_buffers::Target target>
//const Handle<target> Buffer<target, void>::*currentBuffer_ = nullptr;


//TODO: move validation to vao_traits?
// to validate a user-defined attribute
template <class attrib, class members_pack = typename refl_traits<attrib>::field_types>
struct vertex_attrib_validate;


template <class attrib, class ... fields>
struct vertex_attrib_validate<attrib, std::tuple<fields...>>
{
	constexpr static bool valid = (sizeof...(fields) <= 4 &&
		sizeof...(fields) &&
		all_same_v<fields...>) ? true : false;

};

template <class ... attribs>
constexpr bool all_valid_v = (vertex_attrib_validate<attribs>::valid && ...);

template <class ... v_attribs>
using vao_attribs = std::tuple<v_attribs...>;

// VBO
// - allocate size
// - write subdata, or all data
// - clear sub or all data

// vertex attribute

/*
Writing subdata based on template parameters. 
*/

template <class ... attribs>
class glVBO;





// not only for vertex buffer I suppose
template <BufferTarget target, class ... attribs>
class vbo_allocator
{
	const Handle<target> *handle_;
	glBufUse usage_;

	std::array<size_t, sizeof...(attribs)> instancesAllocated_;

public:
	vbo_allocator(const Handle<target>* handle)
		: handle_(handle)
	{}

	void AllocateMemory(to_type<size_t, attribs> ... attribInstances, glBufUse usage)
	{
		size_t totalSize = TotalSize(attribInstances...);
		glt_buffers::AllocateBuffer(*handle_, totalSize, usage);
		UpdateAllocatedInfo(attribInstances..., std::make_index_sequence<sizeof...(attribs)>());
		usage_ = usage;
	}

	size_t AllocatedInstances(size_t attribNum) const
	{
		return instancesAllocated_[attribNum];
	}

	glBufUse Usage() const
	{
		return usage_;
	}

private:

	constexpr static size_t TotalSize(to_type<size_t, attribs> ... attribInstances)
	{
		// explicitly get size of each atribute, it can be compound (contain several consequent attribs)
		return ((attrib_traits<attribs>::stride * attribInstances) + ...);
	}

	void SetAllocatedInfo(size_t indxAttrib, size_t instances)
	{
		instancesAllocated_[indxAttrib] = instances;
	}

	template <size_t ... indx>
	void UpdateAllocatedInfo(to_type<size_t, attribs> ... attribInstances, std::index_sequence<indx...>&&)
	{
		(SetAllocatedInfo(indx, attribInstances), ...);
	}

};

template <class cl_attrib>
constexpr bool valid_user_attribclass();

template <template <class ...> class pack, class ... cl_attribs>
constexpr bool valid_compound_user_attribclass()
{
	return (valid_user_attribclass<cl_attribs>() && ...);
}


// TODO: implement typecheck for user-defined types
// validating user-provided data class
template <class cl_attrib>
constexpr bool valid_user_attribclass()
{
	if constexpr (glm_traits<cl_attrib>::is_glm)
		// glm-types are valid by default
		return true;
	else
	{
		using compound_types = typename refl_traits<cl_attrib>::field_types;
		if constexpr (all_glm_v<compound_types>)
			return true;
		else
		{
			// must be pod or trivially constructable
			static_assert(std::is_pod_v<cl_attrib>, "Class for vao attributes must be a POD class");

			//must have not more than 4 components

			// check memory layout, elem % 4 must be = 0 
			//if constexpr ()
			// if constexpr (is compound)
			// return valid_compound_user_attribclass

			return true;
		}
	}
}

template <class attrib_class, class user_defined>
constexpr bool is_equivalent();

template <class pack_attribs, class pack_user>
struct all_equivalent : std::bool_constant<false>
{
};

template <class ... attribs,
	class ... user_attribs>
	struct all_equivalent<compound<attribs...>, std::tuple<user_attribs...>>
	: std::bool_constant<(is_equivalent<attribs, user_attribs>() && ...)>
{
	static_assert(sizeof...(attribs) == sizeof...(user_attribs), "Numbers of attribs provided mismatch");
	static_assert(!std::conjunction_v<is_compound_attr<attribs>...>,
		"Nested compound attributes are not allowed!");
};

template <class pack_attribs, class pack_user>
constexpr inline bool all_equivalent_v = all_equivalent<pack_attribs, pack_user>();

template <class attrib_class, class user_defined>
constexpr bool is_equivalent()
{
	
	// check base level (if not compound or glm-compound (matrix) )
	if constexpr (glm_traits<attrib_class>::is_glm)
	{
		// user_defined is also glm and same = true
		if constexpr (glm_traits<user_defined>::is_glm)
			return std::is_same_v<attrib_class, user_defined>;
		// user_defined is not glm but equivalent = true (not implemented)
		else
			return false;
	}
	else if constexpr (is_compound_attr_v<attrib_class>)
	{
		if constexpr (attrib_traits<attrib_class>::numAttribs != refl_traits<user_defined>::fields_count())
			return false;
		else 
			return all_equivalent_v<attrib_class, refl_traits<user_defined>::field_types>;
	}
	// TODO: 
	// else if constexpr (not glm && not compound)
	else
	{
		static_assert(false, "Unhandled attribute type");
	}
	
}

template <class attrib_class, class user_defined>
constexpr inline bool is_equivalent_v = is_equivalent<attrib_class, user_defined>();

/*
template <class ... attributes>
class valid_attr_collection;

template <>
class valid_attr_collection<>
{
public:
	constexpr static bool value = false,
//		has_compounds = false,
		has_named = false;
	constexpr static size_t num_elements = 0,
		num_compounds = 0;
	constexpr static std::index_sequence<> indexes_compounds = std::index_sequence<>();
	constexpr static std::index_sequence<> invalid_elems = std::index_sequence<>();
};


template <class first, class ... attributes>
class valid_attr_collection<first, attributes...>
{
	//TODO: check if all named attribs are unique, skip compounds

	constexpr static bool GetValue()
	{
		
		if constexpr (!sizeof...(attributes))
		{
			return true;
		}
		else
		{
			//return !dh_disjunction<attrib_traits<attributes>::is_compound...>;
			return true;
		}
	}

public:

	// invalid if any of 
	constexpr static bool value = GetValue(),
//		has_compounds = dh_disjunction<attrib_traits<first>::is_compound,
//		attrib_traits<attributes>::is_compound...>,
		has_named = false;// dh_disjunction < attrib_traits<first>::

	constexpr static size_t num_elements = 1 + sizeof...(attributes),

		// TODO: implement
		num_compounds = 0;
	constexpr static std::index_sequence<> indexes_compounds = std::index_sequence<>();
	constexpr static std::index_sequence<> invalid_elems = std::index_sequence<>();
};

template <class ... attribs>
constexpr inline bool valid_attr_collection_v = valid_attr_collection<attribs...>::value;
*/

/*
VBO can store only ONE compound instance, and it must be at the beginning,
since you can not specify an offset for the first index, unlike for batched attributes,
where offset is equal to the memory allocated for all previous batched attributes.

This class must represent all the possible buffer types. Must support both named 
and non-named glslt attributes. 
GLM types are considered as valid non-named attributes by default.

TODO: named attributes
TODO: validate attributes' collection
*/
template <class ... attribs>
class glVBO : public vbo_allocator<BufferTarget::array_buffer, attribs...>
{
//	static_assert(valid_attr_collection_v<attribs...>, "Inavlid attributes collection!");
	using vboalloc = vbo_allocator<BufferTarget::array_buffer, attribs...>;

	Handle<BufferTarget::array_buffer> handle_;

public:

//	constexpr static bool has_named_attribs = dh_disjunction<is_named_attr_v<attribs>...>;


	template <size_t attribIndx>
	size_t AttribMemoryTotal() const
	{
		return AllocatedInstances(attribIndx) * sizeof(nth_type<attribIndx, attribs...>);
	}

	// in bytes
	template <size_t attribIndx>
	size_t GetOffset() const
	{
		if constexpr (!attribIndx)
			return 0;
		else
		{
			static_assert(attribIndx < sizeof...(attribs), "Attribute index is out of bounds");
			constexpr size_t prevIndx = attribIndx - 1;
			size_t prevOffset = AttribMemoryTotal<prevIndx>();
			return prevOffset + GetOffset<prevIndx>();
		}
	}


	glVBO(Handle<BufferTarget::array_buffer>&& handle)
		: vboalloc(&handle_),
		handle_(std::move(handle))
	{}

	//using vboalloc::AllocateMemory;
	//using vboalloc::Usage;
	void Bind()
	{
		glt_buffers::BindBuffer(handle_);
	}

	// TODO: add named attributes?
	// TODO: constexpr function to auto detect attribIndex based on type
	// TODO: check for assigned memory range
	template <size_t attribIndx /*= deduce_index<AttribClass>*/, class AttribClass>
	void BufferData(const std::vector<AttribClass>& data, size_t startingElem = 0)
	{
		// check if memory has been allocated prior to call.
		// if not and buffering data of type that VBO solely consists of, allocate memory
		// else throw error
		static_assert(attribIndx < sizeof...(attribs), "Attribute index is out of range!");
		static_assert(valid_user_attribclass<AttribClass>(),
			"Class is not valid as a VAO attribute");

		// check if AttribClass is equivalent to the corresponding vbo attrib
		static_assert(is_equivalent_v<nth_type<attribIndx, attribs...>, AttribClass>,
			"Provided type is not equivalent to attribute type of index attribIndx");

		// size_t offset = sizeof(nth_type<attribIndx, attribs...>) * AllocatedInstances()
		// calculate total offset
		// check boundaries
		bool inBounds = sizeof(AttribClass) * data.size() <= AttribMemoryTotal<attribIndx>();
		assert(inBounds && "Exceeding allocated memory for the attribute");
		glt_buffers::BufferSubData(handle_, GetOffset<attribIndx>(), data);
	}



	//template <class T, const char* name, std::enable_if_t<has_named_attribs> = 0>
	//void BufferData(glslt<T, name>(), )
};

template <class ... attribs>
using glVBOCompound = glVBO<compound<attribs...>>;


// One VBO can continiously store as many attributes as needed. Each attribute can store no more than 4 components
// should we track the active array_buffer and where? in VAO?
template <class ... vaoAttribs>
class Buffer<BufferTarget::array_buffer, vao_attribs<vaoAttribs...>>// : public Buffer_base
{
    Handle<BufferTarget::array_buffer> handle_;// = glt_buffers::GenBuffer<BufferTarget::array_buffer>();

	size_t elemsLoaded_ = 0;

public:
	//assert vaoAttribs to meet gl requirements (1 - 4 elements, etc)
	//this somehow makes the compiler angry!
	//static_assert((vertex_attrib_validate<vaoAttribs>::valid && ...), "Invalid VAO attributes!");
	//static_assert(all_valid_v<vaoAttribs...>, "Invalid VAO attributes!");

    Buffer()
    {}

    Buffer(Handle<BufferTarget::array_buffer>&& handle)
        : handle_(std::move(handle))
    {}

    void Init(Handle<BufferTarget::array_buffer>&& handle)
    {
        handle_ = std::move(handle);
    }

	template <class vertexInfo>
	void BufferData(const std::vector<vertexInfo>& vertexes, glBufUse usage)
	{
		// TODO: compare is_equivalent. glm::vec4 is equivalent to float[4] or std::array<float, 4>

		//static_assert(std::is_same_v<typename refl_traits<vertexInfo, std::tuple<vaoAttribs...>>::field_types,
		//	std::tuple<vaoAttribs...>>, "Attributes mismatch!");
		//assert vertexInfo to have attributes
		//glNamedBufferData(handle_, vertexes.size() * sizeof(vertexInfo), vertexes.data(), usage);     //glNamedBufferData is nullptr!!!!!

		glt_buffers::BufferData(BufferTarget::array_buffer, vertexes, usage);
		elemsLoaded_ = vertexes.size();

	}

	const Handle<BufferTarget::array_buffer>& Handle() const
	{
		return handle_;
	}

	operator const Handle<BufferTarget::array_buffer>&() const
	{
		return handle_;
	}

	size_t ElementsLoaded() const
	{
		return elemsLoaded_;
	}
};

template <class ... vaoAttribs>
using gltVBO = Buffer<BufferTarget::array_buffer, vao_attribs<vaoAttribs...>>;


template <typename>
class vao_enabler;

template <typename ... vaoAttribs>
class vao_enabler<std::tuple<vaoAttribs...>>
{
	// TODO: disable for Release?
	constexpr static size_t totalAttribs_ = sizeof...(vaoAttribs);

public:



	template <size_t nAttrib>
	static void VertexAttribPointer(bool normalized, size_t startingIndex = 0)
	{
		// TODO: runtime check if pointer index out of bounds?

		typedef std::tuple<vaoAttribs...> vaoPack;
		typedef std::tuple_element_t<nAttrib, std::tuple<vaoAttribs...>> curAttrib;
		typedef std::tuple_element_t<0, refl_traits<curAttrib>::field_types> cType;

		size_t attribIndex = nAttrib + startingIndex;

		glVertexAttribPointer((GLuint)attribIndex,
			(GLuint)refl_traits<curAttrib>::fields_count(),
			gl_types_map::found_pair<cType>::value::value,
			(GLboolean)normalized,
			mem_layout_info<vaoPack>::class_size,
			(const void*)mem_layout_info<vaoPack>::padding<nAttrib>
		);

		pointerAttribs_[attribIndex].first = true;
	}

	static void EnableVertexAttrib(size_t attrib)
	{
		glEnableVertexAttribArray((GLuint)attrib);
		pointerAttribs_[attrib].second = true;
	}

private:

	//to generate sizeof...(vaoAttribs) number of bool arguments for "normalized" parameters
	template<class From>
	using to_bool = bool;

	template <size_t ... indx>
	static void VertexAttribPointersPrivate(size_t startingIndex, std::index_sequence<indx...>&&, to_bool<vaoAttribs> ... normalized)
	{
		(VertexAttribPointer<indx>(normalized, startingIndex), ...);
	}

	template <size_t ... indx>
	static void EnableVertexAttributesPrivate(size_t startingIndex, std::index_sequence<indx...>&&)
	{
		(EnableVertexAttrib(startingIndex + indx), ...);
	}

public:

	void VertexAttribPointers(to_bool<vaoAttribs> ... normalized)
	{
		VertexAttribPointersPrivate(std::make_index_sequence<sizeof...(vaoAttribs)>(), normalized...);
	}

	void EnableVertexAttributes()
	{
		EnableVertexAttributesPrivate(std::make_index_sequence<sizeof...(vaoAttribs)>());
	}

};

//TODO: move or substitude with std if any
template <size_t ... s>
constexpr inline size_t sum = (s + ...);

template <class>
class gltVAO;

using AttribAssigned = bool;
using AttribEnabled = bool;

template <class ... vaoAttribs>
class gltVAO<vao_attribs<vaoAttribs...>> //: /*public VAO_base,*/ public vao_enabler<std::tuple<vaoAttribs...>>
{
	//static_assert(all_valid_v<vaoAttribs...>, "Invalid VAO attributes!");

public:
	constexpr static size_t totalAttribs_ = sizeof...(vaoAttribs);
	constexpr static size_t totalComponents_ = sum<glenum_from_type<vaoAttribs>::nComponents...>;

private:
    Handle<glVertexArrayTarget::vao> handle_;
	std::array<std::pair<AttribAssigned, AttribEnabled>, totalComponents_> attribComponents_;

public:


	//using vao_enabler<std::tuple<vaoAttribs...>>::VertexAttribPointers;
	//using vao_enabler<std::tuple<vaoAttribs...>>::EnableVertexAttributes;

    gltVAO(Handle<glVertexArrayTarget::vao>&& handle)
        : handle_(std::move(handle))
    {

    }

    gltVAO() = default;

    void Init(Handle<glVertexArrayTarget::vao>&& handle)
    {
        handle_ = std::move(handle);
        vbo_.Init(glt_buffers::GenBuffer<BufferTarget::array_buffer>());
    }

    bool IsBound() const
    {
        return glt_buffers::IsCurrentVAO(handle_);
    }
   
	// TODO: keep in mind - THIS IS WRONG!
	template <class attribType>
	void SetAttribPointer(size_t indx, size_t indent = 0)
	{
		glVertexAttribPointer(indx,
			attrib_trait<attribType>::nComponents,
			glenum_from_type<attribType>(),
			false,
			sizeof(attrib_trait<vbo_attrs>::KType),
			(void*)0);
	}

	// TODO: rename
	template <class ... vbo_attrs>
	void SetVBO(const gltVBO<vbo_attrs...>& vbo, size_t indxFirst = 0)
	{
		//glVertexAttribPointer()
		glVertexAttribPointer(indxFirst,
			attrib_trait<vbo_attrs>::nComponents,
			glenum_from_type<vbo_attrs>(),
			false,
			sizeof(attrib_trait<vbo_attrs>::KType),
			(void*)0);
	}

	void EnableVertexAttribPointer(size_t indxComponent)
	{
		glEnableVertexAttribArray((GLuint)indxComponent);
	}

	void DisableVertexAttribPointer(size_t indxComponent)
	{
		glDisableVertexAttribArray((GLuint)indxComponent);
	}
};
