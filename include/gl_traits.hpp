#pragma once

/*
This library is intended to provide typesafety for OpenGL functions and enumerators.
Typesafe enum classes and function wrappers allow to eliminate at compile-time the 
following errors:
- 
- 

Additional wrapper classes enforce a workflow that makes easier to avoid or detect
the following run-time errors:
-
-


*/

#ifndef _gl_traits_
#define _gl_traits_
#endif // !_gl_traits_

//Testing

#ifndef GL_TRAITS_EXPORT
#define GLT_API
#endif


//add #ifdef for debug build
#include <iostream>
#define DEBUG_START {
#define DEBUG_END }

//#include "glad/glad.h"
/////////

/////////////////////////
//restructurised
/////////////////////////

#include "gltEnums.hpp"     // includes glad
#include "gltHandle.hpp"    // includes gltEnums

/////////////////////////


//#include "dhconstexpr_lib.hpp"

#include <thread>
#include <functional>

//debug
#include <iostream>

#include <vector>
#include <array>

//#include "traits_common.hpp"

//#include "uniform_traits.hpp"
//#include "buffer_traits.hpp"
//#include "texture_traits.hpp"


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


struct transformFeedbacks_traits;   //-
struct vertexArrays_traits;          //-






//////////////////////////////////////////////////////////////////////////////////////
//classes 
///////////////////////////////////////////////////////////////////////////////////////



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


/*
class GLT_API shader_traits
{
	static GLuint GenShaderPrivate(glShaderTarget target);
	static bool CompileStatusPrivate(GLuint handle);
	static bool CompileShaderPrivate(GLuint handle, const std::string& source);
	static void AttachShaderPrivate(GLuint prog, GLuint shader);

	static std::string ShaderInfoLogPrivate(GLuint shader);

public:

	static gltHandle<glProgramTarget::program> GenProgram();
	static void LinkProgram(const gltHandle<glProgramTarget::program>& prog);
	static bool LinkStatus(const gltHandle<glProgramTarget::program>& prog);

	template <glShaderTarget target>
	static gltHandle<target> GenShader()
	{
		return GenShaderPrivate(target);
	}

	template <glShaderTarget target>
	static bool CompileStatus(const gltHandle<target>& handle)
	{
		return CompileStatusPrivate(handle);
	}

	template <glShaderTarget target>
	static std::string ShaderInfoLog(const gltHandle<target>& handle)
	{
		return ShaderInfoLogPrivate(handle);
	}


	template <glShaderTarget target>
	static bool CompileShader(const gltHandle<target>& handle, const std::string& source)
	{
		return CompileShaderPrivate(handle, source);
	}

	template <glShaderTarget target>
	static void AttachShader(const gltHandle<glProgramTarget::program>& prog, const gltHandle<target>& handleShader)
	{
		AttachShaderPrivate(prog, handleShader);
	}

};


template <glShaderTarget target>
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
	gltHandle<glProgramTarget::program> handle_;
	glUniforms uniforms_;

	//TODO: remove in release?
	bool attachedVertex_ = false;
	bool attachedFragment_ = false;

	bool linked_ = false;

public:

	gltShaderProgram()
	{}

	template <glShaderTarget ... shaders>
	gltShaderProgram(gltHandle<glProgramTarget::program>&& handle,
		const gltShader<glShaderTarget::gl_vertex_shader>& vShader,
		const gltShader<glShaderTarget::gl_fragment_shader>& fShader, 
		const gltShader<shaders>& ... otherShaders)
		: handle_(std::move(handle)),
		attachedVertex_(true),
		attachedFragment_(true)
	{
		AttachShaders(vShader, fShader, otherShaders...);
		Link();
	}

	void ResetHandle(gltHandle<glProgramTarget::program>&& handle)
	{
		handle_ = std::move(handle);
	}

	template <glShaderTarget ... shTarget>
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

*/
//#include "glt_definitions.hpp"