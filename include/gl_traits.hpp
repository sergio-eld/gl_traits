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


