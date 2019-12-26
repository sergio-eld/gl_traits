
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
		return 1;

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

	// see disassembly for these 3 lines
	GLuint rawHandle;
	(*glt::pp_gl_allocator_v<glt::BufferTarget>)(1, &rawHandle);
	glt::HandleBuffer handle = glt::Allocator::Allocate(glt::BufferTarget());

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