#pragma once

#ifndef _gl_traits_
#define _gl_traits_
#endif // !_gl_traits_


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

#include "glad/glad.h"

/*
*/

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
struct shader_traits;               //-
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
template<texture_traits::Target target>
std::array<const gltHandle<target>*, texture_traits::max_gl_textures>
	texture_traits::currentTextures_{};
	*/

struct transformFeedbacks_traits;   //-
struct vertexArrays_traits;          //-






//////////////////////////////////////////////////////////////////////////////////////
//classes 
///////////////////////////////////////////////////////////////////////////////////////


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


struct shader_traits
{
	template <glTargetShader target>
	static gltHandle<target> GenShader()
	{
		return glCreateShader((GLenum)target);
	}

	static gltHandle<glShaderProgram::program> GenProgram()
	{
		return glCreateProgram();
	}

	template <glTargetShader target>
	static bool CompileStatus(const gltHandle<target>& handle)
	{
		int success;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
		return (bool)success;
	}

	static bool LinkStatus(const gltHandle<glShaderProgram::program>& prog)
	{
		int success = 0;
		glGetProgramiv(prog, GL_LINK_STATUS, &success);
		return (bool)success;
	}

	template <glTargetShader target>
	static bool CompileShader(const gltHandle<target>& handle, const std::string& source)
	{
		const char* c_src = source.c_str();
		glShaderSource(handle, 1, &c_src, 0);
		glCompileShader(handle);

		return CompileStatus(handle);
	}

	template <glTargetShader target>
	static void AttachShader(const gltHandle<glShaderProgram::program>& prog, const gltHandle<target>& handleShader)
	{
		glAttachShader(prog, handleShader);
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
		assert(compiled_ && "Failed to compile shader!");
	}

	void Compile(const std::string& source)
	{
		compiled_ = shader_traits::CompileShader(handle_, source);
		assert(compiled_ && "Failed to compile shader!");
	}

	bool IsValid() const
	{
		return compiled_;
	}

};


//TODO: add input layout traits
template <class UniformCollection>
class gltShaderProgram
{
	gltHandle<glShaderProgram::program> handle_ = shader_traits::GenProgram();


	//TODO: remove in release?
	bool attachedVertex_ = false;
	bool attachedFragment_ = false;

	bool linked_ = false;

public:

	gltShaderProgram()
	{}

	template <glTargetShader ... shaders>
	gltShaderProgram(const gltShader<glTargetShader::gl_vertex_shader>& vShader,
		const gltShader<glTargetShader::gl_fragment_shader>& fShader, 
		const gltShader<shaders>& ... otherShaders)
		: attachedVertex_(true),
		attachedFragment_(true)
	{
		AttachShaders(vShader, fShader, otherShaders...);
		linked_ = shader_traits::LinkStatus(handle_);
	}

	template <glTargetShader ... shaders>
	void AttachShaders(const gltShader<shaders>& ... shaders)
	{
		(shader_traits::AttachShader(handle_, shaders), ...);
	}

};
