#pragma once

#ifndef _gl_traits_
#define _gl_traits_
#endif // !_gl_traits_

//Testing
#ifdef GL_TRAITS_DLL
#define GLT_API __declspec(dllexport)
//#define GLAD_GLAPI_EXPORT_BUILD
//#define GLAD_GLAPI_EXPORT
#else 
#define GLT_API __declspec(dllimport)
//#define GLAD_GLAPI_EXPORT
#endif // !GL_TRAITS_DLL


//add #ifdef for debug build
#include <iostream>
#define DEBUG_START {
#define DEBUG_END }

#include "glad/glad.h"
/////////


#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_XYZW_ONLY 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


#define POD_EXTENDS std::tuple<glm::vec1, glm::vec2, glm::vec3, glm::vec4, \
                               glm::uvec1, glm::uvec2, glm::uvec3, glm::uvec4 \
>
#include "pod_reflection.hpp"
#include "dhconstexpr_lib.hpp"


#include <thread>
#include <functional>

//debug
#include <iostream>

#include <vector>
#include <array>




#include "traits_common.hpp"

#include "uniform_traits.hpp"
#include "buffer_traits.hpp"
#include "texture_traits.hpp"


struct frameBuffer_traits;          //-
struct programm_traits;             //-
struct programPipelines_traits;     //-
struct queries_traits;              //-
struct renderBuffers_traits;        //-
struct samplers_traits;             //-
class shader_traits;               //-
struct sync_traits;                 //-

struct primitive_traits
{
    enum Mode {
        points = GL_POINTS,
        line_strip = GL_LINE_STRIP,
        line_loop = GL_LINE_LOOP,
        lines = GL_LINES,
        line_strip_adjacency = GL_LINE_STRIP_ADJACENCY,
        lines_adjacency = GL_LINES_ADJACENCY,
        triangle_strip = GL_TRIANGLE_STRIP,
        triangle_fan = GL_TRIANGLE_FAN,
        triangles = GL_TRIANGLES,
        triangle_strip_adjacency = GL_TRIANGLE_STRIP_ADJACENCY,
        triangles_adjacency = GL_TRIANGLES_ADJACENCY,
        patches = GL_PATCHES
    };

    //there's no errors mentioned for negative first, but it's very unlikely to accept negatives anyway
    template <Mode mode>
    static void DrawArrays(size_t first, size_t count)
    {
        glDrawArrays(mode, (GLint)first, (GLsizei)count);
    }

};

/*
struct GLT_API DebugMessageInfo
{
	GLenum source,
		type;
	GLuint id;
	GLenum severity;
	std::string message;
};*/

struct GLT_API gl_debug
{
	static void MessageCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam);
};

/*
template<texture_traits::Target target>
std::array<const gltHandle<target>*, texture_traits::max_gl_textures>
	texture_traits::currentTextures_{};
	*/

struct transformFeedbacks_traits;   //-
struct vertexArrays_traits;          //-






//////////////////////////////////////////////////////////////////////////////////////
//classes 
///////////////////////////////////////////////////////////////////////////////////////

enum class glCapability : int
{
	gl_blend = GL_BLEND,
	gl_clip_distance0 = GL_CLIP_DISTANCE0,
	gl_color_logic_op = GL_COLOR_LOGIC_OP,
	gl_cull_face = GL_CULL_FACE,
	gl_debug_output = GL_DEBUG_OUTPUT,
	gl_debug_output_synchronous = GL_DEBUG_OUTPUT_SYNCHRONOUS,
	gl_depth_clamp = GL_DEPTH_CLAMP,
	gl_depth_test = GL_DEPTH_TEST,
	gl_dither = GL_DITHER,
	gl_framebuffer_srgb = GL_FRAMEBUFFER_SRGB,
	gl_line_smooth = GL_LINE_SMOOTH,
	gl_multisample = GL_MULTISAMPLE,
	gl_polygon_offset_line = GL_POLYGON_OFFSET_LINE,
	gl_polygon_smooth = GL_POLYGON_SMOOTH,
	gl_primitive_restart = GL_PRIMITIVE_RESTART,
	gl_primitive_restart_fixed_index = GL_PRIMITIVE_RESTART_FIXED_INDEX,
	gl_rasterizer_discard = GL_RASTERIZER_DISCARD,
	gl_sample_alpha_to_coverage = GL_SAMPLE_ALPHA_TO_COVERAGE,
	gl_sample_alpha_to_one = GL_SAMPLE_ALPHA_TO_ONE,
	gl_sample_coverage = GL_SAMPLE_COVERAGE,
	gl_sample_shading = GL_SAMPLE_SHADING,
	gl_sample_mask = GL_SAMPLE_MASK,
	gl_scissor_test = GL_SCISSOR_TEST,
	gl_stencil_test = GL_STENCIL_TEST,
	gl_texture_cube_map_seamless = GL_TEXTURE_CUBE_MAP_SEAMLESS,
	gl_program_point_size = GL_PROGRAM_POINT_SIZE
};

struct GLT_API gl_state
{
	template <class ... glCapabilities>
	static void Enable(glCapabilities ... caps)
	{
		static_assert((std::is_same_v<glCapability, glCapabilities> && ...),
			"Only glCapability is accepted");
		//static assert caps to be unique
		(glEnable((GLenum)caps), ...);
	}
};

//shaders

enum class glTargetShader : int
{
	gl_compute_shader = GL_COMPUTE_SHADER,
	gl_vertex_shader = GL_VERTEX_SHADER,
	gl_tess_control_shader = GL_TESS_CONTROL_SHADER,
	gl_tess_evaluation_shader = GL_TESS_EVALUATION_SHADER,
	gl_geometry_shader = GL_GEOMETRY_SHADER,
	gl_fragment_shader = GL_FRAGMENT_SHADER
};

class GLT_API shader_traits
{
	static GLuint GenShaderPrivate(glTargetShader target);
	static bool CompileStatusPrivate(GLuint handle);
	static bool CompileShaderPrivate(GLuint handle, const std::string& source);
	static void AttachShaderPrivate(GLuint prog, GLuint shader);

	static std::string ShaderInfoLogPrivate(GLuint shader);

public:

	static gltHandle<glShaderProgram::program> GenProgram();
	static void LinkProgram(const gltHandle<glShaderProgram::program>& prog);
	static bool LinkStatus(const gltHandle<glShaderProgram::program>& prog);

	template <glTargetShader target>
	static gltHandle<target> GenShader()
	{
		return GenShaderPrivate(target);
	}

	template <glTargetShader target>
	static bool CompileStatus(const gltHandle<target>& handle)
	{
		return CompileStatusPrivate(handle);
	}

	template <glTargetShader target>
	static std::string ShaderInfoLog(const gltHandle<target>& handle)
	{
		return ShaderInfoLogPrivate(handle);
	}


	template <glTargetShader target>
	static bool CompileShader(const gltHandle<target>& handle, const std::string& source)
	{
		return CompileShaderPrivate(handle, source);
	}

	template <glTargetShader target>
	static void AttachShader(const gltHandle<glShaderProgram::program>& prog, const gltHandle<target>& handleShader)
	{
		AttachShaderPrivate(prog, handleShader);
	}

};


template <glTargetShader target>
class gltShader
{
	gltHandle<target> handle_ = shader_traits::GenShader<target>();

	bool compiled_ = false;

public:
	gltShader()
	{}

	gltShader(const std::string& sourceCode)
		: compiled_(shader_traits::CompileShader(handle_, sourceCode))
	{
		if (!compiled_)
		{
			std::string errorMsg = shader_traits::ShaderInfoLog(handle_);
			DEBUG_START
				std::cout << errorMsg << std::endl;
			DEBUG_END
		}
		assert(compiled_ && "Failed to compile shader!");
		
	}

	void Compile(const std::string& source)
	{
		compiled_ = shader_traits::CompileShader(handle_, source);
		if (!compiled_)
		{
			std::string errorMsg = shader_traits::ShaderInfoLog(handle_);
			DEBUG_START
				std::cout << errorMsg << std::endl;
			DEBUG_END
		}
	}

	bool IsValid() const
	{
		return compiled_;
	}

	const gltHandle<target>& Handle() const
	{
		return handle_;
	}

	constexpr operator const gltHandle<target>&() const
	{
		return handle_;
	}
};

template <class UniformCollection>
class gltShaderProgram;

//TODO: add input layout traits
template <class ... unifDescrs>
class gltShaderProgram<gltUniformCollection<std::tuple<unifDescrs...>>>
{
	using glUniforms = gltUniformCollection<std::tuple<unifDescrs...>>;
	gltHandle<glShaderProgram::program> handle_;
	glUniforms uniforms_;

	//TODO: remove in release?
	bool attachedVertex_ = false;
	bool attachedFragment_ = false;

	bool linked_ = false;

public:

	gltShaderProgram()
	{}

	template <glTargetShader ... shaders>
	gltShaderProgram(gltHandle<glShaderProgram::program>&& handle,
		const gltShader<glTargetShader::gl_vertex_shader>& vShader,
		const gltShader<glTargetShader::gl_fragment_shader>& fShader, 
		const gltShader<shaders>& ... otherShaders)
		: handle_(std::move(handle)),
		attachedVertex_(true),
		attachedFragment_(true)
	{
		AttachShaders(vShader, fShader, otherShaders...);
		Link();
	}

	void ResetHandle(gltHandle<glShaderProgram::program>&& handle)
	{
		handle_ = std::move(handle);
	}

	template <glTargetShader ... shTarget>
	void AttachShaders(const gltShader<shTarget>& ... shaders)
	{
		(shader_traits::AttachShader<shTarget>(handle_, shaders), ...);
		attachedVertex_ = true;
		attachedFragment_ = true;
	}

	void Link()
	{
		assert(attachedVertex_ && attachedFragment_ &&
			"At least vertex and fragment shaders must be attached!");
		shader_traits::LinkProgram(handle_);
		linked_ = shader_traits::LinkStatus(handle_);
		uniforms_.InitAll(handle_);
	}

	bool Linked() const
	{
		return linked_;
	}

	const glUniforms& Uniforms() const
	{
		return uniforms_;
	}

};


//#include "glt_definitions.hpp"