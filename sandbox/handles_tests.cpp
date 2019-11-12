
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


struct abc
{
    int a;
};

int main()
{
    std::is_constructible_v<abc, int>;
    std::is_trivially_constructible_v<abc, int>;
    std::is_pod_v<abc>;
    std::is_aggregate_v<abc>;

    sizeof(abc);
    abc a{ 4 };
    int& b = reinterpret_cast<int&>(a);


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

    return res;
}