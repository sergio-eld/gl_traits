#pragma once

/*
This library is intended to provide typesafety for OpenGL functions and enumerators.
Typesafe enum classes and function wrappers allow to eliminate at compile-time the 
following errors:
- 
- Accessing/Modifying Uniforms that do not exist within a shader program
- restrict range of possible values for input, i.e. 
glVertexAttribPointer size may be 1, 2, 3, 4, or GL_BGRA. Declaring enum will restrict
implicit conversions from other integer values
- invalid input for glVertexAttribPointer


Additional wrapper classes enforce a workflow that makes easier to avoid or detect
the following run-time errors:
- memory leaks (enforce RAII)
-

Each class should have 2 levels of indirection:
0. TypeSafe: no runtime checks
1. With runtime checks: i.e. ensure that object which functions are being invoked
is currently active or bound if required by version of OpenGL specification

There are properties for variables (in/out/uniform/blocks/etc) in glsl shader 
source code that may be defined explicitly (like location or binding) in the source
or omitted. In the latter case they may be set dinamically by user.
Need to implement algorithm selection based on template parameters:
For example: if VAO template arguments are not defined with locations, a run-time
map will be used to bind and validate its indexes.
For such a case may also provide an explicict parameter, that will enforce
location indexes according to the order template arguments had been specified.

TODO: analize possible VertexArrayPointer assignment routines
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

#include "basic_types.hpp"

#include "buffer_traits.hpp"
#include "shader_traits.hpp"
#include "program_traits.hpp"

// ??
// sequence
// uniform
// program
#include "Texture.hpp"


/////////////////////////

#include <thread>
#include <functional>

//debug
#include <iostream>

#include <vector>
#include <array>



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

