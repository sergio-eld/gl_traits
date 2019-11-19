
#include "helpers.hpp"

using namespace glt;


int main()
{
	vertex v{ glm::vec3(), glm::vec2() };

	std::is_aggregate_v<vertex>;
	std::is_constructible_v<vertex, glm::vec3()>;

	is_initializable<vertex, std::tuple<glm::vec3, glm::vec2>>::value;
	is_initializable_from_v<vertex, glm::vec3, glm::vec2>;

	SmartGLFW sglfw{ 4, 5 };

	SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "testing buffers' mapping" };
	sglfw.MakeContextCurrent(window);
	sglfw.LoadOpenGL();

	auto namedbufferdata = &glNamedBufferData;



	{
		Buffer<glm::vec3, glm::vec2> buffer{};

		std::vector<glm::vec3> cube_positions = glm_cube_positions();
		std::vector<glm::vec2> cube_tex_coords = glm_cube_texCoords();

		buffer.Bind(tag<BufferTarget::array_buffer>());
		buffer.AllocateMemory(cube_positions.size(), cube_tex_coords.size(), glBufUse::static_draw);
		buffer.BufferData<0>(cube_positions.data(), cube_positions.size());

		BufferMap<glm::vec3, BufferMapStatus::read_only> mapped =
			buffer.MapBuffer<0, BufferMapStatus::read_only>();

		auto iter_position = cube_positions.cbegin();
		for (auto map_iter : mapped)
			if (map_iter != *iter_position++)
			{
				std::cerr << "Mapped data differs from initial!" << std::endl;
				return -1;
			}

		mapped.UnMap();
		mapped = buffer.MapBuffer<0, BufferMapStatus::read_only>();
	}

	// Loading user-defined vertexes
	{
		using CmpdBuffer = Buffer<compound<glm::vec3, glm::vec2>>;

		std::vector<vertex> vertices = cube_vertexes();

		CmpdBuffer buffer{};
		buffer.Bind(tag<BufferTarget::array_buffer>());
		buffer.AllocateMemory(vertices.size(), glBufUse::static_draw);

		static_assert(std::is_same_v<CmpdBuffer::NthType_t<0>, compound<glm::vec3, glm::vec2>>);
		static_assert(glt::is_equivalent_v<compound<glm::vec3, glm::vec2>, vertex>);
		static_assert(glt::is_equivalent_v<CmpdBuffer::NthType_t<0>, vertex>);

		buffer.BufferData(vertices.data(), vertices.size());



	}



	return 0;

}


