#pragma once

enum class glTargetBuf : int 
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

class buffer_traits
{
public:

	enum Usage {
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
private:
	template <glTargetBuf target>
	static const gltHandle<target> *currentBuffer_;

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
	static bool IsCurrentHandle(const gltHandle<target>& handle)
	{
		return currentBuffer_<target> ? *currentBuffer_<target> == handle : false;
	}

	template <glTargetBuf target>
	static void BindBuffer(const gltHandle<target>& handle)
	{
		glBindBuffer((int)target, handle);
		currentBuffer_<target> = &handle;
	}

	//TODO: restrict usage for array_buffer
	template<size_t sz, typename dataType>
	static void BufferData(glTargetBuf target, const dataType(&data)[sz], Usage usage)
	{
		glBufferData((int)target, sizeof(data), data, usage);
	}

	template <class dataType>
	static void BufferData(glTargetBuf target, const std::vector<dataType>& data, Usage usage)
	{
		glBufferData((int)target, data.size() * sizeof(dataType), data.data(), usage);
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

	gltHandle<target> handle_ = buffer_traits::GenBuffer<target>();

public:
	//TODO: move semantics!

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
		return buffer_traits::IsCurrentHandle(handle);
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

//tracking of active array_buffer should be where? in VAO?
template <class ... vaoAttribs>
class gltBuffer<glTargetBuf::array_buffer, std::tuple<vaoAttribs...>>// : public Buffer_base
{
	gltHandle<glTargetBuf::array_buffer> handle_ = buffer_traits::GenBuffer<glTargetBuf::array_buffer>();

	mutable size_t verticesLoaded_ = 0;

public:
	//assert vaoAttribs to meet gl requirements (1 - 4 elements, etc)
	//this somehow makes the compiler angry!
	//static_assert((vertex_attrib_validate<vaoAttribs>::valid && ...), "Invalid VAO attributes!");
	static_assert(all_valid_v<vaoAttribs...>, "Invalid VAO attributes!");

	template <class vertexInfo>
	void BufferData(const std::vector<vertexInfo>& vertexes, buffer_traits::Usage usage) const
	{
		static_assert(std::is_same_v<typename refl_traits<vertexInfo, std::tuple<vaoAttribs...>>::field_types,
			std::tuple<vaoAttribs...>>, "Attributes mismatch!");
		//assert vertexInfo to have attributes
		//glNamedBufferData(handle_, vertexes.size() * sizeof(vertexInfo), vertexes.data(), usage);     //glNamedBufferData is nullptr!!!!!

		buffer_traits::BufferData(glTargetBuf::array_buffer, vertexes, usage);
		verticesLoaded_ = vertexes.size();

	}

	const gltHandle<glTargetBuf::array_buffer>& Handle() const
	{
		return handle_;
	}

	size_t VerticesLoaded() const
	{
		return verticesLoaded_;
	}
};





class VAO_base
{
	static GLuint vaoCurrent_;

protected:
	GLuint handle_ = 0;

public:
	VAO_base();

	VAO_base(const VAO_base&) = delete;
	VAO_base& operator=(const VAO_base&) = delete;

	VAO_base(VAO_base&& other);
	VAO_base& operator=(VAO_base&&);

	bool IsCurrent() const;
	virtual void Bind();
	virtual void UnBind();

	~VAO_base();
};

template <typename>
class vao_enabler;

template <typename ... vaoAttribs>
class vao_enabler<std::tuple<vaoAttribs...>>
{
public:

	template <size_t nAttrib>
	static void VertexAttribPointer(bool normalized)
	{
		typedef std::tuple<vaoAttribs...> vaoPack;
		typedef std::tuple_element_t<nAttrib, std::tuple<vaoAttribs...>> curAttrib;
		typedef std::tuple_element_t<0, refl_traits<curAttrib>::field_types> cType;

		glVertexAttribPointer((GLuint)nAttrib,
			(GLuint)refl_traits<curAttrib>::fields_count(),
			gl_types_map::found_pair<cType>::value::value,
			(GLboolean)normalized,
			mem_layout_info<vaoPack>::class_size,
			(const void*)mem_layout_info<vaoPack>::padding<nAttrib>
		);
	}

	static void EnableVertexAttrib(size_t attrib)
	{
		glEnableVertexAttribArray((GLuint)attrib);
	}

private:

	//to generate sizeof...(vaoAttribs) number of bool arguments for "normalized" parameters
	template<class From>
	using to_bool = bool;

	template <size_t ... indx>
	static void VertexAttribPointersPrivate(std::index_sequence<indx...>&&, to_bool<vaoAttribs> ... normalized)
	{
		(VertexAttribPointer<indx>(normalized), ...);
	}

	template <size_t ... indx>
	static void EnableVertexAttributesPrivate(std::index_sequence<indx...>&&)
	{
		(EnableVertexAttrib(indx), ...);
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


template <class>
class gltVAO;

template <class ... vaoAttribs>
class gltVAO<std::tuple<vaoAttribs...>> : public VAO_base, public vao_enabler<std::tuple<vaoAttribs...>>
{
	static_assert(all_valid_v<vaoAttribs...>, "Invalid VAO attributes!");
public:
	using VBO = gltBuffer<glTargetBuf::array_buffer, std::tuple<vaoAttribs...>>;

private:
	VBO vbo_{};
	//gltBuffer<buffer_traits //add ebo
	bool vboPending_ = true;

public:

	using vao_enabler<std::tuple<vaoAttribs...>>::VertexAttribPointers;
	using vao_enabler<std::tuple<vaoAttribs...>>::EnableVertexAttributes;

	const VBO& GetVBO() const
	{
		return vbo_;
	}

	void AssignVBO(VBO&& vbo)
	{
		vbo_ = std::move(vbo);
		if (!IsCurrent())
		{
			vboPending_ = true;
			return;
		}
		buffer_traits::BindBuffer<glTargetBuf::array_buffer>(vbo_.Handle());
	}

	void Bind() override
	{
		VAO_base::Bind();
		if (vboPending_)
		{
			buffer_traits::BindBuffer<glTargetBuf::array_buffer>(vbo_.Handle());
			vboPending_ = false;
		}
	}

};
