# gl_traits

OpenGL (GLAD) type-safe template wrapper library


Goals:
1. Define strong type enums for OpenGL values (gltEnums.hpp)
2. RAII (Smart Handles) for OpenGL objects.
3. Reduce third-party dependences



Detailed:
1. gltEnums

 - classify OpenGL values into froups
 - provide conversion from c-types to gl types:
 ```
template <typename cType>
constexpr inline int c_to_gl_v = c_to_gl<cType>();
 ```
 
2. RAII (Smart Handles): 
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

| Create | Type | Delete | Type |
| --- | --- | --- | --- |
| glGenBuffers | void(APIENTRY*)(GLsizei, GLuint*)| glDeleteBuffers | void(APIENTRY*)(GLsizei, const GLuint*) |
| glGenTextures | void(APIENTRY*)(GLsizei, GLuint*)| glDeleteTextures | void(APIENTRY*)(GLsizei, const GLuint*) |
| glGenFramebuffers | void(APIENTRY*)(GLsizei, GLuint*) | glDeleteFramebuffers | void(APIENTRY*)(GLsizei, const GLuint*) |
| glGenSamplers| void(APIENTRY*)(GLsizei, GLuint*) | glDeleteSamplers | void(APIENTRY*)(GLsizei, const GLuint*) |
| glGenVertexArrays| void(APIENTRY*)(GLsizei, GLuint*) | glDeleteVertexArrays | void(APIENTRY*)(GLsizei, const GLuint*) |
| glGenRenderbuffers| void(APIENTRY*)(GLsizei, GLuint*) | glDeleteRenderbuffers | void(APIENTRY*)(GLsizei, const GLuint*) |
| glGenProgramPipelines| void(APIENTRY*)(GLsizei, GLuint*) | glDeleteProgramPipelines | void(APIENTRY*)(GLsizei, const GLuint*) |
| glGenQueries| void(APIENTRY*)(GLsizei, GLuint*) | glDeleteQueries | void(APIENTRY*)(GLsizei, const GLuint*) |
| glGenTransformFeedbacks| void(APIENTRY*)(GLsizei, GLuint*) | glDeleteTransformFeedbacks | void(APIENTRY*)(GLsizei, const GLuint*) |
| glCreateShader| GLuint(APIENTRY*)(GLenum)| glDeleteProgram | void(APIENTRY*)(GLuint) |
| glCreateProgram| GLuint(APIENTRY*)(void) | glDeleteShader | void(APIENTRY*)(GLuint) |


Enum classes to be used as template arguments (and to lookup gen and delete function pointers):
TODO: naming convention for enums.
Some enums are declared only to be used as template parameters and have no defined values
| Create | Enum Typename |
| --- | --- |
| glGenBuffers | glBufferTarget |
| glGenTextures | glTextureTarget |
| glGenFramebuffers | glFrameBufferTarget |
| glGenSamplers| glSamplerTarget |
| glGenVertexArrays| glVertexArrayTarget |
| glGenRenderbuffers| glRenderBufferTarget |
| glGenProgramPipelines| glProgramPipeLineTarget |
| glGenQueries| glQueryTarget |
| glGenTransformFeedbacks| glTransformFeedBackTarget |
| glCreateShader| glShaderTarget |
| glCreateProgram| glProgramTarget |

### Current third-perty dependences:
 - GLAD
 - GLM


## Buffer traits
###Sequence.
A ***Sequence*** is a strong-typed array of data contained within an OpenGL ***Buffer***. An Element of a ***Sequence*** may consist of a single-type variable (*Batched* or *Tightly-packed* Sequence), or of a multi-type struct-like variable (*Compound* Sequence).


###Buffer
Each ***Buffer*** stores one or several sequences. 