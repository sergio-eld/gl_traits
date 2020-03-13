#pragma once


#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_XYZW_ONLY 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define POD_EXTENDS std::tuple<glm::vec1, glm::vec2, glm::vec3, glm::vec4, \
                               glm::uvec1, glm::uvec2, glm::uvec3, glm::uvec4 \
>
#include "pod_reflection.hpp"

//common gl traits

// glm type deduction

template <typename type>
struct glm_traits
{
	constexpr static bool is_glm = false;
	using Type = typename type;
};

template <template <glm::length_t, typename, glm::qualifier> class glm_type, 
	glm::length_t L, typename T, glm::qualifier Q>
	struct glm_traits<glm_type<L, T, Q>>
{
	constexpr static bool is_glm = true;
	using Type = typename T;
};

template <template <glm::length_t, glm::length_t, typename, glm::qualifier> class glm_type,
	glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct glm_traits<glm_type<C, R, T, Q>>
{
	constexpr static bool is_glm = true;
	using Type = typename T;
};

template <class>
struct all_glm : public std::bool_constant<false>
{};

template <class ... params>
struct all_glm<std::tuple<params...>> : public std::bool_constant<(glm_traits<params>::is_glm && ...)>
{};

template <class T>
constexpr inline bool all_glm_v = all_glm<T>::value;


//These wrappers serve to enforse usage of predefined parameters in functions that accept only curtain values
/*
struct gl_none : public gl_value<GL_NONE> {};
struct gl_lequal : public gl_value<GL_LEQUAL> {};
struct gl_gequal : public gl_value<GL_GEQUAL> {};
struct gl_less : public gl_value<GL_LESS> {};
struct gl_greater : public gl_value<GL_GREATER> {};
struct gl_equal : public gl_value<GL_EQUAL> {};
struct gl_notequal : public gl_value<GL_NOTEQUAL> {};
struct gl_always : public gl_value<GL_ALWAYS> {};
struct gl_never : public gl_value<GL_NEVER> {};
*/