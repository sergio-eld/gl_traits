#include "assimp_model_impl.h"

BufferPosNormTex assimp_loader::LoadPosNormTex(const aiMesh & mesh)
{
	BufferPosNormTex bufMesh;

	bufMesh.Bind(glt::BufferTarget::array);
	bufMesh.AllocateMemory(mesh.mNumVertices, glt::BufUsage::static_draw);

	assert(bufMesh().Allocated() == mesh.mNumVertices && "Ranges mismatch!");

	size_t vertIndx = 0;

	for (VertexDataPosNormTex& vD : glt::MapGuard(bufMesh(), glt::MapAccessBit::write))
	{
		glm::vec3& posXYZ = vD.Get(glt::tag_s<0>()),
			&norm = vD.Get(glt::tag_s<1>()),
			&texCoord = vD.Get(glt::tag_s<2>());

		const aiVector3D &aiPosXYZ = mesh.mVertices[vertIndx],
			&aiNorm = mesh.mNormals[vertIndx];

		posXYZ.x = aiPosXYZ.x;
		posXYZ.y = aiPosXYZ.y;
		posXYZ.z = aiPosXYZ.z;

		norm.x = aiNorm.x;
		norm.y = aiNorm.y;
		norm.z = aiNorm.z;

		assert(glt::AssertGL());

		if (!mesh.mTextureCoords[0])
		{
			texCoord.x = 0;
			texCoord.y = 0;
			texCoord.z = 0;

			++vertIndx;
			continue;
		}

		texCoord.x = mesh.mTextureCoords[0][vertIndx].x;
		texCoord.y = mesh.mTextureCoords[0][vertIndx].y;
		texCoord.z = mesh.mTextureCoords[0][vertIndx].z;

		++vertIndx;
	}

	assert(check_loaded_PosNormTex(bufMesh, mesh) && "Failed to load PosNormTex data");

	bufMesh.UnBind();

	return std::move(bufMesh);
}

BufferElems assimp_loader::LoadElements(const aiMesh & mesh)
{
	std::vector<std::reference_wrapper<aiFace>> faces{ mesh.mFaces,
		std::next(mesh.mFaces, mesh.mNumFaces) };

	BufferElems bufElems;

	bufElems.Bind(glt::BufferTarget::element_array);
	bufElems.AllocateMemory(mesh.mNumFaces * 3, glt::BufUsage::static_draw);

	assert(bufElems().Allocated() == mesh.mNumFaces * 3 && "Elements ranges mismatch!");
	{
		glt::MapGuard elemsMap{ bufElems(), glt::MapAccessBit::write };

		decltype(elemsMap)::iterator elIter = elemsMap.begin();

		for (const aiFace& f : faces)
			for (size_t i = 0; i != f.mNumIndices; ++i)
			{
				unsigned int& elem = *elIter;
				elem = f.mIndices[i];
				++elIter;
			}

		assert(elIter == elemsMap.end() && "Elements range mismatch!");
	}

	assert(check_loaded_elems(bufElems, mesh) && "Failed to load elements data!");

	bufElems.UnBind();

	return std::move(bufElems);
}

bool assimp_loader::check_loaded_PosNormTex(BufferPosNormTex & buf, const aiMesh & mesh)
{
	assert(buf().Allocated() == mesh.mNumVertices && "Ranges mismatch!");
	size_t vertIndx = 0;

	for (const VertexDataPosNormTex& vD : glt::MapGuard(buf(), glt::MapAccessBit::read))
	{
		const glm::vec3& posXYZ = vD.Get(glt::tag_s<0>()),
			&norm = vD.Get(glt::tag_s<1>()),
			&texCoord = vD.Get(glt::tag_s<2>());

		const aiVector3D &aiPosXYZ = mesh.mVertices[vertIndx],
			&aiNorm = mesh.mNormals[vertIndx];

		if (posXYZ.x != aiPosXYZ.x ||
			posXYZ.y != aiPosXYZ.y ||
			posXYZ.z != aiPosXYZ.z ||

			norm.x != aiNorm.x ||
			norm.y != aiNorm.y ||
			norm.z != aiNorm.z)
			return false;

		++vertIndx;
	}

	return true;
}

bool assimp_loader::check_loaded_elems(BufferElems & buf, const aiMesh & mesh)
{
	std::vector<std::reference_wrapper<aiFace>> faces{ mesh.mFaces,
			std::next(mesh.mFaces, mesh.mNumFaces) };

	assert(buf().Allocated() == mesh.mNumFaces * 3 && "Elements ranges mismatch!");
	{
		glt::MapGuard elemsMap{ buf(), glt::MapAccessBit::read };

		decltype(elemsMap)::iterator elIter = elemsMap.begin();

		for (const aiFace& f : faces)
			for (size_t i = 0; i != f.mNumIndices; ++i)
			{
				const unsigned int& elem = *elIter;
				if (elem != f.mIndices[i])
					return false;
				++elIter;
			}

		assert(elIter == elemsMap.end() && "Elements range mismatch!");
	}

	return true;
}
