
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
	// int, float, double, not glslt
	template <class T, class =
		decltype(std::make_index_sequence<elements_count_v<T>>())>
		struct get_uniform_arg_types
	{
		using type = std::tuple<T>;
		using vtype = std::tuple<const glm::vec<1, T>&>;
	};

	template <glm::length_t L, typename T,
		size_t ... indx>
		struct get_uniform_arg_types<glm::vec<L, T>, std::index_sequence<indx...>>
	{
		using type = std::tuple<convert_v_to<T, indx>...>;
		using vtype = std::tuple<const glm::vec<L, T>&>;
	};

	template<glm::length_t C, glm::length_t R, typename T,
		size_t ... indx>
		struct get_uniform_arg_types<glm::mat<C, R, T>, std::index_sequence<indx...>>
	{
		// TODO: validate 'type' case
		using type = std::tuple<bool, const glm::mat<C, R, T>&>;
		using vtype = std::tuple<bool, const glm::mat<C, R, T>*>;
	};
}

template <class glslt, class T = glt::unwrap_glslt_t<glslt>,
	class ArgsList = typename glt::get_uniform_arg_types<T>::type,
	class ArgsListV = typename glt::get_uniform_arg_types<T>::vtype>
	struct DummyUpdate;

template <class T, const char *name,
	class ... Args,
	class ... ArgsV>
	struct DummyUpdate<glt::glslt<T, name>, T,
	std::tuple<Args...>,
	std::tuple<ArgsV...>>
{
	void Update(glt::tag_c<name>, Args ... args)
	{
		(std::cout << ... << args);
	}

	void Update(glt::tag_c<name>, ArgsV ... args)
	{
		
	}
};

void foo(int *a)
{
	std::cout << *a << std::endl;
}

int main()
{
	int a = 69;
	foo(&a);

	void(*pFooRef)(const int&) = reinterpret_cast<void(*)(const int&)>(&foo);

	(*pFooRef)(a);

	static_assert(std::is_same_v<std::tuple<float, float, float, float>,
		typename glt::get_uniform_arg_types<glm::vec4>::type>);
	static_assert(std::is_same_v<std::tuple<const glm::vec4&>,
		typename glt::get_uniform_arg_types<glm::vec4>::vtype>);

	glt::get_uniform_arg_types<glm::mat4>::type;

	/*
	static_assert(std::is_same_v<std::tuple<const glm::vec4&, // fails
		const glm::vec4&,
		const glm::vec4&, 
		const glm::vec4&, 
		const glm::vec4&>,
		typename glt::get_uniform_arg_types<glm::mat4>::type>);*/



	// intellisence is mental for integer arguemnt input
	DummyUpdate<glt::glslt<int, 0>>().Update(glt::tag_c<0>(), 16);  


	DummyUpdate<glt::glslt<glm::vec4, 0>>().
		Update(glt::tag_c<0>(), 3.0f, 2.0f, 1.0f, 4.0f);

//	DummyUpdate<glt::glslt<glm::mat4, 0>>().Update(glt::tag_c<0>(), glm::mat4());


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
	//glt::UniformCollection<collect_uniforms> uniforms{ prog.GetHandle() };

	// uniforms.Update()

	return 0;
}