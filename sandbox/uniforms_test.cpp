
#include "helpers.hpp"

#include <fstream>
#include <streambuf>

constexpr inline const char uname_model[] = "model",
uname_view[] = "view",
uname_projection[] = "projection",
uname_texture1[] = "texture1",
uname_texture2[] = "texture2";

using collect_uniforms = std::tuple<glt::glslt<glm::mat4, uname_model>,
	glt::glslt<glm::mat4, uname_view>,
	glt::glslt<glm::mat4, uname_projection>,
	glt::glslt<int, uname_texture1>,
	glt::glslt<int, uname_texture2>>;

namespace glt
{

	template <typename T, size_t sz = 0,
		class = decltype(std::make_index_sequence<sz>())>
	struct UniformModifier;

	template <typename T, size_t sz, size_t ... indx>
	struct UniformModifier<T, sz, std::index_sequence<indx...>>
	{


		// TODO: change GLint to handle
		static void Update(GLint handle,
			convert_v_to<T, indx>... vals)
		{
			p_gl_uniform_t<T, sz> pglUniformT = get_p_gl_uniform<T, sz>();

			(*pglUniformT)(handle, vals...);
		}

		static void Get(const HandleProg& prog, GLint handle, glm::vec<sz, T>&ret)
		{
			void(*ptr)(GLint, GLint, T*) = *pp_gl_get_uniform_map<T>();
			//constexpr auto ptr = pp_gl_get_uniform_map<T>();
			(*ptr)(handle_accessor(prog), handle, &ret);
		}

		static glm::vec<sz, T> Get(const HandleProg& prog, GLint handle)
		{
			glm::vec<sz, T> ret{};
			void(*ptr)(/*const HandleProg&*/ GLint, GLint, glm::vec<sz, T>&) = 
				reinterpret_cast<decltype(ptr)>(*pp_gl_get_uniform_map<T>());

			(*ptr)(handle_accessor(prog), handle, ret);

			return ret;
		}


	};

	template <typename T, glm::length_t L>
	struct UniformModifier<glm::vec<L, T>>
	{
		// TODO: change GLint to handle
		static void Update(GLint handle, const glm::vec<L, T>& vec)
		{
			p_gl_uniform_t<glm::vec<L, T>> pglUniformT = 
				get_p_gl_uniform<glm::vec<L, T>, 1>();

			(*pglUniformT)(handle, L, vec);
		}
	};

	template <typename T, glm::length_t C, glm::length_t R>
	struct UniformModifier<glm::mat<C, R, T>>
	{
		// TODO: change GLint to handle
		static void Update(GLint handle, const glm::mat<C, R, T>& mat, 
			bool transpose = false)
		{
			p_gl_uniform_t<glm::mat<C, R, T >> pglUniformT =
				get_p_gl_uniform<glm::mat<C, R, T>, 1>();

			(*pglUniformT)(handle, 1, transpose, mat);
		}
	};

	template <typename T, const char *name>
	class UniformBase
	{
	protected:

		// TODO: make prog a pointer?
		const HandleProg& prog_;
		GLint handle_ = -1;

		UniformBase(const HandleProg& prog)
			: prog_(prog),
			handle_(GetHandle())
		{
			assert(handle_ != -1 && "Uniform::Failed to get Uniform location!");
		}

	private:
		GLint GetHandle()
		{
			assert(ActiveProgram::IsActive(prog_) &&
				"Uniform::Trying to access a uniform when Program is not active!");
			return glGetUniformLocation(handle_accessor(prog_),
				name);
		}

	};

	
	template <class glslt_T>
	class UniformTest;

	// TODO: use unwrapper to get size of T. size = 0 for mat and vec
	template <class T, const char * name>
	class UniformTest<glslt<T, name>>
	{

	};

	template <class glslt_T, class T = unwrap_glslt_t<glslt_T>,
		class = decltype(std::make_index_sequence<elements_count_v<T>>())>
		class Uniform;

	// glUniformMatrix
	template <glm::length_t C, glm::length_t R, typename T,
		const char *name>
		class Uniform<glslt<glm::mat<C, R, T>, name>,
		glm::mat<C, R, T>> : protected UniformBase<glm::mat<C, R, T>, name>
	{
	public:

		Uniform(const HandleProg& prog)
			: UniformBase<glm::mat<C, R, T>, name>(prog)
		{
			assert(handle_ != -1 && "Uniform::Failed to get Uniform location!");
		}

		void Update(tag_c<name>, const glm::mat<C, R, T>& val, bool transpose = false) const
		{
			p_gl_uniform_t<glm::mat<C, R, T>> pglUniformMatrixT =
				get_p_gl_uniform<glm::mat<C, R, T>>();

			(*pglUniformMatrixT)(handle_, 1, transpose, val);
		}

	};

	// glUniform1* and glUniform1*v
	template <typename T, const char *name>
	class Uniform<glslt<T, name>> : protected UniformBase<T, name>
	{

	public:

		Uniform(const HandleProg& prog)
			: UniformBase<T, name>(prog)
		{}

		void Update(tag_c<name>, T val) const
		{
			UniformModifier<T, 1>::Update(handle_, val);

			// TODO: implement
			//p_gl_uniform_t<T, 1> pglUniformT = get_p_gl_uniform<T, 1>();

			//(*pglUniformT)(handle_, val);
		}

		void Update(tag_c<name>, const glm::vec<1, T>& val) const
		{
			p_gl_uniform_t<glm::vec<1, T>> pglUniformTv = get_p_gl_uniform<glm::vec<1, T>>();

			(*pglUniformTv)(handle_, 1, val);
		}

	
	};

	// glUniform1-4* and glUniform1-4*v
	template <glm::length_t L, typename T, const char *name, size_t ... indx>
	class Uniform<glslt<glm::vec<L, T>, name>,
		glm::vec<L, T>,
		std::index_sequence<indx...>> 
		: protected UniformBase<glm::vec<L, T>, name>
	{

	public:

		Uniform(const HandleProg& prog)
			: UniformBase<glm::vec<L, T>, name>(prog)
		{}

		void Update(tag_c<name>, convert_v_to<T, indx> ... val) const
		{
			// TODO: implement
			p_gl_uniform_t<T, L> pglUniformT = get_p_gl_uniform<T, L>();

			(*pglUniformT)(handle_, val...);
		}

		void Update(tag_c<name>, const glm::vec<L, T>& val) const
		{
			p_gl_uniform_t<glm::vec<L, T>> pglUniformTv = get_p_gl_uniform<glm::vec<L, T>>();

			(*pglUniformTv)(handle_, L, val);
		}

	};

	

	template <class TupleArgs,
		class = decltype(std::make_index_sequence<std::tuple_size_v<TupleArgs>>())>
		class UniformCollection;

	template <class ... Attribs, size_t ... indx>
	class UniformCollection<std::tuple<Attribs...>, std::index_sequence<indx...>>
		: Uniform<Attribs> ...
	{

		template <size_t i>
		using UnifBase = Uniform<nth_element_t<i, Attribs...>>;

	public:

		UniformCollection(const HandleProg& handle)
			: Uniform<Attribs>(handle)...
		{}

		using UnifBase<indx>::Update...;
	};

}


using uint = unsigned int;

int main()
{
	// check function types
	static_assert(std::is_same_v<glt::p_gl_uniform_t<int>,
		void(*)(GLint, int)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<int, 2>,
		void(*)(GLint, int, int)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<int, 3>,
		void(*)(GLint, int, int, int)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<int, 4>,
		void(*)(GLint, int, int, int, int)>);

	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::ivec1>,
		void(*)(GLint, GLsizei, const glm::ivec1&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::ivec2>,
		void(*)(GLint, GLsizei, const glm::ivec2&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::ivec3>,
		void(*)(GLint, GLsizei, const glm::ivec3&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::ivec4>,
		void(*)(GLint, GLsizei, const glm::ivec4&)>);

	static_assert(std::is_same_v<glt::p_gl_uniform_t<uint>,
		void(*)(GLint, uint)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<uint, 2>,
		void(*)(GLint, uint, uint)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<uint, 3>,
		void(*)(GLint, uint, uint, uint)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<uint, 4>,
		void(*)(GLint, uint, uint, uint, uint)>);

	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::uvec1>,
		void(*)(GLint, GLsizei, const glm::uvec1&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::uvec2>,
		void(*)(GLint, GLsizei, const glm::uvec2&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::uvec3>,
		void(*)(GLint, GLsizei, const glm::uvec3&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::uvec4>,
		void(*)(GLint, GLsizei, const glm::uvec4&)>);

	static_assert(std::is_same_v<glt::p_gl_uniform_t<float>,
		void(*)(GLint, float)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<float, 2>,
		void(*)(GLint, float, float)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<float, 3>,
		void(*)(GLint, float, float, float)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<float, 4>,
		void(*)(GLint, float, float, float, float)>);

	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::vec1>,
		void(*)(GLint, GLsizei, const glm::vec1&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::vec2>,
		void(*)(GLint, GLsizei, const glm::vec2&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::vec3>,
		void(*)(GLint, GLsizei, const glm::vec3&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::vec4>,
		void(*)(GLint, GLsizei, const glm::vec4&)>);

	static_assert(std::is_same_v<glt::p_gl_uniform_t<double>,
		void(*)(GLint, double)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<double, 2>,
		void(*)(GLint, double, double)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<double, 3>,
		void(*)(GLint, double, double, double)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<double, 4>,
		void(*)(GLint, double, double, double, double)>);

	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::dvec1>,
		void(*)(GLint, GLsizei, const glm::dvec1&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::dvec2>,
		void(*)(GLint, GLsizei, const glm::dvec2&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::dvec3>,
		void(*)(GLint, GLsizei, const glm::dvec3&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::dvec4>,
		void(*)(GLint, GLsizei, const glm::dvec4&)>);

	// TODO: check all the Matrices
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::mat<4, 1, float>>,
		void(*)(GLint, GLsizei, GLboolean, const glm::mat<4, 1, float>&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::mat2>,
		void(*)(GLint, GLsizei, GLboolean, const glm::mat2&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::mat3>,
		void(*)(GLint, GLsizei, GLboolean, const glm::mat3&)>);
	static_assert(std::is_same_v<glt::p_gl_uniform_t<glm::mat4>,
		void(*)(GLint, GLsizei, GLboolean, const glm::mat4&)>);

	// run-time checks
	SmartGLFW sglfw{ 4, 5 };
	SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "Testing uniforms" };

	sglfw.MakeContextCurrent(window);
	sglfw.LoadOpenGL();

	
	glt::Program prog;

	{
		std::fstream f{ path.generic_string() + "vshader.vs", std::fstream::in };

		if (!f)
		{
			std::cerr << "Failed to open vshader.vs" << std::endl;
			return -1;
		}

		std::string vSource{ std::istreambuf_iterator<char>(f),
			std::istreambuf_iterator<char>() };

		f.close();
		f.open(path.generic_string() + "fshader.fs", std::fstream::in);

		if (!f)
		{
			std::cerr << "Failed to open fshader.fs" << std::endl;
			return -1;
		}

		std::string fSource{ std::istreambuf_iterator<char>(f),
			std::istreambuf_iterator<char>() };

		glt::VertexShader vShader{ vSource };
		glt::FragmentShader fShader{ fSource };

		prog.Link(vShader, fShader);
	}

	prog.Use();

	glt::p_gl_uniform_t<glm::vec4> pglUniform4fv =
		glt::get_p_gl_uniform<glm::vec4, 1>();
	glt::p_gl_uniform_t<float, 4> pglUniform4f =
		glt::get_p_gl_uniform<float, 4>();

	

	glt::Uniform<glt::glslt<int, uname_texture1>> u1{ prog.GetHandle() };
	glt::Uniform<glt::glslt<int, uname_texture2>> u2{ prog.GetHandle() };

	GLint loc_tex1 =
		glGetUniformLocation(glt::handle_accessor(prog.GetHandle()), uname_texture1);
	int tex_val = 0;
	glGetUniformiv(glt::handle_accessor(prog.GetHandle()), loc_tex1, &tex_val);

	u1.Update(glt::tag_c<uname_texture1>(), 1);

	glm::ivec1 tex_val2 = glt::UniformModifier<int, 1>::Get(prog.GetHandle(), loc_tex1);


	//glGetUniformiv(glt::handle_accessor(prog.GetHandle()), loc_tex1, &tex_val);



	glt::Uniform<glt::glslt<glm::mat4, uname_model>> u3{ prog.GetHandle() };

	glt::UniformModifier<float, 4>::Update(-1, 4, 8, 15, 16);
	glt::UniformModifier<glm::vec4>::Update(-1, glm::vec4());
	glt::UniformModifier<glm::mat4>::Update(-1, glm::mat4());

	//u3.Update(glt::tag_c<uname_model>(), glm::mat4());
	//glt::UniformCollection<collect_uniforms> uniforms{ prog.GetHandle() };


	return 0;
}