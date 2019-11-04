# gl_traits

OpenGL (GLAD) type-safe template wrapper library


Goals:
1. Define strong type enums for OpenGL values (gltEnums.hpp)
2. Creating Smart Handles for OpenGL objects.
3. Reduce third-party dependences



Detailed:
1. gltEnums
 - classify OpenGL values into froups
 - provide conversion from c-types to gl types:
 ```
template <typename cType>
constexpr inline int c_to_gl_v = c_to_gl<cType>();
 ```
 
 2. Smart Handles: 
 - individual header?
 - one template class for each OpenGL object type
 - Template specializations using enum classes, example:
 
 ```
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
```
 - Template specialization for OpenGL object deleter. Default parameter, deduced from enum type
 - Restrict multiple ownership?
 - Restrict default constructor?
2. Current third-perty dependences:
 - GLAD
 - GLM
 - pod_reflection
 - dh_constexpr lib (using cexpr_generic_map)