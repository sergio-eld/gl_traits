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

namespace glt
{
    void LoadOpenGL(void(*loader)(void));
}



