#pragma once

#include <cassert>

#include "glad/glad.h"

// how to remove this dependency?
#include "glm/glm.hpp"

#include <type_traits>

namespace glt
{
	// gl-type conversion
	enum class glType : int
	{
		gl_byte = GL_BYTE,
		gl_unsigned_byte = GL_UNSIGNED_BYTE,
		gl_short = GL_SHORT,
		gl_unsigned_short = GL_UNSIGNED_SHORT,
		gl_int = GL_INT,
		gl_unsigned_int = GL_UNSIGNED_INT,
		gl_fixed = GL_FIXED,
		gl_half_float = GL_HALF_FLOAT,
		gl_float = GL_FLOAT,
		gl_double = GL_DOUBLE
	};



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

	template <typename eTargetType>
	class Handle;

	template <auto A>
	using glt_constant = std::integral_constant<decltype(A), A>;

	enum class BufferTarget : int;
	enum class glFrameBufferTarget : int;
	enum class TextureTarget : int;
	enum class glShaderTarget : int;

	enum class glTransformFeedBackTarget : int; // GL_TRANSFORM_FEEDBACK only
	enum class glQueryTarget : int;             // used in glBeginQuery


	// types only to be used as a paramaeter for Handle<T>, to find allocator and deleter functions 
	enum class glProgramTarget : int;           // empty

	enum class VAOTarget : int;       // empty
	enum class glProgramPipeLineTarget : int;   // empty
	enum class glRenderBufferTarget : int;      // empty
	enum class glSamplerTarget : int;           // empty

	// map for gl allocators
	template <typename glObjType>
	struct pp_gl_allocator;

	template <> struct pp_gl_allocator<BufferTarget> : glt_constant<&glGenBuffers> {};
	template <> struct pp_gl_allocator<glFrameBufferTarget> : glt_constant<&glGenFramebuffers> {};
	template <> struct pp_gl_allocator<TextureTarget> : glt_constant<&glGenTextures> {};
	template <> struct pp_gl_allocator<VAOTarget> : glt_constant<&glGenVertexArrays> {};

	template <> struct pp_gl_allocator<glTransformFeedBackTarget> : glt_constant<&glGenTransformFeedbacks> {};
	template <> struct pp_gl_allocator<glQueryTarget> : glt_constant<&glGenQueries> {};

	template <> struct pp_gl_allocator<glProgramPipeLineTarget> : glt_constant<&glGenProgramPipelines> {};
	template <> struct pp_gl_allocator<glRenderBufferTarget> : glt_constant<&glGenRenderbuffers> {};
	template <> struct pp_gl_allocator<glSamplerTarget> : glt_constant<&glGenSamplers> {};

	template <> struct pp_gl_allocator<glProgramTarget> : glt_constant<&glCreateProgram> {};

	// takes argument 
	template <> struct pp_gl_allocator<glShaderTarget> : glt_constant<&glCreateShader> {};

	template <typename glObjType>
	constexpr inline auto pp_gl_allocator_v = pp_gl_allocator<glObjType>::value;

	// map for gl deleters
	template <typename glObjType>
	struct pp_gl_deleter;

	template <> struct pp_gl_deleter<BufferTarget> : glt_constant<&glDeleteBuffers> {};
	template <> struct pp_gl_deleter<glFrameBufferTarget> : glt_constant<&glDeleteFramebuffers> {};
	template <> struct pp_gl_deleter<TextureTarget> : glt_constant<&glDeleteTextures> {};
	template <> struct pp_gl_deleter<VAOTarget> : glt_constant<&glDeleteVertexArrays> {};

	template <> struct pp_gl_deleter<glTransformFeedBackTarget> : glt_constant<&glDeleteTransformFeedbacks> {};
	template <> struct pp_gl_deleter<glQueryTarget> : glt_constant<&glDeleteQueries> {};

	template <> struct pp_gl_deleter<glProgramPipeLineTarget> : glt_constant<&glDeleteProgramPipelines> {};
	template <> struct pp_gl_deleter<glRenderBufferTarget> : glt_constant<&glDeleteRenderbuffers> {};
	template <> struct pp_gl_deleter<glSamplerTarget> : glt_constant<&glDeleteSamplers> {};


	template <> struct pp_gl_deleter<glShaderTarget> : glt_constant<&glDeleteShader> {};
	template <> struct pp_gl_deleter<glProgramTarget> : glt_constant<&glDeleteProgram> {};

	template <typename glObjType>
	constexpr inline auto pp_gl_deleter_v = pp_gl_deleter<glObjType>::value;

	template <typename glObjType>
	constexpr inline bool has_func_bind_v = std::bool_constant<std::disjunction_v<
		std::is_same<glObjType, BufferTarget>,
		std::is_same<glObjType, glFrameBufferTarget>,
		std::is_same<glObjType, TextureTarget>,
		std::is_same<glObjType, VAOTarget>,
		std::is_same<glObjType, glTransformFeedBackTarget>
		>>::value;

	// map for binding functions
	template <typename glObjType>
	struct pp_gl_binder;

	template <> struct pp_gl_binder<BufferTarget> : glt_constant<&glBindBuffer> {};
	template <> struct pp_gl_binder<glFrameBufferTarget> : glt_constant<&glBindFramebuffer> {};
	template <> struct pp_gl_binder<TextureTarget> : glt_constant<&glBindTexture> {};
	template <> struct pp_gl_binder<glTransformFeedBackTarget> : glt_constant<&glBindTransformFeedback> {};

	template <> struct pp_gl_binder<VAOTarget> : glt_constant<&glBindVertexArray> {};

	template <typename glObjType>
	constexpr inline auto pp_gl_binder_v = pp_gl_binder<glObjType>::value;

	// buffer targets 
	enum class BufferTarget : int
	{
		none = 0,
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

	using BufferTargetList = std::integer_sequence<BufferTarget,
		BufferTarget::array_buffer,
		BufferTarget::atomic_counter_buffer,
		BufferTarget::copy_read_buffer,
		BufferTarget::copy_write_buffer,
		BufferTarget::dispatch_indirect_buffer,
		BufferTarget::draw_indirect_buffer,
		BufferTarget::element_array_buffer,
		BufferTarget::pixel_pack_buffer,
		BufferTarget::pixel_unpack_buffer,
		BufferTarget::query_buffer,
		BufferTarget::shader_storage_buffer,
		BufferTarget::texture_buffer,
		BufferTarget::transform_feedback_buffer,
		BufferTarget::uniform_buffer>;


	enum class glBufferBinding : int
	{
		array_buffer_binding = GL_ARRAY_BUFFER_BINDING,
		atomic_buffer_binding = GL_ATOMIC_COUNTER_BUFFER_BINDING,
		copy_read_buffer_binding = GL_COPY_READ_BUFFER_BINDING,
		copy_write_buffer_binding = GL_COPY_WRITE_BUFFER_BINDING,
		draw_indirect_buffer_binding = GL_DRAW_INDIRECT_BUFFER_BINDING,
		dispatch_indirect_buffer_binding = GL_DISPATCH_INDIRECT_BUFFER_BINDING,
		element_array_buffer_binding = GL_ELEMENT_ARRAY_BUFFER_BINDING,
		pixel_pack_buffer_binding = GL_PIXEL_PACK_BUFFER_BINDING,
		pixel_unpack_buffer_binding = GL_PIXEL_UNPACK_BUFFER_BINDING,
		shader_storage_buffer_binding = GL_SHADER_STORAGE_BUFFER_BINDING,
		transform_feedback_buffer_binding = GL_TRANSFORM_FEEDBACK_BUFFER_BINDING,
		uniform_buffer_binding = GL_UNIFORM_BUFFER_BINDING
	};

	template <auto>
	struct get_binding : std::integral_constant<void*, 0> {};

	template <auto A>
	constexpr inline bool has_gl_binding_v = !std::is_same_v<get_binding<A>::type, void*>;

	// TODO: generate this maps using python
	template <> struct get_binding<BufferTarget::array_buffer> : glt_constant<glBufferBinding::array_buffer_binding> {};
	template <> struct get_binding<BufferTarget::atomic_counter_buffer> : glt_constant<glBufferBinding::atomic_buffer_binding> {};
	template <> struct get_binding<BufferTarget::copy_read_buffer> : glt_constant<glBufferBinding::copy_read_buffer_binding> {};
	template <> struct get_binding<BufferTarget::copy_write_buffer> : glt_constant<glBufferBinding::copy_write_buffer_binding> {};
	template <> struct get_binding<BufferTarget::dispatch_indirect_buffer> : glt_constant<glBufferBinding::dispatch_indirect_buffer_binding> {};
	template <> struct get_binding<BufferTarget::draw_indirect_buffer> : glt_constant<glBufferBinding::draw_indirect_buffer_binding> {};
	template <> struct get_binding<BufferTarget::element_array_buffer> : glt_constant<glBufferBinding::element_array_buffer_binding> {};
	template <> struct get_binding<BufferTarget::pixel_pack_buffer> : glt_constant<glBufferBinding::pixel_pack_buffer_binding> {};
	template <> struct get_binding<BufferTarget::pixel_unpack_buffer> : glt_constant<glBufferBinding::pixel_unpack_buffer_binding> {};
	template <> struct get_binding<BufferTarget::shader_storage_buffer> : glt_constant<glBufferBinding::shader_storage_buffer_binding> {};
	template <> struct get_binding<BufferTarget::transform_feedback_buffer> : glt_constant<glBufferBinding::transform_feedback_buffer_binding> {};
	template <> struct get_binding<BufferTarget::uniform_buffer> : glt_constant<glBufferBinding::uniform_buffer_binding> {};

	// get gl Binding value from gl Target value (to access corresponing function pointer)
	template <auto A>
	constexpr inline auto get_binding_v = get_binding<A>::value;

	template <class T, typename = std::void_t<>>
	struct has_GetHandle : std::false_type {};

	template <template <typename>  class T, typename TargetType>
	struct has_GetHandle<T<TargetType>,
		std::void_t<decltype(	// get type from function pointer
		(const Handle<TargetType>&(T<TargetType>::*)() const)	// cast function pointer to type
			&T<TargetType>::GetHandle)>> : std::true_type {};	// function pointer

	enum class BufferUse : int
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


	// texture targets // remove underscore
	enum class TextureTarget : int
	{
		//for glBindTexture
		texture_1d = GL_TEXTURE_1D,							//+
		texture_2d = GL_TEXTURE_2D,							//+
		texture_3d = GL_TEXTURE_3D,							//+
		texture_1d_array = GL_TEXTURE_1D_ARRAY,				//+
		texture_2d_array = GL_TEXTURE_2D_ARRAY,				//+
		texture_rectangle = GL_TEXTURE_RECTANGLE,			//+	
		texture_cube_map = GL_TEXTURE_CUBE_MAP,				//?
		texture_cube_map_array = GL_TEXTURE_CUBE_MAP_ARRAY,	//??
		texture_buffer = GL_TEXTURE_BUFFER,					//??
		texture_2d_multisample = GL_TEXTURE_2D_MULTISAMPLE,				//+
		texture_2d_multisample_array = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,	//+


		proxy_texture_1d = GL_PROXY_TEXTURE_1D,  //glTexImage1D

		proxy_texture_2d = GL_PROXY_TEXTURE_2D, //glTexImage2D
		proxy_texture_1d_array = GL_PROXY_TEXTURE_1D_ARRAY,
		proxy_texture_rectangle = GL_PROXY_TEXTURE_RECTANGLE,
		texture_cube_map_positive_x = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		texture_cube_map_negative_x = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		texture_cube_map_positive_y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		texture_cube_map_negative_y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		texture_cube_map_positive_z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		texture_cube_map_negative_z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		proxy_texture_cube_map = GL_PROXY_TEXTURE_CUBE_MAP,

		proxy_texture_2d_multisample = GL_PROXY_TEXTURE_2D_MULTISAMPLE,	//glTexImage2DMultisample

		proxy_texture_3d = GL_PROXY_TEXTURE_3D,
		proxy_texture_2d_array = GL_PROXY_TEXTURE_2D_ARRAY,

		proxy_texture_2d_multisample_array = GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY
	};


	enum class ShaderTarget : int
	{
		compute = GL_COMPUTE_SHADER,
		vertex = GL_VERTEX_SHADER,
		tess_control = GL_TESS_CONTROL_SHADER,
		tess_evaluation = GL_TESS_EVALUATION_SHADER,
		geometry = GL_GEOMETRY_SHADER,
		fragment = GL_FRAGMENT_SHADER
	};


	// glEnable arguments
	enum class Capability : int
	{
		blend = GL_BLEND,
		clip_distance0 = GL_CLIP_DISTANCE0,
		color_logic_op = GL_COLOR_LOGIC_OP,
		cull_face = GL_CULL_FACE,
		debug_output = GL_DEBUG_OUTPUT,
		debug_output_synchronous = GL_DEBUG_OUTPUT_SYNCHRONOUS,
		depth_clamp = GL_DEPTH_CLAMP,
		depth_test = GL_DEPTH_TEST,
		dither = GL_DITHER,
		framebuffer_srgb = GL_FRAMEBUFFER_SRGB,
		line_smooth = GL_LINE_SMOOTH,
		multisample = GL_MULTISAMPLE,
		polygon_offset_line = GL_POLYGON_OFFSET_LINE,
		polygon_smooth = GL_POLYGON_SMOOTH,
		primitive_restart = GL_PRIMITIVE_RESTART,
		primitive_restart_fixed_index = GL_PRIMITIVE_RESTART_FIXED_INDEX,
		rasterizer_discard = GL_RASTERIZER_DISCARD,
		sample_alpha_to_coverage = GL_SAMPLE_ALPHA_TO_COVERAGE,
		sample_alpha_to_one = GL_SAMPLE_ALPHA_TO_ONE,
		sample_coverage = GL_SAMPLE_COVERAGE,
		sample_shading = GL_SAMPLE_SHADING,
		sample_mask = GL_SAMPLE_MASK,
		scissor_test = GL_SCISSOR_TEST,
		stencil_test = GL_STENCIL_TEST,
		texture_cube_map_seamless = GL_TEXTURE_CUBE_MAP_SEAMLESS,
		program_point_size = GL_PROGRAM_POINT_SIZE
	};

	enum class MapAccess : int
	{
		read_only = GL_READ_ONLY,
		write_only = GL_WRITE_ONLY,
		read_write = GL_READ_WRITE
	};

	enum class MapAccessBit : GLbitfield
	{
		read = GL_MAP_READ_BIT,
		write = GL_MAP_WRITE_BIT,
		persistent = GL_MAP_PERSISTENT_BIT,
		coherent = GL_MAP_COHERENT_BIT,

		// modifying flags
		invalidate_range = GL_MAP_INVALIDATE_RANGE_BIT,
		invalidate_buffer = GL_MAP_INVALIDATE_BUFFER_BIT,
		flush_explicit = GL_MAP_FLUSH_EXPLICIT_BIT,
		unsynchronized = GL_MAP_UNSYNCHRONIZED_BIT,

	};

	enum class VAOAttribSize : GLint
	{
		zero = 0,
		one = 1,
		two = 2,
		three = 3,
		four = 4,
		bgra = GL_BGRA
	};

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

	// TODO: add glm types recognition
	template <typename cType>
	constexpr inline glType c_to_gl_v = c_to_gl<cType>::value;

	//////////////////////////////////////////////////
	// attribute traits
	/////////////////////////////////////////////////

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
	struct is_tuple : std::false_type {};

	template <class ... T>
	struct is_tuple<std::tuple<T...>> : std::true_type {};

	template <class T>
	constexpr inline bool is_tuple_v = is_tuple<T>();

	template <class T>
	struct is_compound_attr : std::bool_constant<false> {};

	template <class ... T>
	struct is_compound_attr<compound<T...>> : std::bool_constant<(sizeof...(T) > 1)> {};

	// matrices are compound
	template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct is_compound_attr<glm::mat<C, R, T, Q>> : std::bool_constant<true> {};

	template <class T>
	constexpr inline bool is_compound_attr_v = is_compound_attr<T>();
	
	// get n_th attribute type
	template <size_t N, class Tuple, class T = std::tuple_element_t<N, Tuple>>
	struct nth_element
	{
		using type = T;
	};

	// case failure
	template <size_t N, class NotTuple>
	struct nth_element<N, NotTuple, void>
	{
		using type = void;
	};

	template <size_t N, class T, class ... Attribs>
	struct nth_element<N, std::tuple<Attribs...>, T>
	{
		using type = T;
	};

	template <size_t N, class T, const char* name, class ... Attribs>
	struct nth_element<N, std::tuple<Attribs...>, glslt<T, name>>
	{
		using type = T;
	};


	// convinience wrapper to provide just raw pack
	template <size_t N, class ... T>
	using nth_element_t = typename nth_element<N, std::tuple<T...>>::type;


	// get size of elements in a vector for AttributePointer
	template <class>
	struct vao_attrib_size 
		: std::integral_constant<VAOAttribSize, VAOAttribSize::one> {};

	template <glm::length_t L, typename T, glm::qualifier Q>
	struct vao_attrib_size<glm::vec<L, T, Q>> 
		: std::integral_constant<VAOAttribSize, (VAOAttribSize)L> {};
}

