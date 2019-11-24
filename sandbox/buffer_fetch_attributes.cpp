
#include "helpers.hpp"

/*
- Initialize BufferL with given arguments;
- allocate memory, using SizesArray;
- fetch each attribute and check offset and stride
*/

// TOOD: customize glt::Buffer arguments at precompile

using T0 = float;
using T1 = glm::vec4;
using T2 = glt::compound<glm::vec3, glm::vec2>;
using T20 = glm::vec3;
using T21 = glm::vec2;
using T3 = glm::vec2;
using T4 = glt::compound<float, glm::vec2, glm::vec3>;
using T40 = float;
using T41 = glm::vec2;
using T42 = glm::vec3;

using attribs = glt::AttrList<T0, T1, T2, T3, T4>;


using SizesArray = std::array<size_t, glt::templ_params_count_v<attribs>>;

using TestBuffer = glt::BufferL<attribs>;

template <class AttrList,
    class indexes = decltype(std::make_index_sequence<glt::templ_params_count_v<AttrList>>())>
    struct FetchTester;

template <typename ... Attr, size_t ... indx>
struct FetchTester<glt::AttrList<Attr...>, std::index_sequence<indx...>>
{
    glt::Buffer<Attr...> buffer{};
    //std::array<size_t, sizeof...(Attr)> sizes;

    FetchTester(glt::convert_to<size_t, Attr> ... sizes, glt::BufferUse use)
    {
        buffer.Bind(glt::BufferTarget::array_buffer);
        buffer.AllocateMemory(sizes..., use);
    }

    FetchTester(size_t sizes, glt::BufferUse use)
    {
        buffer.Bind(glt::BufferTarget::array_buffer);
        buffer.AllocateMemory(glt::convert_to<size_t, Attr>(sizes)..., use);
    }


    template <size_t n, size_t ... subIndx>
    int FetchCompound(std::index_sequence<subIndx...>) const
    {
        // TODO: implement
        return 0;
    }

    template <size_t n>
    int Fetch(glt::tag_s<n>) const
    {
        std::ptrdiff_t offset = buffer.GetOffset(glt::tag_s<n>());

        int res = 0;

        using A = glt::nth_element_t<n, Attr...>;
        glt::VertexAttrib<A> va = buffer.Attribute(glt::tag_s<n>());

        if (va.Offset() != offset)
        {
            res = -1;
            std::cout << "Invalid offset at index " << n << std::endl;
        }

        // TODO: check stride for compound attribute
        return res;
    }

    int operator()() const
    {
        return (Fetch(glt::tag_s<indx>()) + ...);
    }
};


int main()
{
    SmartGLFW sglfw{ 4, 5 };
    SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "Test: Fetching Attributes" };

    sglfw.MakeContextCurrent(window);
    sglfw.LoadOpenGL();

    FetchTester<attribs> fTester{ 1000, glt::BufferUse::static_draw };

    return fTester();
}