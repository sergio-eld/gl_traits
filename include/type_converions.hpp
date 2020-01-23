#pragma once

namespace glt
{
	template <auto A>
	using glt_constant = std::integral_constant<decltype(A), A>;

	template <typename cType>
	struct c_to_gl;

	template <> struct c_to_gl<GLbyte> : glt_constant<glType::gl_byte> {};
	template <> struct c_to_gl<GLubyte> : glt_constant<glType::gl_unsigned_byte> {};
	template <> struct c_to_gl<GLshort> : glt_constant<glType::gl_short> {};
	template <> struct c_to_gl<GLushort> : glt_constant<glType::gl_unsigned_short> {};
	template <> struct c_to_gl<GLint> : glt_constant<glType::gl_int> {};
	template <> struct c_to_gl<GLuint> : glt_constant<glType::gl_unsigned_int> {};
	//template <> struct c_to_gl<GLfixed> : std::integral_constant<int, GL_FIXED> {};
	//template <> struct c_to_gl<GLhalf> :  std::integral_constant<int, GL_HALF_FLOAT> {};
	template <> struct c_to_gl<GLfloat> : glt_constant<glType::gl_float> {};
	template <> struct c_to_gl<GLdouble> : glt_constant<glType::gl_double> {};

	template <typename cType>
	struct c_to_gl_find
	{
		constexpr static glType value = c_to_gl<cType>();
	};

	template <glm::length_t L, typename T, glm::qualifier Q>
	struct c_to_gl_find<glm::vec<L, T, Q>>
	{
		constexpr static glType value = c_to_gl<T>();
	};

	// get glsl enum value from c type
	// TODO: add glm types recognition
	template <typename cType>
	constexpr inline glType c_to_gl_v = c_to_gl_find<cType>::value;


	/*
	OpenGl has similar pipelines for using various objects:
	1. Gen/Create
	2. Bind/Use
	3. Delete

	void glGenBuffers           (GLsizei, GLuint*)      void glDeleteBuffers            (GLsizei, const GLuint*)     void glBindBuffer      (GLenum, GLuint)
	void glGenTextures          (GLsizei, GLuint*)      void glDeleteTextures           (GLsizei, const GLuint*)     void glBindTexture     (GLenum, GLuint)
	void glGenFramebuffers      (GLsizei, GLuint*)      void glDeleteFramebuffers       (GLsizei, const GLuint*)     void glBindFramebuffer (GLenum, GLuint)
	void glGenSamplers          (GLsizei, GLuint*)      void glDeleteSamplers           (GLsizei, const GLuint*)     void glBindSampler     (GLuint, GLuint)

	void glGenVertexArrays      (GLsizei, GLuint*)      void glDeleteVertexArrays       (GLsizei, const GLuint*)     void glBindVertexArray (GLuint)


	void glGenRenderbuffers     (GLsizei, GLuint*)      void glDeleteRenderbuffers      (GLsizei, const GLuint*)     none ??

	void glGenProgramPipelines  (GLsizei, GLuint*)      void glDeleteProgramPipelines   (GLsizei, const GLuint*)     void glBindProgramPipeline(GLuint)
	void glGenQueries           (GLsizei, GLuint*)      void glDeleteQueries            (GLsizei, const GLuint*)     none ??
	void glGenTransformFeedbacks(GLsizei, GLuint*)      void glDeleteTransformFeedbacks (GLsizei, const GLuint*)     void glBindTransformFeedback(GLuint)

	GLuint glCreateProgram      (void)                  void glDeleteProgram            (GLuint)
	GLuint glCreateShader       (GLenum)                void glDeleteShader             (GLuint)

	*/

	// map for gl allocators
	template <typename glObjType>
	struct pp_gl_allocator;

	template <> struct pp_gl_allocator<BufferTarget> : glt_constant<&glGenBuffers> {};
	template <> struct pp_gl_allocator<FrameBufferTarget> : glt_constant<&glGenFramebuffers> {};
	template <> struct pp_gl_allocator<TextureTarget> : glt_constant<&glGenTextures> {};
	template <> struct pp_gl_allocator<VAOTarget> : glt_constant<&glGenVertexArrays> {};

	template <> struct pp_gl_allocator<TransformFeedBackTarget> : glt_constant<&glGenTransformFeedbacks> {};
	template <> struct pp_gl_allocator<QueryTarget> : glt_constant<&glGenQueries> {};

	template <> struct pp_gl_allocator<ProgramPipeLineTarget> : glt_constant<&glGenProgramPipelines> {};
	template <> struct pp_gl_allocator<RenderBufferTarget> : glt_constant<&glGenRenderbuffers> {};
	template <> struct pp_gl_allocator<SamplerTarget> : glt_constant<&glGenSamplers> {};

	template <> struct pp_gl_allocator<ProgramTarget> : glt_constant<&glCreateProgram> {};

	// takes argument 
	template <> struct pp_gl_allocator<ShaderTarget> : glt_constant<&glCreateShader> {};

	template <typename glObjType>
	constexpr inline auto pp_gl_allocator_v = pp_gl_allocator<glObjType>::value;

	// map for gl deleters
	template <typename glObjType>
	struct pp_gl_deleter;

	template <> struct pp_gl_deleter<BufferTarget> : glt_constant<&glDeleteBuffers> {};
	template <> struct pp_gl_deleter<FrameBufferTarget> : glt_constant<&glDeleteFramebuffers> {};
	template <> struct pp_gl_deleter<TextureTarget> : glt_constant<&glDeleteTextures> {};
	template <> struct pp_gl_deleter<VAOTarget> : glt_constant<&glDeleteVertexArrays> {};

	template <> struct pp_gl_deleter<TransformFeedBackTarget> : glt_constant<&glDeleteTransformFeedbacks> {};
	template <> struct pp_gl_deleter<QueryTarget> : glt_constant<&glDeleteQueries> {};

	template <> struct pp_gl_deleter<ProgramPipeLineTarget> : glt_constant<&glDeleteProgramPipelines> {};
	template <> struct pp_gl_deleter<RenderBufferTarget> : glt_constant<&glDeleteRenderbuffers> {};
	template <> struct pp_gl_deleter<SamplerTarget> : glt_constant<&glDeleteSamplers> {};


	template <> struct pp_gl_deleter<ShaderTarget> : glt_constant<&glDeleteShader> {};
	template <> struct pp_gl_deleter<ProgramTarget> : glt_constant<&glDeleteProgram> {};

	template <typename glObjType>
	constexpr inline auto pp_gl_deleter_v = pp_gl_deleter<glObjType>::value;

	template <typename glObjType>
	constexpr inline bool has_func_bind_v = std::bool_constant<std::disjunction_v<
		std::is_same<glObjType, BufferTarget>,
		std::is_same<glObjType, FrameBufferTarget>,
		std::is_same<glObjType, TextureTarget>,
		std::is_same<glObjType, VAOTarget>,
		std::is_same<glObjType, TransformFeedBackTarget>
		>>::value;

	// map for binding functions
	template <typename glObjType>
	struct pp_gl_binder;

	template <> struct pp_gl_binder<BufferTarget> : glt_constant<&glBindBuffer> {};
	template <> struct pp_gl_binder<FrameBufferTarget> : glt_constant<&glBindFramebuffer> {};
	template <> struct pp_gl_binder<TextureTarget> : glt_constant<&glBindTexture> {};
	template <> struct pp_gl_binder<TransformFeedBackTarget> : glt_constant<&glBindTransformFeedback> {};

	template <> struct pp_gl_binder<VAOTarget> : glt_constant<&glBindVertexArray> {};

	template <typename glObjType>
	constexpr inline auto pp_gl_binder_v = pp_gl_binder<glObjType>::value;

	// buffer targets 


	template <auto>
	struct get_binding : std::integral_constant<void*, 0> {};

	template <auto A>
	constexpr inline bool has_gl_binding_v = !std::is_same_v<get_binding<A>::type, void*>;

	// TODO: generate this maps using python
	template <> struct get_binding<BufferTarget::array> : glt_constant<glBufferBinding::array_buffer_binding> {};
	template <> struct get_binding<BufferTarget::atomic_counter> : glt_constant<glBufferBinding::atomic_buffer_binding> {};
	template <> struct get_binding<BufferTarget::copy_read> : glt_constant<glBufferBinding::copy_read_buffer_binding> {};
	template <> struct get_binding<BufferTarget::copy_write> : glt_constant<glBufferBinding::copy_write_buffer_binding> {};
	template <> struct get_binding<BufferTarget::dispatch_indirect> : glt_constant<glBufferBinding::dispatch_indirect_buffer_binding> {};
	template <> struct get_binding<BufferTarget::draw_indirect> : glt_constant<glBufferBinding::draw_indirect_buffer_binding> {};
	template <> struct get_binding<BufferTarget::element_array> : glt_constant<glBufferBinding::element_array_buffer_binding> {};
	template <> struct get_binding<BufferTarget::pixel_pack> : glt_constant<glBufferBinding::pixel_pack_buffer_binding> {};
	template <> struct get_binding<BufferTarget::pixel_unpack> : glt_constant<glBufferBinding::pixel_unpack_buffer_binding> {};
	template <> struct get_binding<BufferTarget::shader_storage> : glt_constant<glBufferBinding::shader_storage_buffer_binding> {};
	template <> struct get_binding<BufferTarget::transform_feedback> : glt_constant<glBufferBinding::transform_feedback_buffer_binding> {};
	template <> struct get_binding<BufferTarget::uniform> : glt_constant<glBufferBinding::uniform_buffer_binding> {};

	// get gl Binding value from gl Target value (to access corresponing function pointer)
	template <auto A>
	constexpr inline auto get_binding_v = get_binding<A>::value;


	//////////////////////////////////////////////////
	// attribute traits
	/////////////////////////////////////////////////

	/* Get size of an Attribute for glVertexAttributePointer */
	template <class>
	struct vao_attrib_size
		: std::integral_constant<VAOAttribSize, VAOAttribSize::one> {};

	template <glm::length_t L, typename T, glm::qualifier Q>
	struct vao_attrib_size<glm::vec<L, T, Q>>
		: std::integral_constant<VAOAttribSize, (VAOAttribSize)L> {};

	/*
	template <class T>
	constexpr inline VAOAttribSize vao_attrib_size_v = vao_attrib_size<T>();*/

	template <class T>
	struct elements_count : glt_constant<(size_t)1> {};

	template <glm::length_t L, typename T, glm::qualifier Q>
	struct elements_count<glm::vec<L, T, Q>> : glt_constant<L> {};

	template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct elements_count<glm::mat<C, R, T, Q>> : glt_constant<C> {};

	template <class T>
	constexpr inline size_t elements_count_v = elements_count<T>();



	/* Convert one type to another. Used when expanding parameter pack.
    Example: set n = sizeof...(T) arguments in a Foo<T..>::bar;
    template <class ... T>
    class Foo
    {
	    void bar(convert_to<bool, T> ... arg) // to invoke, n bool args is needed.
	    {}
    };
    */
	template <typename To, typename From>
	using convert_to = To;

	/*
	Similar to "convert_to", but converting from non-template parameter.
	*/
	template <typename To, auto indx>
	using convert_v_to = To;

	template <class T, size_t sz = 1, class = decltype(std::make_index_sequence<sz>())>
	struct p_gl_uniform;

	template <class T, size_t sz, size_t ... indx>
	struct p_gl_uniform<T, sz, std::index_sequence<indx...>>
	{
		using type = void(*)(GLint, convert_v_to<T, indx>...);
	};

	template <glm::length_t L, typename T, glm::qualifier Q>
	struct p_gl_uniform<glm::vec<L, T, Q>>
	{
		using type = void(*)(GLint, GLsizei, const glm::vec<L, T, Q>&);
	};

	template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct p_gl_uniform<glm::mat<C, R, T, Q>>
	{
		using type = void(*)(GLint, GLsizei, GLboolean, const glm::mat<C, R, T, Q>&);
	};

	template <class T, size_t sz = 1>
	using p_gl_uniform_t = typename p_gl_uniform<T, sz>::type;


	template <auto *vPtr, class T, size_t sz = 1>
	struct p_gl_uniform_const
	{
		constexpr static p_gl_uniform_t<T, sz> *value = 0;// vPtr;

		constexpr p_gl_uniform_t<T, sz>* operator()() const
		{
			return value;
		}
	};


	template <class T, size_t = 1>
	struct pp_gl_uniform_map;

	template <> struct pp_gl_uniform_map<float> : glt_constant<&glUniform1f> {};
	template <> struct pp_gl_uniform_map<float, 2> : glt_constant<&glUniform2f> {};
	template <> struct pp_gl_uniform_map<float, 3> : glt_constant<&glUniform3f> {};
	template <> struct pp_gl_uniform_map<float, 4> : glt_constant<&glUniform4f> {};

	template <> struct pp_gl_uniform_map<double> : glt_constant<&glUniform1d> {};
	template <> struct pp_gl_uniform_map<double, 2> : glt_constant<&glUniform2d> {};
	template <> struct pp_gl_uniform_map<double, 3> : glt_constant<&glUniform3d> {};
	template <> struct pp_gl_uniform_map<double, 4> : glt_constant<&glUniform4d> {};

	template <> struct pp_gl_uniform_map<int> : glt_constant<&glUniform1i> {};
	template <> struct pp_gl_uniform_map<int, 2> : glt_constant<&glUniform2i> {};
	template <> struct pp_gl_uniform_map<int, 3> : glt_constant<&glUniform3i> {};
	template <> struct pp_gl_uniform_map<int, 4> : glt_constant<&glUniform4i> {};

	template <> struct pp_gl_uniform_map<unsigned int> : glt_constant<&glUniform1ui> {};
	template <> struct pp_gl_uniform_map<unsigned int, 2> : glt_constant<&glUniform2ui> {};
	template <> struct pp_gl_uniform_map<unsigned int, 3> : glt_constant<&glUniform3ui> {};
	template <> struct pp_gl_uniform_map<unsigned int, 4> : glt_constant<&glUniform4ui> {};

	template <> struct pp_gl_uniform_map<glm::vec1> : glt_constant<&glUniform1fv> {};
	template <> struct pp_gl_uniform_map<glm::vec2> : glt_constant<&glUniform2fv> {};
	template <> struct pp_gl_uniform_map<glm::vec3> : glt_constant<&glUniform3fv> {};
	template <> struct pp_gl_uniform_map<glm::vec4> : glt_constant<&glUniform4fv> {};

	template <> struct pp_gl_uniform_map<glm::ivec1> : glt_constant<&glUniform1iv> {};
	template <> struct pp_gl_uniform_map<glm::ivec2> : glt_constant<&glUniform2iv> {};
	template <> struct pp_gl_uniform_map<glm::ivec3> : glt_constant<&glUniform3iv> {};
	template <> struct pp_gl_uniform_map<glm::ivec4> : glt_constant<&glUniform4iv> {};

	template <> struct pp_gl_uniform_map<glm::uvec1> : glt_constant<&glUniform1uiv> {};
	template <> struct pp_gl_uniform_map<glm::uvec2> : glt_constant<&glUniform2uiv> {};
	template <> struct pp_gl_uniform_map<glm::uvec3> : glt_constant<&glUniform3uiv> {};
	template <> struct pp_gl_uniform_map<glm::uvec4> : glt_constant<&glUniform4uiv> {};

	template <> struct pp_gl_uniform_map<glm::mat2> : glt_constant<&glUniformMatrix2fv> {};
	template <> struct pp_gl_uniform_map<glm::mat3> : glt_constant<&glUniformMatrix3fv> {};
	template <> struct pp_gl_uniform_map<glm::mat4> : glt_constant<&glUniformMatrix4fv> {};

	template <> struct pp_gl_uniform_map<glm::mat2x3> : glt_constant<&glUniformMatrix2x3fv> {};
	template <> struct pp_gl_uniform_map<glm::mat3x2> : glt_constant<&glUniformMatrix3x2fv> {};

	template <> struct pp_gl_uniform_map<glm::mat2x4> : glt_constant<&glUniformMatrix2x4fv> {};
	template <> struct pp_gl_uniform_map<glm::mat4x2> : glt_constant<&glUniformMatrix4x2fv> {};

	template <> struct pp_gl_uniform_map<glm::mat4x3> : glt_constant<&glUniformMatrix4x3fv> {};
	template <> struct pp_gl_uniform_map<glm::mat3x4> : glt_constant<&glUniformMatrix3x4fv> {};

	template <class T>
	constexpr inline auto p_gl_uniform_map_v = pp_gl_uniform_map<T>();

	// default inpArgs wll lead to an error, must be 1 for vectors and matrices
	template <class T, size_t inpArgs>
	constexpr p_gl_uniform_t<T, inpArgs> get_p_gl_uniform()
	{
		return reinterpret_cast<p_gl_uniform_t<T, inpArgs>>(*p_gl_uniform_map_v<T>);
	}


	template <class T>
	struct pp_gl_get_uniform_map;

	template <> struct pp_gl_get_uniform_map<float> : glt_constant<&glGetUniformfv> {};
	template <> struct pp_gl_get_uniform_map<double> : glt_constant<&glGetUniformdv> {};
	template <> struct pp_gl_get_uniform_map<int> : glt_constant<&glGetUniformiv> {};
	template <> struct pp_gl_get_uniform_map<unsigned int> : glt_constant<&glGetUniformuiv> {};

}