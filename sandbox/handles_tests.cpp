
#include "helpers.hpp"

/* This unit checks for correctness: 
- handle allocations
*/

template <typename T>
int CheckHandle()
{
	T t = T();
	if constexpr (std::is_same_v<T, glt::ShaderTarget>)
		t = glt::ShaderTarget::vertex;

    glt::Handle<T> h1 = glt::Allocator::Allocate(t);
    glt::Handle<T> h2 = glt::Allocator::Allocate(t);

    if (glt::handle_accessor(h1) != 1 ||
		glt::handle_accessor(h2) != 2)
        throw ("Invalid values' sequence!");

    return 0;
}

template <typename ... T>
int CheckHandles()
{
    return (CheckHandle<T>() + ...);
}

int main()
{
    SmartGLFW sglfw{4, 4};
    SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "testing buffers" };

    sglfw.MakeContextCurrent(window);
    sglfw.LoadOpenGL();

	return CheckHandles<glt::BufferTarget,
		glt::FrameBufferTarget,
		glt::TextureTarget,
		glt::VAOTarget,
		glt::TransformFeedBackTarget,
		glt::QueryTarget,
		glt::ProgramPipeLineTarget,
		glt::RenderBufferTarget,
		glt::SamplerTarget,
		glt::ShaderTarget,
		glt::ProgramTarget
	>();
}