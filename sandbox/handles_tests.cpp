
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
        throw ("Invalid values' sequence!");

    return 0;
}

template <typename ... T>
int CheckHandles()
{
    return (CheckHandle<T>() + ...);
}


#include <chrono>
#include <map>

template <int ... keys>
struct test
{
	inline static std::map<int, char> map_{ std::pair(keys, 'a' + keys) ... };
};

template struct test<4, 5, 8, 12, 16>;

int main()
{

	std::array<int, 4> arr;
	modify_array<int, 4>::modify(arr, 2, 3, 2, 1);

	is_compound_attr_v<glm::vec4>;
	is_compound_attr_v<comp_attr<glm::vec4>>;
	is_compound_attr_v<comp_attr<glm::vec4, glm::vec2>>;

    SmartGLFW sglfw{4, 4};
    SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "testing buffers" };

    sglfw.MakeContextCurrent(window);

    sglfw.LoadOpenGL();

    int res = CheckHandles<gltBufferTarget,
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


    //gl_bound_handle_base<gltBufferTarget, gltBufferTarget::array_buffer>::Bind(tag<gltBufferTarget::array_buffer>(), hBuf);
    gl_bound_handle_base<gltBufferTarget, gltBufferTarget::array_buffer>::binding;

    has_gl_binding_v<gltBufferTarget::array_buffer>;
    get_binding_v<gltBufferTarget::array_buffer>;


	gltHandle<gltBufferTarget> hBuf = gltAllocator<gltBufferTarget>::Allocate();


	//gl_bound_handle<glBufferTargetList>::Bind(tag<gltBufferTarget::array_buffer>(), hBuf);

	gltBuffer<glm::vec3, glm::vec2> buf{};
	//buf.Bind(tag<gltBufferTarget::array_buffer>());
	buf.Bind(gltBufferTarget::array_buffer);
	buf.AllocateMemory(16, 16, glBufUse::static_draw);

	try
	{
		buf.InstancesAllocated(2);
	}
	catch (const std::out_of_range& e)
	{
		std::cerr << e.what() << std::endl;
	}
	bool isCurrent = buf.IsBound(gltBufferTarget::array_buffer);
		//gl_bound_handle<glBufferTargetList>::IsBound(tag<gltBufferTarget::array_buffer>(), buf);

    return res;
}