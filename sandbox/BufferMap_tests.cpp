
#include "helpers.hpp"


using namespace glt;

struct Coord
{
	glm::vec3 xyz;
};

int main()
{
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
		BufferOld<glm::vec3, glm::vec2> buffer{};

		std::vector<glm::vec3> cube_positions = glm_cube_positions();
		std::vector<glm::vec2> cube_tex_coords = glm_cube_texCoords();

		buffer.Bind(tag_v<BufferTarget::array>());
		buffer.AllocateMemory(cube_positions.size(), cube_tex_coords.size(), BufUsage::static_draw);
		buffer.BufferData<0>(cube_positions.data(), cube_positions.size());

		// testing offset. TODO: move to another unit test
		if (buffer.GetOffset(tag_s<0>()) != 0 ||
			buffer.GetOffset(tag_s<1>()) != sizeof(glm::vec3) * cube_positions.size())
		{
			std::cerr << "Invalid offset recieved!" << std::endl;
			return -1;
		}

		BufferMap<glm::vec3, MapAccess::read_only> mapped =
			buffer.MapBuffer<0, MapAccess::read_only>();

		auto iter_position = cube_positions.cbegin();
		for (auto map_iter : mapped)
			if (map_iter != *iter_position++)
			{
				std::cerr << "Mapped data differs from initial!" << std::endl;
				return -1;
			}

		mapped.UnMap();
		mapped = buffer.MapBuffer<0, MapAccess::read_only>();
	}

	// Loading user-defined vertexes
	{
		using CmpdBuffer = BufferOld<compound<glm::vec3, glm::vec2>>;

		std::vector<vertex> vertices = cube_vertexes();

		CmpdBuffer buffer{};
		buffer.Bind(tag_v<BufferTarget::array>());
		buffer.AllocateMemory(vertices.size(), BufUsage::static_draw);

		// testing offset. TODO: move to another unit test
		if (buffer.GetOffset(tag_s<0>(), tag_s<0>()) != 0 ||
			buffer.GetOffset(tag_s<0>(), tag_s<1>()) != sizeof(glm::vec3))
		{
			std::cerr << "Invalid offset recieved!" << std::endl;
			return -1;
		}


		/*
		static_assert(std::is_same_v<CmpdBuffer::NthType_t<0>, compound<glm::vec3, glm::vec2>>);
		static_assert(glt::is_equivalent_v<compound<glm::vec3, glm::vec2>, vertex>);
		static_assert(glt::is_equivalent_v<CmpdBuffer::NthType_t<0>, vertex>);*/

		buffer.BufferData(vertices.data(), vertices.size());

		// BufferMap<Coord, MapAccess::read_only> mapped = // assertion fails, not equivalent
		//	buffer.MapBuffer<0, MapAccess::read_only>(glt::tag_t<Coord>());

		try 
		{
			BufferMap<vertex, MapAccess::read_only> mapped =
				buffer.MapBuffer<0, MapAccess::read_only>(tag_t<vertex>());

			auto iter_position = vertices.cbegin();
			for (const auto& map_iter : mapped)
				if (map_iter != *iter_position++)
				{
					std::cerr << "Mapped data differs from initial!" << std::endl;
					return -1;
				}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			return -1;
		}
	}

	return 0;

}


