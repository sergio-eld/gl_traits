#include "assimp_model.h"

#include "assimp_model_impl.h"


glt::TexFormat translate_texture_format(const Image & im)
{
	switch (im.NumChannels())
	{
	case 1:
		// this case should be handled inside a frag shader
		assert(false && "Unhandled case");
		return glt::TexFormat::red;
	case 2:
		// this case should be handled inside a frag shader (need to check return value)
		assert(false && "Unhandled case");
		return glt::TexFormat::rd;
	case 3:
		return glt::TexFormat::rgb;
	case 4:
		return glt::TexFormat::rgba;
	default:
		throw std::exception("Unhandled case!");
	}
}


TexturesCached Model::cached_textures_{};

Model::Model(const fsys::path & path)
	: path_(path)
{
}

void Model::Draw(ProgModel::ProgGuard & pg)
{
	for (Mesh& m : meshes_)
	{
		// TODO: set up textures (for now only texture diffuse)
		assert(m.materialIndx_ < materials_.size() && "Invalid material index!");
		Material& mat = materials_[m.materialIndx_];

		// no diffuse textures???
		if (!mat.tex_refs_.Count(glt::tag_v<aiTextureType_DIFFUSE>()))
		{
			m.Draw(pg);
			continue;
		}

		std::optional<RefTexture2Drgba> refTexture =
			cached_textures_.FindTexture(
				mat.tex_refs_.RefPath(glt::tag_v<aiTextureType_DIFFUSE>(), 0));

		assert(refTexture && "Texture is not cached!");
		glt::Texture2Drgba& texture = *refTexture;

		glActiveTexture(GL_TEXTURE0);
		pg.Uniforms().Set(texture_diffuse_sampler2D{ 0 });

		texture.Bind();

		m.Draw(pg);

		texture.UnBind();
	}
}

void Model::LoadMaterials(Model & model, const aiScene & scene)
{
	for (size_t t = 0; t != scene.mNumMaterials; ++t)
	{
		const aiMaterial& mat = *scene.mMaterials[t];

		model.materials_.emplace_back(
			Material(mat, Model::cached_textures_, model.path_.parent_path()));
	}
}

void Model::LoadModelMesh(Model& model, const aiScene& scene)
{
	model.name_ = std::string(scene.mRootNode->mName.C_Str(),
		scene.mRootNode->mName.length);

	std::vector<aiMesh*> meshes{ scene.mMeshes,
		std::next(scene.mMeshes, scene.mNumMeshes) };

	for (aiMesh *m : meshes)
		model.meshes_.emplace_back(*m);

}

std::optional<RefTexture2Drgba> TexturesCached::FindTexture(fsys::path p)
{
	std::map<fsys::path, glt::Texture2Drgba>::iterator found =
		textures_.find(p);

	if (found == textures_.cend())
		return std::nullopt;

	return RefTexture2Drgba(found->second);
}

Mesh::Mesh(const aiMesh& mesh)
	: name({ mesh.mName.C_Str(), mesh.mName.length }),
	//bufMesh(glt::buffer_cast<BufferMesh&&>(assimp_loader::LoadPosNormTex(mesh))),
	bufMesh(reinterpret_cast<BufferMesh&&>(assimp_loader::LoadPosNormTex(mesh))),
	bufElems(assimp_loader::LoadElements(mesh))
{
	materialIndx_ = mesh.mMaterialIndex;

	static_assert(glt::is_equivalent_v<attrNormals_vec3, glm::vec3>);
	static_assert(glt::is_equivalent_v<attrPosXYZ_vec3, glm::vec3>);
	static_assert(glt::is_equivalent_v<attrTexCoords_vec3, glm::vec3>);

	static_assert(glt::is_equivalent_v<glm::vec3, attrNormals_vec3>);
	static_assert(glt::is_equivalent_v<glm::vec3, attrPosXYZ_vec3>);
	static_assert(glt::is_equivalent_v<glm::vec3, attrTexCoords_vec3>);

	// TODO: this must yield true!
	//static_assert(glt::is_equivalent_v<VertexData, VertexDataPosNormTex>);

	vao.Bind();
	bufMesh.Bind(glt::BufferTarget::array);

	vao.AttributePointer(bufMesh()());
	vao.AttributePointer(bufMesh()(glt::tag_s<1>()));
	vao.AttributePointer(bufMesh()(glt::tag_s<2>()));
	bufMesh.UnBind();

	vao.EnablePointers();
	vao.UnBind();
	
	assert(glt::AssertGL());
}

Mesh::Mesh(Mesh && other)
	: name(std::move(other.name)),
	materialIndx_(other.materialIndx_),
	bufMesh(std::move(other.bufMesh)),
	bufElems(std::move(other.bufElems)),
	vao(std::move(other.vao))
{}



void Mesh::Draw(ProgModel::ProgGuard & pg)
{
	// TODO: setup textures?
	vao.Bind();
	pg.DrawElements(bufElems, glt::RenderMode::triangles, bufElems().Allocated());
	vao.UnBind();
}
