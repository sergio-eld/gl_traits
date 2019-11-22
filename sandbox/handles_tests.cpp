
#include "helpers.hpp"

/* Check for correctness: 
- handle allocations
- handle bindings
*/

using namespace glt;

template <typename T>
int CheckHandle()
{
    Handle<T> h1 = Allocator<T>::Allocate();
    Handle<T> h2 = Allocator<T>::Allocate();

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

constexpr char coord[] = "coord";

int main()
{

	std::array<int, 4> arr;
	modify_array<int, 4>::modify(arr, 2, 3, 2, 1);

	is_compound_attr_v<glm::vec4>;
	is_compound_attr_v<compound<glm::vec4>>;
	is_compound_attr_v<compound<glm::vec4, glm::vec2>>;

	// move this to another module
	// recover n_th type (also recovering from glslt)
	using tuple_test = std::tuple<compound<float>, glm::vec2, glslt<glm::vec3, coord>>;

	/*
	static_assert(std::is_same_v<compound<float>,
		nth_element_t<0, compound<float>, glm::vec2, glslt<glm::vec3, coord>>>);

	static_assert(std::is_same_v<glm::vec2,
		nth_element_t<1, float, glm::vec2, glslt<glm::vec3, coord>>>);
	static_assert(std::is_same_v<glm::vec3,
		nth_element_t<2, float, glm::vec2, glslt<glm::vec3, coord>>>);
		*/
	/////////////////////////////////////////////////////////////////////

    SmartGLFW sglfw{4, 4};
    SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "testing buffers" };

    sglfw.MakeContextCurrent(window);

    sglfw.LoadOpenGL();

    int res = CheckHandles<BufferTarget,
        glFrameBufferTarget,
        TextureTarget,
        VAOTarget,
        glTransformFeedBackTarget,
        glQueryTarget,
        glProgramPipeLineTarget,
        glRenderBufferTarget,
        glSamplerTarget,
        glProgramTarget>();

    auto namedbufferdata = &glNamedBufferData;

    // TODO: add glShaderTarget test;


    //bound_handle_base<BufferTarget, BufferTarget::array_buffer>::Bind(tag_v<BufferTarget::array_buffer>(), hBuf);
    bound_handle_base<BufferTarget, BufferTarget::array_buffer>::binding;

    has_gl_binding_v<BufferTarget::array_buffer>;
    get_binding_v<BufferTarget::array_buffer>;


	Handle<BufferTarget> hBuf = Allocator<BufferTarget>::Allocate();


	//bound_handle<BufferTargetList>::Bind(tag_v<BufferTarget::array_buffer>(), hBuf);

	Buffer<glm::vec3, glm::vec2> buf{};
	//buf.Bind(tag_v<BufferTarget::array_buffer>());
	buf.Bind(BufferTarget::array_buffer);
	buf.AllocateMemory(16, 16, BufferUse::static_draw);

	try
	{
		buf.InstancesAllocated(2);
	}
	catch (const std::out_of_range& e)
	{
		std::cerr << e.what() << std::endl;
	}
	bool isCurrent = buf.IsBound(BufferTarget::array_buffer);
		//bound_handle<BufferTargetList>::IsBound(tag_v<BufferTarget::array_buffer>(), buf);

    return res;
}