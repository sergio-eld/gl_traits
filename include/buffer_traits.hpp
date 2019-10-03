#pragma once

template <class T, const char * glslName>
struct glslt
{
	using type = typename T;
	constexpr static const char * name = glslName;
};

template <class T>
struct is_named_attr : std::bool_constant<false>
{};

template <class T, const char * glslName>
struct is_named_attr<glslt<T, glslName>> : std::bool_constant<true>
{};

template <class T>
constexpr inline bool is_named_attr_v = is_named_attr<T>();

template <class ... vAttribs>
using attr_compound = std::tuple<vAttribs...>;

// TODO: specialization for glm matrix?
template <class vAttrib>
struct attrib_traits
{
	// TODO: check if template parameter is valid  
	constexpr static bool is_compound = false;
	constexpr static size_t stride = sizeof(vAttrib);
	constexpr static size_t numAttribs = 1;
	using base_attribs = typename std::tuple<vAttrib>;
};

template <class ... vAttribs>
struct attrib_traits<attr_compound<vAttribs...>>
{
	constexpr static bool is_compound = true;
	constexpr static size_t stride = sum<sizeof(vAttribs)...>;
	constexpr static size_t numAttribs = sizeof...(vAttribs);
	using base_attribs = typename std::tuple<vAttribs...>;
};


enum class glTargetBuf : unsigned int 
{
	array_buffer = GL_ARRAY_BUFFER,
	atomic_counter_buffer = GL_ATOMIC_COUNTER_BUFFER,
	copy_read_buffer = GL_COPY_READ_BUFFER,
	copy_write_buffer = GL_COPY_WRITE_BUFFER,
	dispatch_indirect_buffer = GL_DISPATCH_INDIRECT_BUFFER,
	draw_indirect_buffer = GL_DRAW_INDIRECT_BUFFER,
	element_array_buffer = GL_ELEMENT_ARRAY_BUFFER,
	pixel_pack_buffer = GL_PIXEL_PACK_BUFFER,
	pixel_unpack_buffer = GL_PIXEL_UNPACK_BUFFER,
	query_buffer = GL_QUERY_BUFFER,
	shader_storage_buffer = GL_SHADER_STORAGE_BUFFER,
	texture_buffer = GL_TEXTURE_BUFFER,
	transform_feedback_buffer = GL_TRANSFORM_FEEDBACK_BUFFER,
	uniform_buffer = GL_UNIFORM_BUFFER
};

enum class glBufferUse : unsigned int
{
	stream_draw = GL_STREAM_DRAW,
	stream_read = GL_STREAM_READ,
	stream_copy = GL_STREAM_COPY,
	static_draw = GL_STATIC_DRAW,
	static_read = GL_STATIC_READ,
	static_copy = GL_STATIC_COPY,
	dynamic_draw = GL_DYNAMIC_DRAW,
	dynamic_read = GL_DYNAMIC_READ,
	dynamic_copy = GL_DYNAMIC_COPY
};


template <size_t indx, class tuple, class seq = indx_seq<indx>>
struct get_offset;

template <size_t indx, class ... attribs, size_t ... i>
struct get_offset<indx, std::tuple<attribs...>, std::index_sequence<i...>>
{
	using sub_tuple = typename std::tuple<nth_type<i, attribs...>...>;
	//constexpr static size_t value = 
	constexpr static size_t value = !indx ? 0 : (sizeof(nth_type<i - 1, attribs>) + ...);
};


class GLT_API buffer_traits
{
	template <glTargetBuf target>
	static const gltHandle<target> *currentBuffer_;
    static const gltHandle<glVAO::vao> *currentVAO_;

public:

	template <glTargetBuf target>
	static gltHandle<target> GenBuffer()
	{
		GLuint handle = 0;
		glGenBuffers(1, &handle);
		assert(handle && "Failed to generate buffer!");
		return gltHandle<target>(handle);
	}

	template <glTargetBuf target>
	static bool IsCurrentBuffer(const gltHandle<target>& handle)
	{
		return currentBuffer_<target> ? *currentBuffer_<target> == handle : false;
	}

	template <glTargetBuf target>
	static void BindBuffer(const gltHandle<target>& handle)
	{
		glBindBuffer((int)target, handle);
		currentBuffer_<target> = &handle;
	}

    static void BindVAO(const gltHandle<glVAO::vao>& handle)
    {
        assert(handle.IsValid() && "Binding invalid VAO");
        glBindVertexArray(handle);
        currentVAO_ = &handle;
    }



	template <glTargetBuf target>
	static void AllocateBuffer(const gltHandle<target>& handle, size_t size, glBufferUse usage)
	{
		assert(IsCurrentBuffer(handle) && "Allocating memory for non-active buffer!");
		glBufferData((GLenum)target, size, nullptr, (GLenum)usage);
	}

	template <glTargetBuf target, typename dataType>
	static void BufferSubData(const gltHandle<target>& handle, size_t offset, const std::vector<dataType>& data)
	{
		// static check for container to store sequenced data?
		glBufferSubData((GLenum)target, (GLintptr)offset, sizeof(dataType) * data.size(), data.data());
	}

	//TODO: restrict usage for array_buffer
	template<size_t sz, typename dataType>
	static void BufferData(glTargetBuf target, const dataType(&data)[sz], glBufferUse usage)
	{
		glBufferData(target, sizeof(data), data, usage);
	}

	template <class dataType>
	static void BufferData(glTargetBuf target, const std::vector<dataType>& data, glBufferUse usage)
	{
		glBufferData((GLenum)target, data.size() * sizeof(dataType), data.data(), (GLenum)usage);
	}

	static gltHandle<glVAO::vao> GenVAO()
	{
		GLuint vao;
		glGenVertexArrays(1, &vao);
		assert(vao && "Failed to generate VAO");
		return gltHandle<glVAO::vao>(vao);
	}

	static bool IsCurrentVAO(const gltHandle<glVAO::vao>& handle)
	{
		return currentVAO_ ? *currentVAO_ == handle : false;
	}


	// TODO: add shader prog handle to VAO class
	// TODO: specialization for glm::vec and glm::mat?
	// TODO: specializtion for compound?
	// use tag dispatching?
	// not compound
	template <typename Attrib>
	static void VertexAttribPointer(Attrib&& tag, const gltHandle<glVAO::vao>& handle, size_t indx,
		size_t strideBytes, size_t offsetBytes,
		bool normalize = false)
	{
		bool is_current = IsCurrentVAO(handle);
		assert(is_current && "Setting attribute pointer for non-active VAO");

		glVertexAttribPointer((GLuint)indx,				// attribute index
			(GLint)glenum_from_type<Attrib>::nComponents,	// attribute's components
			glenum_from_type_v<Attrib>,						// OpenGL type
			normalize,
			(GLsizei)strideBytes,							// stride. 0 if Attrib is not compound (tightly packed)
			(void*)offsetBytes);
	}

	template <class compound, class = indx_seq<attrib_traits<compound>::numAttribs>>
	struct compound_attrib_pointer_;

	template <class ... Attribs, size_t ... indxs>
	struct compound_attrib_pointer_<attr_compound<Attribs...>, std::index_sequence<indxs...>>
	{
		
		static void VertexAttribPointer(const gltHandle<glVAO::vao>& handle,
			size_t firstIndx,
			size_t offsetBytes,
			const std::tuple<to_type<bool, Attribs>...>& normalize)
		{
			constexpr size_t stride = attrib_traits<attr_compound<Attribs...>>::stride;

			/*
			(glVertexAttribPointer((GLuint)(firstIndx + indxs),
				(GLint)glenum_from_type<Attribs>::nComponents,
				glenum_from_type_v<Attribs>,
				std::get<indxs>(normalize),
				stride,
				(void*)offset(std::make_index_sequence<indxs>())), ...);
*/
		}
	};



	// TODO: think of a better way to pass normalized parameters
	template <typename ... Attribs>
	static void VertexAttribPointer(attr_compound<Attribs...>&& tag, const gltHandle<glVAO::vao>& handle, size_t indx,
		size_t offsetBytes, const std::tuple<to_type<bool, Attribs>...>& normalize)
	{
		bool is_current = IsCurrentVAO(handle);
		assert(is_current && "Setting attribute pointer for non-active VAO");

		constexpr size_t stride = attrib_traits<attr_compound<Attribs...>>::stride;
		// need index sequence
		// expand parameter pack here with function call

	}

	static void EnableVertexAttribArray(const gltHandle<glVAO::vao>& handle, size_t indx)
	{
		bool is_current = IsCurrentVAO(handle);
		assert(is_current && "Setting attribute pointer for non-active VAO");

		glEnableVertexAttribArray((GLuint)indx);

	}

};

template <glTargetBuf target>
const gltHandle<target> *buffer_traits::currentBuffer_ = nullptr;


template <glTargetBuf, class ... params>
class gltBuffer;

template <glTargetBuf target>
class gltBuffer<target>
{
	static_assert(target != glTargetBuf::array_buffer, "Array buffer target is not allowed, use corresponding gltBuffer specialization");
	static_assert(target != glTargetBuf::element_array_buffer, "Element Array buffer target is not allowed, use corresponding gltBuffer specialization");

	//TODO: add support for multiple threads. Use static map or something
	//static const gltHandle<target> *currentBuffer_;

    gltHandle<target> handle_;// = buffer_traits::GenBuffer<target>();

public:
	//TODO: move semantics!
    gltBuffer() = default;

    gltBuffer(gltHandle<target>&& handle)
        : handle_(std::move(handle))
    {}

	void Bind()
	{
		buffer_traits::BindBuffer(handle_);
		//currentBuffer_ = &handle_;
	}

	void UnBind()
	{
		assert(IsCurrent() && "Trying to unbind non-active buffer object!");
		buffer_traits::BindBuffer<target>(0);
		currentBuffer_ = nullptr;
	}

	bool IsCurrent() const
	{
		if (!handle_)
			return false;
		return buffer_traits::IsCurrentBuffer(handle);
	}

	operator const gltHandle<target>&() const
	{
		return handle_;
	}

	~gltBuffer()
	{
		if (IsCurrent())
			UnBind();
	}
};

//TODO: move to buffer_traits?
//template <buffer_traits::Target target>
//const gltHandle<target> gltBuffer<target, void>::*currentBuffer_ = nullptr;


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






template <glTargetBuf target, class ... attribs>
class vbo_allocator
{
	const gltHandle<target> *handle_;
	glBufferUse usage_;

	std::array<size_t, sizeof...(attribs)> instancesAllocated_;

public:
	vbo_allocator(const gltHandle<target>* handle)
		: handle_(handle)
	{}

	void AllocateMemory(to_type<size_t, attribs> ... attribInstances, glBufferUse usage)
	{
		size_t totalSize = TotalSize(attribInstances...);
		buffer_traits::AllocateBuffer(*handle_, totalSize, usage);
		UpdateAllocatedInfo(attribInstances..., std::make_index_sequence<sizeof...(attribs)>());
		usage_ = usage;
	}

	size_t AllocatedInstances(size_t attribNum) const
	{
		return instancesAllocated_[attribNum];
	}

	glBufferUse Usage() const
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
	struct all_equivalent<attr_compound<attribs...>, std::tuple<user_attribs...>>
	: std::bool_constant<(is_equivalent<attribs, user_attribs>() && ...)>
{
	static_assert(sizeof...(attribs) == sizeof...(user_attribs), "Numbers of attribs provided mismatch");
	static_assert(!std::conjunction_v<std::bool_constant<attrib_traits<attribs>::is_compound>...>,
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
	else if constexpr (attrib_traits<attrib_class>::is_compound)
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


// TODO: named attributes
template <class ... attribs>
class glVBO : public vbo_allocator<glTargetBuf::array_buffer, attribs...>
{
	using vboalloc = vbo_allocator<glTargetBuf::array_buffer, attribs...>;

	gltHandle<glTargetBuf::array_buffer> handle_;

	template <class C>
	struct deduce_index
	{
		// must fail if more than one similar types are present in the attribs...
		// must fail if C is not equivalent to any of the attribs
		// TODO: deduce if C is compound and compare to only compound attribs

		constexpr static size_t Get()
		{
			static_assert(false, "Can't deduce attribute index from provided class");
			return 0;
		}

		constexpr static size_t indx = Get();
	};

public:

	template <size_t attribIndx>
	size_t AttribMemoryTotal() const
	{
		return AllocatedInstances(attribIndx) * sizeof(nth_type<attribIndx, attribs...>);
	}

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


	glVBO(gltHandle<glTargetBuf::array_buffer>&& handle)
		: vboalloc(&handle_),
		handle_(std::move(handle))
	{}

	//using vboalloc::AllocateMemory;
	//using vboalloc::Usage;


	void Bind()
	{
		buffer_traits::BindBuffer(handle_);
	}

	// TODO: add named attributes?
	// TODO: constexpr function to auto detect attribIndex based on type
	// TODO: check for assigned memory range
	template <size_t attribIndx /*= deduce_index<AttribClass>*/, class AttribClass>
	void BufferData(const std::vector<AttribClass>& data, size_t indxOffset = 0)
	{
		// check if memory has been allocated prior to call.
		// if not and buffering data of type that VBO solely consists of, allocate memory
		// else throw error

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
		buffer_traits::BufferSubData(handle_, GetOffset<attribIndx>(), data);
	}
};

template <class ... attribs>
using glVBO_Compound = glVBO<attr_compound<attribs...>>;

template <class ... attribs>
using glVBO_Batched = glVBO<attribs...>;

// One VBO can continiously store as many attributes as needed. Each attribute can store no more than 4 components
// should we track the active array_buffer and where? in VAO?
template <class ... vaoAttribs>
class gltBuffer<glTargetBuf::array_buffer, vao_attribs<vaoAttribs...>>// : public Buffer_base
{
    gltHandle<glTargetBuf::array_buffer> handle_;// = buffer_traits::GenBuffer<glTargetBuf::array_buffer>();

	size_t elemsLoaded_ = 0;

public:
	//assert vaoAttribs to meet gl requirements (1 - 4 elements, etc)
	//this somehow makes the compiler angry!
	//static_assert((vertex_attrib_validate<vaoAttribs>::valid && ...), "Invalid VAO attributes!");
	//static_assert(all_valid_v<vaoAttribs...>, "Invalid VAO attributes!");

    gltBuffer()
    {}

    gltBuffer(gltHandle<glTargetBuf::array_buffer>&& handle)
        : handle_(std::move(handle))
    {}

    void Init(gltHandle<glTargetBuf::array_buffer>&& handle)
    {
        handle_ = std::move(handle);
    }

	template <class vertexInfo>
	void BufferData(const std::vector<vertexInfo>& vertexes, glBufferUse usage)
	{
		// TODO: compare is_equivalent. glm::vec4 is equivalent to float[4] or std::array<float, 4>

		//static_assert(std::is_same_v<typename refl_traits<vertexInfo, std::tuple<vaoAttribs...>>::field_types,
		//	std::tuple<vaoAttribs...>>, "Attributes mismatch!");
		//assert vertexInfo to have attributes
		//glNamedBufferData(handle_, vertexes.size() * sizeof(vertexInfo), vertexes.data(), usage);     //glNamedBufferData is nullptr!!!!!

		buffer_traits::BufferData(glTargetBuf::array_buffer, vertexes, usage);
		elemsLoaded_ = vertexes.size();

	}

	const gltHandle<glTargetBuf::array_buffer>& Handle() const
	{
		return handle_;
	}

	operator const gltHandle<glTargetBuf::array_buffer>&() const
	{
		return handle_;
	}

	size_t ElementsLoaded() const
	{
		return elemsLoaded_;
	}
};

template <class ... vaoAttribs>
using gltVBO = gltBuffer<glTargetBuf::array_buffer, vao_attribs<vaoAttribs...>>;


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
    gltHandle<glVAO::vao> handle_;
	std::array<std::pair<AttribAssigned, AttribEnabled>, totalComponents_> attribComponents_;

public:


	//using vao_enabler<std::tuple<vaoAttribs...>>::VertexAttribPointers;
	//using vao_enabler<std::tuple<vaoAttribs...>>::EnableVertexAttributes;

    gltVAO(gltHandle<glVAO::vao>&& handle)
        : handle_(std::move(handle))
    {

    }

    gltVAO() = default;

    void Init(gltHandle<glVAO::vao>&& handle)
    {
        handle_ = std::move(handle);
        vbo_.Init(buffer_traits::GenBuffer<glTargetBuf::array_buffer>());
    }

    bool IsCurrent() const
    {
        return buffer_traits::IsCurrentVAO(handle_);
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
