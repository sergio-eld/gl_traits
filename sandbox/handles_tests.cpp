
#include "helpers.hpp"

/* Check for correctness: 
- handle allocations
- handle bindings
*/

template <typename T>
int CheckHandle()
{
    gltHandle<T> h1 = gltAllocator<T>::Allocate();
    gltHandle<T> h2 = gltAllocator<T>::Allocate();

    if (handle_accessor<T>()(h1) != 1 ||
        handle_accessor<T>()(h2) != 2)
        throw ("Invalid values sequence!");

    return 0;
}

template <typename ... T>
int CheckHandles()
{
    return (CheckHandle<T>() + ...);
}


template <int a>
struct dummy
{
	void print(tag<a>) const
	{
		std::cout << a << std::endl;
	}
};

template <class ... cl>
struct collection : cl...
{
	using cl::print...;
};

template <int ... I>
struct dummy_collection : public collection<dummy<I>...>
{
	template <size_t n>
	constexpr static int arg_n = std::tuple_element_t<n, std::tuple<std::integral_constant<int, I>...>>::value;
	
	template <size_t iter = 0>
	constexpr void print_or_next(int a) const
	{
		if constexpr (iter != sizeof...(I))
		{
			constexpr int cur = arg_n<iter>;
			if (a == cur)
				return print(tag<cur>());
			return print_or_next<iter + 1>(a);
		}
		throw std::invalid_argument("Invalid input value");
	}

	void print_rt(int a)
	{
		print_or_next(a);
	}

};


int main()
{
	dummy_collection<4, 8, 15, 16, 23, 42> col;
	col.print(tag<4>());
	col.print_rt(8);
	col.print_rt(23);
	col.print_rt(42);
	try
	{
		col.print_rt(2);
	}
	catch (const std::invalid_argument& e)
	{
		std::cerr << e.what() << std::endl;
	}

	std::tuple_element_t<1, std::tuple<std::integral_constant<int, 3>, std::integral_constant<int, 4>>>::value;

	std::array<int, 4> arr;
	modify_array<int, 4>::modify(arr, 2, 3, 2, 1);

	is_compound_attr_v<glm::vec4>;
	is_compound_attr_v<comp_attr<glm::vec4>>;
	is_compound_attr_v<comp_attr<glm::vec4, glm::vec2>>;

	(gltBuffer<glm::vec4>*)nullptr;
	(gltBuffer<comp_attr<glm::vec4>>*)nullptr;
	(gltBuffer<glm::vec4, comp_attr<glm::vec4, glm::vec3>>*)nullptr;


    SmartGLFW sglfw{4, 4};
    SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "testing buffers" };

    sglfw.MakeContextCurrent(window);

    sglfw.LoadOpenGL();

    int res = CheckHandles<glBufferTarget,
        glFrameBufferTarget,
        glTextureTarget,
        glVertexArrayTarget,
        glTransformFeedBackTarget,
        glQueryTarget,
        glProgramPipeLineTarget,
        glRenderBufferTarget,
        glSamplerTarget,
        glProgramTarget>();

    auto namedbufferdata = &glNamedBufferData;

    // TODO: add glShaderTarget test;

    gltHandle<glBufferTarget> hBuf = gltAllocator<glBufferTarget>::Allocate();

    //gl_current_object_base<glBufferTarget, glBufferTarget::array_buffer>::Bind(tag<glBufferTarget::array_buffer>(), hBuf);
    gl_current_object_base<glBufferTarget, glBufferTarget::array_buffer>::binding;

    has_gl_binding_v<glBufferTarget::array_buffer>;
    get_binding_v<glBufferTarget::array_buffer>;

    gl_current_object<glBufferTargetList>::Bind(tag<glBufferTarget::array_buffer>(), hBuf);
    gl_current_object<glBufferTargetList>::IsCurrent(tag<glBufferTarget::array_buffer>(), hBuf);

	gltBuffer<glm::vec3, glm::vec2> buf{};
	//buf.Bind(tag<glBufferTarget::array_buffer>());
	buf.Bind(glBufferTarget::array_buffer);
	buf.AllocateMemory(16, 16, glBufUse::static_draw);

	try
	{
		buf.InstancesAllocated(2);
	}
	catch (const std::out_of_range& e)
	{
		std::cerr << e.what() << std::endl;
	}
	bool isCurrent = buf.IsCurrent(glBufferTarget::array_buffer);
		//gl_current_object<glBufferTargetList>::IsCurrent(tag<glBufferTarget::array_buffer>(), buf);

    return res;
}