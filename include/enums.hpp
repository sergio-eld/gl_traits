#pragma once

// TODO: remove it, use own generator to generate enums directly from specs
#include "glad/glad.h"

namespace glt
{
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

	enum class BufferTarget : GLenum
	{
		none = 0,
		array = GL_ARRAY_BUFFER,
		atomic_counter = GL_ATOMIC_COUNTER_BUFFER,
		copy_read = GL_COPY_READ_BUFFER,
		copy_write = GL_COPY_WRITE_BUFFER,
		dispatch_indirect = GL_DISPATCH_INDIRECT_BUFFER,
		draw_indirect = GL_DRAW_INDIRECT_BUFFER,
		element_array = GL_ELEMENT_ARRAY_BUFFER,
		pixel_pack = GL_PIXEL_PACK_BUFFER,
		pixel_unpack = GL_PIXEL_UNPACK_BUFFER,
		query = GL_QUERY_BUFFER,
		shader_storage = GL_SHADER_STORAGE_BUFFER,
		texture = GL_TEXTURE_BUFFER,
		transform_feedback = GL_TRANSFORM_FEEDBACK_BUFFER,
		uniform = GL_UNIFORM_BUFFER
	};

	using BufferTargetList = std::integer_sequence<BufferTarget,
		BufferTarget::array,
		BufferTarget::atomic_counter,
		BufferTarget::copy_read,
		BufferTarget::copy_write,
		BufferTarget::dispatch_indirect,
		BufferTarget::draw_indirect,
		BufferTarget::element_array,
		BufferTarget::pixel_pack,
		BufferTarget::pixel_unpack,
		BufferTarget::query,
		BufferTarget::shader_storage,
		BufferTarget::texture,
		BufferTarget::transform_feedback,
		BufferTarget::uniform>;


	enum class glBufferBinding : GLenum
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

	enum class FrameBufferTarget : int;

	// texture targets // remove underscore
	enum class TextureTarget : GLenum
	{
		//for glBindTexture
		texture_1d = GL_TEXTURE_1D,							//+
		texture_1d_array = GL_TEXTURE_1D_ARRAY,				//+
		texture_2d = GL_TEXTURE_2D,							//+
		texture_2d_array = GL_TEXTURE_2D_ARRAY,				//+
		texture_2d_multisample = GL_TEXTURE_2D_MULTISAMPLE,				//+
		texture_2d_multisample_array = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,	//+
		texture_3d = GL_TEXTURE_3D,							//+

		texture_rectangle = GL_TEXTURE_RECTANGLE,			//+	
		texture_cube_map = GL_TEXTURE_CUBE_MAP,				//?
		texture_cube_map_array = GL_TEXTURE_CUBE_MAP_ARRAY,	//??
		texture = GL_TEXTURE_BUFFER,					//??


		// separate those below into another enum?
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
		fragment = GL_FRAGMENT_SHADER,

		unknown = std::numeric_limits<int>::max()
	};

	enum class TransformFeedBackTarget : int; // GL_TRANSFORM_FEEDBACK only
	enum class QueryTarget : int;             // used in glBeginQuery

	// types only to be used as a paramaeter for Handle<T>, to find allocator and deleter functions 
	enum class ProgramTarget : int {}; // empty

	enum class VAOTarget : int;       // empty
	enum class ProgramPipeLineTarget : int;   // empty
	enum class RenderBufferTarget : int;      // empty
	enum class SamplerTarget : int;           // empty

	enum class BufUsage : GLenum
	{
        none = 0,

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
        none = 0,

		read_only = GL_READ_ONLY,
		write_only = GL_WRITE_ONLY,
		read_write = GL_READ_WRITE
	};

	enum class MapAccessBit : GLbitfield
	{
        none = 0,

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

}