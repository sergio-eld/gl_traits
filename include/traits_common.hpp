#pragma once

//common gl traits

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
enum class glTargetBuf : int;
enum class glTargetTex : int;
enum class glTexParamName : int;
enum class glTargetShader : int;
enum class glShaderProgram : int
{
	program
};

//deleters map
typedef cexpr_generic_map<
	cexpr_pair<glTargetBuf, auto_t<&glDeleteBuffers>>,
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
	GLuint handle_;

public:

	using Target = typename decltype(target);

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
		return handle_;
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