
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
		Buffer<glm::vec3, glm::vec2> buffer{};

		std::vector<glm::vec3> cube_positions = glm_cube_positions();
		std::vector<glm::vec2> cube_tex_coords = glm_cube_texCoords();

		buffer.Bind(tag_v<BufferTarget::array_buffer>());
		buffer.AllocateMemory(cube_positions.size(), cube_tex_coords.size(), BufferUse::static_draw);
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
		using CmpdBuffer = Buffer<compound<glm::vec3, glm::vec2>>;

		std::vector<vertex> vertices = cube_vertexes();

		CmpdBuffer buffer{};
		buffer.Bind(tag_v<BufferTarget::array_buffer>());
		buffer.AllocateMemory(vertices.size(), BufferUse::static_draw);

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

	// testing Offset
	{
		static_assert(sizeof(vertex) == sizeof(compound<glm::vec3, glm::vec2>));

		using T0 = float;
		using T1 = glm::vec4;
		using T2 = compound<glm::vec3, glm::vec2>;
		using T20 = glm::vec3;
		using T21 = glm::vec2;
		using T3 = glm::vec2;
		using T4 = compound<float, glm::vec2, glm::vec3>;
		using T40 = float;
		using T41 = glm::vec2;
		using T42 = glm::vec3;

		using CmpdBuffer = Buffer<T0,
			T1,									
			T2,				
			T3,									
			T4		
		>;

		size_t s0 = 16,
			s1 = 40,
			s2 = 23,
			s3 = 50,
			s4 = 38;



		std::vector<vertex> vertices = cube_vertexes();

		CmpdBuffer buffer{};
		buffer.Bind(tag_v<BufferTarget::array_buffer>());
		buffer.AllocateMemory(s0, s1, s2, s3, s4, BufferUse::static_draw);

		VertexAttrib<T1> va1 = buffer.Attribute(tag_s<1>());
		VertexAttrib<T2> va2 = buffer.Attribute(tag_s<2>());
		VertexAttrib<T3> va3 = buffer.Attribute(tag_s<3>());
		VertexAttrib<T4> va4 = buffer.Attribute(tag_s<4>());


		// buffer.GetOffset<0>(tag_s<1>());					// assertion must fail
		// offset22 = buffer.GetOffset<2>(tag_s<2>());		// assertion must fail

		// TODO: make it prettier!!!!
		std::ptrdiff_t array_sizes[]
		{
			s0 * sizeof(T0),
			s1 * sizeof(T1),
			s2 * sizeof(T2),
			s3 * sizeof(T3),
			s4 * sizeof(T4)
		},
		valid_results[]
		{
			0,									// offset0
			array_sizes[0],						// offset1
			array_sizes[0] + array_sizes[1],	// offset20
			array_sizes[0] + array_sizes[1] + sizeof(T20),	// offset 21
			array_sizes[0] + array_sizes[1] + array_sizes[2],	// offset 3
			array_sizes[0] + array_sizes[1] + array_sizes[2] +
												array_sizes[3],	// offset 40
			array_sizes[0] + array_sizes[1] + array_sizes[2] +
								array_sizes[3] + sizeof(T42), 	// offset 41
			array_sizes[0] + array_sizes[1] + array_sizes[2] +
								array_sizes[3] + sizeof(T42) * 2	// offset 42
		},
		offsets[]
		{
			buffer.GetOffset(tag_s<0>()),
			buffer.GetOffset(tag_s<1>()),
			buffer.GetOffset(tag_s<2>()),
			buffer.GetOffset(tag_s<2>(), tag_s<1>()),
			buffer.GetOffset(tag_s<3>()),
			buffer.GetOffset(tag_s<4>()),
			buffer.GetOffset(tag_s<4>(), tag_s<1>()),
			buffer.GetOffset(tag_s<4>(), tag_s<2>())
		};

		static_assert(std::extent_v<decltype(valid_results)> ==
			std::extent_v<decltype(offsets)>, "Arrays' dimensions mismatch!");

		bool test_res = true;

		for (size_t i = 0; i != std::extent_v<decltype(offsets)>; ++i)
			if (valid_results[i] != offsets[i])
			{
				std::cerr << "Invalid offset in case " << i << std::endl;
				test_res = false;
			}
		
		if (!test_res)
			return -1;

	}



	return 0;

}


