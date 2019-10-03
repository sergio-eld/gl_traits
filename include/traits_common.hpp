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

//TODO: add glm types
//gl types conversion
typedef cexpr_generic_map<
	cexpr_pair<GLbyte, auto_t<GL_BYTE>>,
	cexpr_pair<GLubyte, auto_t<GL_UNSIGNED_BYTE>>,
	cexpr_pair<GLshort, auto_t<GL_SHORT>>,
	cexpr_pair<GLushort, auto_t<GL_UNSIGNED_SHORT>>,
	cexpr_pair<GLint, auto_t<GL_INT>>,
	cexpr_pair<GLuint, auto_t<GL_UNSIGNED_INT>>,
	cexpr_pair<GLfixed, auto_t<GL_FIXED>>,
	cexpr_pair<GLhalf, auto_t<GL_HALF_FLOAT>>,
	cexpr_pair<GLfloat, auto_t<GL_FLOAT>>,
	cexpr_pair<GLdouble, auto_t<GL_DOUBLE>>

>
gl_types_map;


template <typename key_cType>
struct glenum_from_type 
	: public std::integral_constant<GLenum, gl_types_map::found_pair<key_cType>::value::value>
{
	constexpr static size_t nComponents = 1;
	using KType = typename key_cType;
};

template <template <glm::length_t, typename, glm::qualifier> class glm_vec,
	glm::length_t L, typename T, glm::qualifier Q>
struct glenum_from_type<glm_vec<L, T, Q>> 
	: public std::integral_constant<GLenum, gl_types_map::found_pair<T>::value::value>
{
	constexpr static size_t nComponents = L;
	using KType = typename glm_vec<L, T, Q>;
};

template <template <glm::length_t, glm::length_t, typename, glm::qualifier> class glm_mat,
	glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct glenum_from_type<glm_mat<C, R, T, Q>>
	: public std::integral_constant<GLenum, gl_types_map::found_pair<T>::value::value>
{
	constexpr static size_t nComponents = C;
	using KType = typename glm_mat<C, R, T, Q>;
};

template <class T>
constexpr inline GLenum glenum_from_type_v = glenum_from_type<T>();

template <auto gl_val>
using gl_value = std::integral_constant<decltype(gl_val), gl_val>;

//These wrappers serve to enforse usage of predefined parameters in functions that accept only curtain values
struct gl_none : public gl_value<GL_NONE> {};
struct gl_lequal : public gl_value<GL_LEQUAL> {};
struct gl_gequal : public gl_value<GL_GEQUAL> {};
struct gl_less : public gl_value<GL_LESS> {};
struct gl_greater : public gl_value<GL_GREATER> {};
struct gl_equal : public gl_value<GL_EQUAL> {};
struct gl_notequal : public gl_value<GL_NOTEQUAL> {};
struct gl_always : public gl_value<GL_ALWAYS> {};
struct gl_never : public gl_value<GL_NEVER> {};

//targets forward declaration
enum class glTargetBuf : unsigned int;
enum class glTargetTex : int;
enum class glTexParamName : int;
enum class glTargetShader : int;
enum class glShaderProgram : int
{
	program
};
enum class glVAO : int
{
    vao
};

//deleters map
typedef cexpr_generic_map<
	cexpr_pair<glTargetBuf, auto_t<&glDeleteBuffers>>,
    cexpr_pair<glVAO, auto_t<&glDeleteVertexArrays>>,
	// cexpr_pair<target_enum<frameBuffer_traits>,          auto_t<&glDeleteFramebuffers>>,
	cexpr_pair<glShaderProgram, auto_t<&glDeleteProgram>>,
	// cexpr_pair<target_enum<programPipelines_traits>,     auto_t<&glDeleteProgramPipelines>>,
	// cexpr_pair<target_enum<renderBuffers_traits>,        auto_t<&glDeleteRenderbuffers>>,
	//  cexpr_pair<target_enum<samplers_traits>,             auto_t<&glDeleteSamplers>>,
	cexpr_pair<glTargetShader, auto_t<&glDeleteShader>>,
	// cexpr_pair<target_enum<sync_traits>,                 auto_t<&glDeleteSync>>,
	cexpr_pair<glTargetTex, auto_t<&glDeleteTextures>>
	//  cexpr_pair<target_enum<transformFeedbacks_traits>,   auto_t<&glDeleteTransformFeedbacks>>,
	//  cexpr_pair<target_enum<vertexArrays_traits>,         auto_t<&glDeleteBuffers>>
> gl_deleters;


template <auto target, class = typename decltype(target)>
class gltHandle;

template <auto target>
class gltHandle<target, typename decltype(target)>
{
	//TODO: add Glsync handle type
	GLuint handle_ = 0;

public:

	using Target = typename decltype(target);

	gltHandle() = default;
	gltHandle(GLuint handle)
		: handle_(handle)
	{
		//ownership?
		//handle = 0;
	}

	gltHandle(const gltHandle<target>& other) = delete;
	gltHandle<target>& operator=(const gltHandle<target>& other) = delete;

	gltHandle(gltHandle<target>&& other)
		: handle_(other.handle_)
	{
		other.handle_ = 0;
	}
	gltHandle<target>& operator=(gltHandle<target>&& other)
	{
		if (handle_)
			DestroyHandle();
		handle_ = other.handle_;
		other.handle_ = 0;
		return *this;
	}

	bool operator==(const gltHandle<target>& other) const
	{
		return handle_ == other.handle_;
	}

	bool operator!=(const gltHandle<target>& other) const
	{
		return !operator==(other);
	}

	bool IsValid() const
	{
		return (bool)handle_;
	}

	bool operator!() const
	{
		return !IsValid();
	}

	constexpr operator GLuint() const
	{
		return handle_;
	}

	~gltHandle()
	{
		if (handle_)
			DestroyHandle();
	}

private:
	void DestroyHandle()
	{
		constexpr auto pDestroy =
			gl_deleters::found_pair<decltype(target)>::value::value;
		constexpr size_t args = func_traits<std::remove_pointer_t<decltype(pDestroy)>(nullptr)>::args_count;
		if constexpr (args == 1)
			(*pDestroy)(handle_);
		else 
			(*pDestroy)(1, &handle_);

	}
};