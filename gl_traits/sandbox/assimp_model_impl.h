#pragma once

#include "gl_traits.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

using VertexDataPosNormTex = glt::compound<glm::vec3, glm::vec3, glm::vec3>;

using BufferPosNormTex = glt::Buffer<VertexDataPosNormTex>;
using BufferElems = glt::Buffer<unsigned int>;

/* these class's methods may be called only after opengl context initialization*/
class assimp_loader
{
	static bool check_loaded_PosNormTex(BufferPosNormTex& buf, const aiMesh & mesh);
	static bool check_loaded_elems(BufferElems& buf, const aiMesh & mesh);

public:

	static BufferPosNormTex LoadPosNormTex(const aiMesh & mesh);
	static BufferElems LoadElements(const aiMesh & mesh);




};
