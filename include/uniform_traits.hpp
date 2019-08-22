#pragma once

////////////////////////////////////////////////////////////////
//glUniform Wrappers
////////////////////////////////////////////////////////////////

//struct to describe glUniform with 1 - 4 arguments
template <typename cType, size_t elem, typename tuple = typename gen_tuple<cType, elem>::tuple>
struct unif_info;

struct UnifType
{
	enum type { def, vect, matrx };
};

typedef cexpr_generic_map <
	cexpr_pair<unif_info<float, 1>, auto_t<&glUniform1f>>,
	cexpr_pair<unif_info<float, 2>, auto_t<&glUniform2f>>,
	cexpr_pair<unif_info<float, 3>, auto_t<&glUniform3f>>,
	cexpr_pair<unif_info<float, 4>, auto_t<&glUniform4f>>,

	cexpr_pair<unif_info<int, 1>, auto_t<&glUniform1i>>,
	cexpr_pair<unif_info<int, 2>, auto_t<&glUniform2i>>,
	cexpr_pair<unif_info<int, 3>, auto_t<&glUniform3i>>,
	cexpr_pair<unif_info<int, 4>, auto_t<&glUniform4i>>,

	cexpr_pair<unif_info<unsigned int, 1>, auto_t<&glUniform1ui>>,
	cexpr_pair<unif_info<unsigned int, 2>, auto_t<&glUniform2ui>>,
	cexpr_pair<unif_info<unsigned int, 3>, auto_t<&glUniform3ui>>,
	cexpr_pair<unif_info<unsigned int, 4>, auto_t<&glUniform4ui>>,

	cexpr_pair<glm::vec1, auto_t<&glUniform1fv>>,
	cexpr_pair<glm::vec2, auto_t<&glUniform2fv>>,
	cexpr_pair<glm::vec3, auto_t<&glUniform3fv>>,
	cexpr_pair<glm::vec4, auto_t<&glUniform4fv>>,

	cexpr_pair<glm::ivec1, auto_t<&glUniform1iv>>,
	cexpr_pair<glm::ivec2, auto_t<&glUniform2iv>>,
	cexpr_pair<glm::ivec3, auto_t<&glUniform3iv>>,
	cexpr_pair<glm::ivec4, auto_t<&glUniform4iv>>,

	cexpr_pair<glm::uvec1, auto_t<&glUniform1uiv>>,
	cexpr_pair<glm::uvec2, auto_t<&glUniform2uiv>>,
	cexpr_pair<glm::uvec3, auto_t<&glUniform3uiv>>,
	cexpr_pair<glm::uvec4, auto_t<&glUniform4uiv>>,

	cexpr_pair<glm::mat2, auto_t<&glUniformMatrix2fv>>,
	cexpr_pair<glm::mat3, auto_t<&glUniformMatrix3fv>>,
	cexpr_pair<glm::mat4, auto_t<&glUniformMatrix4fv>>,

	cexpr_pair<glm::mat2x3, auto_t<&glUniformMatrix2x3fv>>,
	cexpr_pair<glm::mat3x2, auto_t<&glUniformMatrix3x2fv>>,

	cexpr_pair<glm::mat2x4, auto_t<&glUniformMatrix2x4fv>>,
	cexpr_pair<glm::mat4x2, auto_t<&glUniformMatrix4x2fv>>,

	cexpr_pair<glm::mat4x3, auto_t<&glUniformMatrix4x3fv>>,
	cexpr_pair<glm::mat3x4, auto_t<&glUniformMatrix3x4fv>>

> glUniformMap;


//primary
template <const char* uniformName, typename gltype>
struct gltUnifDescr;

//regular
template <const char* uniformName, typename cType, size_t elems>
struct gltUnifDescr<uniformName, unif_info<cType, elems>>
{
	constexpr static const char* name = uniformName;
	typedef cType type;
	constexpr static size_t count = elems;
	constexpr static UnifType::type glm_type = UnifType::def;
};

//vectors
template <const char* uniformName,
	glm::length_t elems, typename cType, glm::qualifier Q>
	struct gltUnifDescr<uniformName, glm::vec<elems, cType, Q>>
{
	constexpr static const char* name = uniformName;
	typedef cType type;
	constexpr static size_t count = elems;
	constexpr static UnifType::type glm_type = UnifType::vect;
};

//matrices
template <const char* uniformName,
	glm::length_t C, glm::length_t R, typename cType, glm::qualifier Q>
	struct gltUnifDescr<uniformName, glm::mat<C, R, cType, Q>>
{
	constexpr static const char* name = uniformName;
	typedef cType type;
	constexpr static size_t columns = C,
		rows = R;
	constexpr static UnifType::type glm_type = UnifType::matrx;
};

template <class>
class gltUniform_impl;

template <const char* unifromName, typename uniformSpec>
class gltUniform_impl<gltUnifDescr<unifromName, uniformSpec>>;


/*Here a trick is used: tuple in unif_info is generated as a default
argument (tuple with "elems" number of cType template arguments)*/
//specialization for regular uniform taking 1 to 4 arguments of type T
template <const char* uniformName, typename cType, size_t elems, typename ... cTypes>
class gltUniform_impl<gltUnifDescr<uniformName, unif_info<cType, elems, std::tuple<cTypes ...>>>>
{
	GLint handle_ = -1;

public:

	void Init(char_t<uniformName>&&, GLuint handleShaderProg)
	{
		//uniform name has to be a null terminated string
		handle_ = glGetUniformLocation(handleShaderProg, uniformName);

		//this is critical
		assert(handle_ != -1 && "Failed to get uniform location!");
	}

	void UpdateUniform(char_t<uniformName>&&, cTypes ... args)
	{
		// find function pointer in the generic map
		constexpr auto pUnifFunc = glUniformMap::found_pair<unif_info<cType, sizeof...(cTypes)>>::value::value;
		(*pUnifFunc)(handle_, args...);
	}

};

//specialization for vectors
template <const char* uniformName, glm::length_t elems, typename cType, glm::qualifier Q>
class gltUniform_impl<gltUnifDescr<uniformName, glm::vec<elems, cType, Q>>>
{
	GLint handle_ = -1;
public:

	void Init(char_t<uniformName>&&, GLuint handleShaderProg)
	{
		//uniform name has to be a null terminated string
		handle_ = glGetUniformLocation(handleShaderProg, uniformName);

		//this is critical
		assert(handle_ != -1 && "Failed to get uniform location!");
	}

	//did not check. But assume that it works
	void UpdateUniform(char_t<uniformName>&&, const glm::vec<elems, cType, Q>& vec)
	{
		constexpr //void(__stdcall *pUnifFunc)(GLint, GLsizei, GLboolean, const cType*) =
			auto pUnifFunc =
			glUniformMap::found_pair<glm::vec<elems, cType, Q>>::value::value;
		(*pUnifFunc)(handle_, 1, glm::value_ptr(vec));
	}
};

//specialization for matrices
template <const char* uniformName, glm::length_t C, glm::length_t R, typename cType, glm::qualifier Q>
class gltUniform_impl<gltUnifDescr<uniformName, glm::mat<C, R, cType, Q>>>
{
	GLint handle_ = -1;

public:

	void Init(char_t<uniformName>&&, GLuint handleShaderProg)
	{
		//uniform name has to be a null terminated string
		handle_ = glGetUniformLocation(handleShaderProg, uniformName);

		//this is critical
		assert(handle_ != -1 && "Failed to get uniform location!");
	}

	//works
	void UpdateUniform(char_t<uniformName>&&, const glm::mat<C, R, cType, Q>& matrx, bool transpose = false)
	{
		constexpr //void(__stdcall *pUnifFunc)(GLint, GLsizei, GLboolean, const cType*) =
			auto pUnifFunc =
			glUniformMap::found_pair<glm::mat<C, R, cType, Q>>::value::value;
		(*pUnifFunc)(handle_, 1, transpose, glm::value_ptr(matrx));
	}
};


template <class ... glUniform_impls>
struct gltUniformCollection_base : public glUniform_impls...
{
	using glUniform_impls::Init...;
	using glUniform_impls::UpdateUniform...;
};


template <class>
struct gltUniformCollection;

template <class ... UnifDescrs>
struct gltUniformCollection<std::tuple<UnifDescrs...>> : public gltUniformCollection_base<gltUniform_impl<UnifDescrs>...>
{
	//TODO: assert that provided UnifDescrs all have ::name
	void InitAll(GLuint hShaderProg)
	{
		(Init(char_t<UnifDescrs::name>(), hShaderProg), ...);
	}
};