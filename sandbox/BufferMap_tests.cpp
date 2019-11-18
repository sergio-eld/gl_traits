
#include "helpers.hpp"


int main()
{
	SmartGLFW sglfw{ 4, 5 };

	SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "testing buffers' mapping" };
	sglfw.MakeContextCurrent(window);
	sglfw.LoadOpenGL();

	auto namedbufferdata = &glNamedBufferData;

	gltBuffer<glm::vec3, glm::vec2> buffer{};

	std::vector<glm::vec3> cube_positions = glm_cube_positions();
	std::vector<glm::vec2> cube_tex_coords = glm_cube_texCoords();

	buffer.Bind(tag<gltBufferTarget::array_buffer>());
	buffer.AllocateMemory(cube_positions.size(), cube_tex_coords.size(), glBufUse::static_draw);
	buffer.BufferData<0>(cube_positions.data(), cube_positions.size());

	gltBufferMap<glm::vec3, gltBufferMapStatus::read_only> mapped =
		buffer.MapBuffer<0, gltBufferMapStatus::read_only>();

	auto iter_position = cube_positions.cbegin();
	for (auto map_iter : mapped)
		if (map_iter != *iter_position++)
		{
			std::cerr << "Mapped data differs from initial!" << std::endl;
			return -1;
		}

	mapped.UnMap();
	mapped = buffer.MapBuffer<0, gltBufferMapStatus::read_only>();


	return 0;

}


