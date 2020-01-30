/* buffer_test.cpp

This module test buffer objects for:

- Memory allocation: flag 1;
- Buffering Data (SubData): flag 2;
- Mapping Data for Write: flag 4;
- Mapping Data for Read: flag 8;
- Mapping Ranged Data for Read and Write: flag 16;

return code is a bitmask of flags set for each failed case;

// TODO: implement Named AttribPtr:
- user-defined conversion from Sequence class
*/

#include "helpers.hpp"

int test_allocation(int& mask);
int test_SubData_MapRead(int& mask);


int main()
{
   
    // run-time tests
	SmartGLFW glfw{ 3, 3 };
	SmartGLFWwindow window{ SCR_HEIGHT, SCR_WIDTH, "buffer test" };
	glfw.MakeContextCurrent(window);
	glfw.LoadOpenGL();

    /* use this to generate asssembly

    // TODO: optimize??
    glt::Buffer<glm::vec3, glm::vec4, float, int, glm::vec3, glm::fvec4, glm::vec3, glm::ivec4> batchedBuf;
    batchedBuf.Bind(glt::BufferTarget::array);

    size_t inst = 0;
    std::cin >> inst;

    batchedBuf.AllocateMemory(inst, inst, inst, inst, inst, inst, inst, inst, glt::BufUsage::static_draw);
    */

	int retMask = 0;
	test_SubData_MapRead(retMask);
    
	return retMask;
}



int test_SubData_MapRead(int & mask)
{
	glt::Buffer<glm::vec3, float, glm::vec2> batchedBuf;
	glt::Buffer<int> bElements;

    
	sizeof(batchedBuf);

	batchedBuf.Bind(glt::BufferTarget::array);
	bElements.Bind(glt::BufferTarget::element_array);

	std::vector<glm::vec3> positions = glm_cube_positions();
	std::vector<glm::vec2> tex_coords = glm_cube_texCoords();

	batchedBuf.AllocateMemory(positions.size(), 283, tex_coords.size(),
		glt::BufUsage::static_draw);
	bElements.AllocateMemory(positions.size(), glt::BufUsage::static_draw);

    batchedBuf().SubData(positions.data(), positions.size());
	batchedBuf(glt::tag_s<2>()).SubData(tex_coords.data(), 
		tex_coords.size());

    // test batched sequences

	for (const glm::vec3& v : glt::MapGuard(batchedBuf(), glt::MapAccessBit::read))
	{
		static std::vector<glm::vec3>::const_iterator vec3Iter = 
			positions.cbegin();
		if (v != *vec3Iter++)
		{
			mask |= 16;
			return mask;
		}
	}

	for (const glm::vec2& t : glt::MapGuard(batchedBuf(glt::tag_s<2>()),
		glt::MapAccessBit::read))
	{
		static std::vector<glm::vec2>::const_iterator vec2Iter =
			tex_coords.cbegin();
		if (t != *vec2Iter++)
		{
			mask |= 16;
			return mask;
		}
	}
        

    // check map guard range loop with equivalent types

    std::vector<posVec3> pos_struct_vec3 = posVec3_cube_positions();
    std::vector<posVec3>::const_iterator pos_vec3_iter = 
        pos_struct_vec3.cbegin();

    for (const posVec3& v : glt::MapGuard(batchedBuf(), glt::MapAccessBit::read))
        if (v != *pos_vec3_iter++)
        {
            mask |= 16;
            return mask;
        }


    // test compound sequences
    static_assert(glt::is_equivalent_v<glt::compound<glm::vec3, glm::vec2>, vertex>);
    static_assert(glt::is_equivalent_v<vertex, glt::compound<glm::vec3, glm::vec2>>);
    static_assert(std::is_convertible_v<glt::compound<glm::vec3, glm::vec2>, vertex>); 
    static_assert(std::is_convertible_v<vertex, glt::compound<glm::vec3, glm::vec2>>);

    glt::Buffer<glt::compound<glm::vec3, glm::vec2>> vertBuf;
    vertBuf.Bind(glt::BufferTarget::array);

    std::vector<vertex> vertices = cube_vertices();

    vertBuf.AllocateMemory(vertices.size(), glt::BufUsage::static_draw);
    glt::Sequence<glm::vec3, glm::vec2>& vertSequence = vertBuf;

    std::vector<vertex>::const_iterator vertIter = vertices.cbegin();

    // assigning values through map access
    for (vertex& v : glt::MapGuard(vertSequence, glt::MapAccessBit::write))
        v = *vertIter++;

    //vertSequence.SubData(vertices.data(), vertices.size());

    vertIter = vertices.cbegin();
    for (const vertex& v : glt::MapGuard(vertSequence, glt::MapAccessBit::read))
        if (v != *vertIter++)
        {
            mask |= 16;
            return mask;
        }

    
	return mask;
}
