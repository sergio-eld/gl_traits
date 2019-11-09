
#include "helpers.hpp"

template <auto ...>
struct str;

template <int a>
struct str<a>
{
    void invoke() const
    {
        std::cout << "int" << std::endl;
    }
};

template <bool b, int a>
struct str<b, a>
{
    void invoke() const
    {
        std::cout << b << " " << a << std::endl;
    }
};


template <typename T>
int CheckHandle()
{
    gltHandle<T> h1 = handle_allocator<T>::Allocate();
    gltHandle<T> h2 = handle_allocator<T>::Allocate();

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

int main()
{
    str<6>().invoke();
    str<false, 16>().invoke();

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

    // TODO: add glShaderTarget test;

    return res;
}