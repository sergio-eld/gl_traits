#pragma once

#include <cassert>

#include "glad/glad.h"

#include "glm/glm.hpp"

#include <type_traits>

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

template <auto A>
using glt_constant = std::integral_constant<decltype(A), A>;

enum class glBufferTarget : int;
enum class glFrameBufferTarget : int;
enum class glTextureTarget : int;
enum class glShaderTarget : int;

enum class glTransformFeedBackTarget : int; // GL_TRANSFORM_FEEDBACK only
enum class glQueryTarget : int;             // used in glBeginQuery


// types only to be used as a paramaeter for gltHandle<T>, to find allocator and deleter functions 
enum class glProgramTarget : int;           // empty

enum class glVertexArrayTarget : int;       // empty
enum class glProgramPipeLineTarget : int;   // empty
enum class glRenderBufferTarget : int;      // empty
enum class glSamplerTarget : int;           // empty

// map for gl allocators
template <typename glObjType>
struct pp_gl_allocator;

template <> struct pp_gl_allocator<glBufferTarget> :            glt_constant<&glGenBuffers> {};
template <> struct pp_gl_allocator<glFrameBufferTarget> :       glt_constant<&glGenFramebuffers> {};
template <> struct pp_gl_allocator<glTextureTarget> :           glt_constant<&glGenTextures> {};
template <> struct pp_gl_allocator<glVertexArrayTarget> :       glt_constant<&glGenVertexArrays> {};

template <> struct pp_gl_allocator<glTransformFeedBackTarget> : glt_constant<&glGenTransformFeedbacks> {};
template <> struct pp_gl_allocator<glQueryTarget> :             glt_constant<&glGenQueries> {};

template <> struct pp_gl_allocator<glProgramPipeLineTarget> :   glt_constant<&glGenProgramPipelines> {};
template <> struct pp_gl_allocator<glRenderBufferTarget> :      glt_constant<&glGenRenderbuffers> {};
template <> struct pp_gl_allocator<glSamplerTarget> :           glt_constant<&glGenSamplers> {};

template <> struct pp_gl_allocator<glProgramTarget> :           glt_constant<&glCreateProgram> {};

// takes argument 
template <> struct pp_gl_allocator<glShaderTarget> :            glt_constant<&glCreateShader> {};

template <typename glObjType>
constexpr inline auto pp_gl_allocator_v = pp_gl_allocator<glObjType>::value;

// map for gl deleters
template <typename glObjType>
struct pp_gl_deleter;

template <> struct pp_gl_deleter<glBufferTarget> :              glt_constant<&glDeleteBuffers> {};
template <> struct pp_gl_deleter<glFrameBufferTarget> :         glt_constant<&glDeleteFramebuffers> {};
template <> struct pp_gl_deleter<glTextureTarget> :             glt_constant<&glDeleteTextures> {};
template <> struct pp_gl_deleter<glVertexArrayTarget> :         glt_constant<&glDeleteVertexArrays> {};

template <> struct pp_gl_deleter<glTransformFeedBackTarget> :   glt_constant<&glDeleteTransformFeedbacks> {};
template <> struct pp_gl_deleter<glQueryTarget> :               glt_constant<&glDeleteQueries> {};

template <> struct pp_gl_deleter<glProgramPipeLineTarget> :     glt_constant<&glDeleteProgramPipelines> {};
template <> struct pp_gl_deleter<glRenderBufferTarget> :        glt_constant<&glDeleteRenderbuffers> {};
template <> struct pp_gl_deleter<glSamplerTarget> :             glt_constant<&glDeleteSamplers> {};


template <> struct pp_gl_deleter<glShaderTarget> :              glt_constant<&glDeleteShader> {};
template <> struct pp_gl_deleter<glProgramTarget> :             glt_constant<&glDeleteProgram> {};

template <typename glObjType>
constexpr inline auto pp_gl_deleter_v = pp_gl_deleter<glObjType>::value;

template <typename glObjType>
constexpr inline bool has_func_bind_v = std::bool_constant<std::disjunction_v<
    std::is_same<glObjType, glBufferTarget>,
    std::is_same<glObjType, glFrameBufferTarget>,
    std::is_same<glObjType, glTextureTarget>,
    std::is_same<glObjType, glVertexArrayTarget>,
    std::is_same<glObjType, glTransformFeedBackTarget>
    >>::value;

// map for binding functions
template <typename glObjType>
struct pp_gl_binder;

template <> struct pp_gl_binder<glBufferTarget> :               glt_constant<&glBindBuffer> {};
template <> struct pp_gl_binder<glFrameBufferTarget> :          glt_constant<&glBindFramebuffer> {};
template <> struct pp_gl_binder<glTextureTarget> :              glt_constant<&glBindTexture> {};
template <> struct pp_gl_binder<glTransformFeedBackTarget> :    glt_constant<&glBindTransformFeedback> {};

template <> struct pp_gl_binder<glVertexArrayTarget> :          glt_constant<&glBindVertexArray> {};

template <typename glObjType>
constexpr inline auto pp_gl_binder_v = pp_gl_binder<glObjType>::value;

// buffer targets // remove underscore later
enum class glBufferTarget : int
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

using glBufferTargetList = std::integer_sequence<glBufferTarget,
    glBufferTarget::array_buffer,
    glBufferTarget::atomic_counter_buffer,
    glBufferTarget::copy_read_buffer,
    glBufferTarget::copy_write_buffer,
    glBufferTarget::dispatch_indirect_buffer,
    glBufferTarget::draw_indirect_buffer,
    glBufferTarget::element_array_buffer,
    glBufferTarget::pixel_pack_buffer,
    glBufferTarget::pixel_unpack_buffer,
    glBufferTarget::query_buffer,
    glBufferTarget::shader_storage_buffer,
    glBufferTarget::texture_buffer,
    glBufferTarget::transform_feedback_buffer,
    glBufferTarget::uniform_buffer>;


enum class glBufferBinding : int
{
    array_buffer_binding =              GL_ARRAY_BUFFER_BINDING,
    atomic_buffer_binding =             GL_ATOMIC_COUNTER_BUFFER_BINDING,
    copy_read_buffer_binding =          GL_COPY_READ_BUFFER_BINDING,
    copy_write_buffer_binding =         GL_COPY_WRITE_BUFFER_BINDING,
    draw_indirect_buffer_binding =      GL_DRAW_INDIRECT_BUFFER_BINDING,
    dispatch_indirect_buffer_binding =  GL_DISPATCH_INDIRECT_BUFFER_BINDING,
    element_array_buffer_binding =      GL_ELEMENT_ARRAY_BUFFER_BINDING,
    pixel_pack_buffer_binding =         GL_PIXEL_PACK_BUFFER_BINDING,
    pixel_unpack_buffer_binding =       GL_PIXEL_UNPACK_BUFFER_BINDING,
    shader_storage_buffer_binding =     GL_SHADER_STORAGE_BUFFER_BINDING,
    transform_feedback_buffer_binding = GL_TRANSFORM_FEEDBACK_BUFFER_BINDING,
    uniform_buffer_binding =            GL_UNIFORM_BUFFER_BINDING
};

template <auto>
struct get_binding : std::integral_constant<void*, 0> {};

template <auto A>
constexpr inline bool has_gl_binding_v = !std::is_same_v<get_binding<A>::type, void*>;

// TODO: generate this maps using python
template <> struct get_binding<glBufferTarget::array_buffer> :              glt_constant<glBufferBinding::array_buffer_binding>{};
template <> struct get_binding<glBufferTarget::atomic_counter_buffer> :     glt_constant<glBufferBinding::atomic_buffer_binding>{};
template <> struct get_binding<glBufferTarget::copy_read_buffer> :          glt_constant<glBufferBinding::copy_read_buffer_binding>{};
template <> struct get_binding<glBufferTarget::copy_write_buffer> :         glt_constant<glBufferBinding::copy_write_buffer_binding>{};
template <> struct get_binding<glBufferTarget::dispatch_indirect_buffer> :  glt_constant<glBufferBinding::dispatch_indirect_buffer_binding>{};
template <> struct get_binding<glBufferTarget::draw_indirect_buffer> :      glt_constant<glBufferBinding::draw_indirect_buffer_binding>{};
template <> struct get_binding<glBufferTarget::element_array_buffer> :      glt_constant<glBufferBinding::element_array_buffer_binding>{};
template <> struct get_binding<glBufferTarget::pixel_pack_buffer> :         glt_constant<glBufferBinding::pixel_pack_buffer_binding>{};
template <> struct get_binding<glBufferTarget::pixel_unpack_buffer> :       glt_constant<glBufferBinding::pixel_unpack_buffer_binding>{};
template <> struct get_binding<glBufferTarget::shader_storage_buffer> :     glt_constant<glBufferBinding::shader_storage_buffer_binding>{};
template <> struct get_binding<glBufferTarget::transform_feedback_buffer> : glt_constant<glBufferBinding::transform_feedback_buffer_binding>{};
template <> struct get_binding<glBufferTarget::uniform_buffer> :            glt_constant<glBufferBinding::uniform_buffer_binding>{};

template <auto A>
constexpr inline auto get_binding_v = get_binding<A>::value;

enum class glBufUse : int
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
enum class glTextureTarget : int
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


enum class glShaderTarget : int
{
    compute_shader = GL_COMPUTE_SHADER,
    vertex_shader = GL_VERTEX_SHADER,
    tess_control_shader = GL_TESS_CONTROL_SHADER,
    tess_evaluation_shader = GL_TESS_EVALUATION_SHADER,
    geometry_shader = GL_GEOMETRY_SHADER,
    fragment_shader = GL_FRAGMENT_SHADER
};


// glEnable arguments
enum class glCapability : int
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

template <typename cType>
struct c_to_gl;

template <> struct c_to_gl<GLbyte> :    glt_constant<glType::gl_byte> {};
template <> struct c_to_gl<GLubyte> :   glt_constant<glType::gl_unsigned_byte> {};
template <> struct c_to_gl<GLshort> :   glt_constant<glType::gl_short> {};
template <> struct c_to_gl<GLushort> :  glt_constant<glType::gl_unsigned_short> {};
template <> struct c_to_gl<GLint> :     glt_constant<glType::gl_int> {};
template <> struct c_to_gl<GLuint> :    glt_constant<glType::gl_unsigned_int> {};
//template <> struct c_to_gl<GLfixed> : std::integral_constant<int, GL_FIXED> {};
//template <> struct c_to_gl<GLhalf> :  std::integral_constant<int, GL_HALF_FLOAT> {};
template <> struct c_to_gl<GLfloat> :   glt_constant<glType::gl_float> {};
template <> struct c_to_gl<GLdouble> :  glt_constant<glType::gl_double> {};

// TODO: add glm types recognition
template <typename cType>
constexpr inline glType c_to_gl_v = c_to_gl<cType>::value;


