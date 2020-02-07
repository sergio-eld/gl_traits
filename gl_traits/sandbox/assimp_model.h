#pragma once

#include "glt_Common.h"

#include "helpers.hpp"

using Uniforms = glt::uniform_collection<std::tuple<model_mat4,
	view_mat4,
	projection_mat4,
	texture_diffuse_sampler2D>>;

using VertexData = glt::compound<attrPosXYZ_vec3, attrNormals_vec3, attrTexCoords_vec3>;

using ProgModel = glt::Program<VAO_nanosuit_simple, Uniforms>;
using BufferMesh = glt::Buffer<VertexData>;
using BufferElems = glt::Buffer<unsigned int>;
using BufferTexture = glt::Buffer<unsigned char>;

using RefTexture2Drgba = std::reference_wrapper<glt::Texture2Drgba>;

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <numeric>
#include <optional>

#include <string_view>



glt::TexFormat translate_texture_format(const Image & im);


template <aiTextureType ... types>
class types_list
{
public:
	constexpr static size_t size = sizeof...(types);

	template <size_t n>
	constexpr static aiTextureType nth_type = get_nth<n, types...>();

private:

	template <size_t n, aiTextureType cur, aiTextureType ... next>
	constexpr static aiTextureType get_nth()
	{
		static_assert(n < size, "n is out of range!");
		if constexpr ((bool)n)
			return get_nth<n - 1, next...>();
		else
			return cur;
	}
};


// TODO: use hash for path
struct TexturesCached
{
	std::map<fsys::path, glt::Texture2Drgba> textures_;

	std::optional<RefTexture2Drgba> FindTexture(fsys::path p);

	glt::Texture2Drgba& GetTexture(fsys::path p)
	{
		std::optional<RefTexture2Drgba> loaded = FindTexture(p);
		if (loaded)
			return *loaded;

		Image im{ p };

		glt::Texture2Drgba tex;
		tex.Bind();

		tex.SetImage(0, im.Width(), im.Height());
		tex.SubImage(0, im.Width(), im.Height(), translate_texture_format(im),
			glt::TexType::unsigned_byte, im.Data());

		tex.GenerateMipMap();
		tex.UnBind();

		auto emplaced = textures_.emplace(std::make_pair(p, std::move(tex)));

		glt::Texture2Drgba& out = emplaced.first->second;
		return out;
	}

};

class Material
{

	template <aiTextureType type>
	class tex_ref
	{
		std::vector<fsys::path> textures_;

	public:

		void AppendRef(glt::tag_v<type>, fsys::path&& p)
		{
			textures_.emplace_back(std::move(p));
		}

		void ClearRefs(glt::tag_v<type>)
		{
			textures_.clear();
		}

		size_t Count(glt::tag_v<type>) const
		{
			return textures_.size();
		}

		const fsys::path& RefPath(glt::tag_v<type>, size_t indx) const
		{
			assert(indx < Count(glt::tag_v<type>()) && "Index out of range!");
			return textures_[indx];
		}

	};

	template <class texTypes, class = decltype(std::make_index_sequence<texTypes::size>())>
	class texture_refs;

	template <aiTextureType ... types, size_t ... indx>
	class texture_refs<types_list<types...>,
		std::index_sequence<indx...>> : protected tex_ref<types> ...
	{
		using t_list = types_list<types...>;

		template <size_t n>
		using tex_base = tex_ref<t_list::nth_type<n>>;

	public:

		using tex_base<indx>::AppendRef...;
		using tex_base<indx>::ClearRefs...;
		using tex_base<indx>::Count...;
		using tex_base<indx>::RefPath...;


	};

	using assimp_tex_types = types_list<aiTextureType_DIFFUSE,
		aiTextureType_SPECULAR,
		aiTextureType_AMBIENT,
		aiTextureType_EMISSIVE,
		aiTextureType_HEIGHT,
		aiTextureType_NORMALS,
		aiTextureType_SHININESS,
		aiTextureType_OPACITY,
		aiTextureType_DISPLACEMENT,
		aiTextureType_LIGHTMAP,
		aiTextureType_REFLECTION>;


public:

	texture_refs<assimp_tex_types> tex_refs_;


	Material(const aiMaterial& mat, TexturesCached& cached, const fsys::path& absFolder)
	{
		LoadTextures(mat, cached, absFolder, assimp_tex_types());
	}

private:

	template <aiTextureType type>
	void LoadTexture_t(const aiMaterial& mat, TexturesCached& cached, const fsys::path& absFolder)
	{
		aiString path;
		for (unsigned int i = 0; i != mat.GetTextureCount(type); ++i)
		{
			mat.GetTexture(type, i, &path);

			fsys::path p{ absFolder };
			p.append(path.C_Str());

			cached.GetTexture(p);

			tex_refs_.AppendRef(glt::tag_v<type>(), std::move(p));
		}
	}

	template <aiTextureType ... types>
	void LoadTextures(const aiMaterial& mat, TexturesCached& cached, const fsys::path& absFolder, types_list<types...>)
	{
		(LoadTexture_t<types>(mat, cached, absFolder), ...);
	}

};

struct Mesh
{
	std::string name;
	size_t materialIndx_;

	BufferMesh bufMesh;
	BufferElems bufElems;

	ProgModel::vao vao;

	Mesh(std::string_view name, size_t matIndx,
		BufferMesh&& bufMesh, BufferElems&& bufElems)
		: name(name),
		materialIndx_(matIndx),
		bufMesh(std::move(bufMesh)),
		bufElems(std::move(bufElems))
	{}

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(const aiMesh& mesh);

	Mesh(Mesh&& other);

	// TODO: add material?
	void Draw(ProgModel::ProgGuard &pg);

};

// model with compound buffer
struct Model
{
	static TexturesCached cached_textures_;

	fsys::path path_;
	std::string name_;

	std::vector<Mesh> meshes_;
	std::vector<Material> materials_;

	Model(const fsys::path& path);

	void Draw(ProgModel::ProgGuard &pg);

	static void LoadMaterials(Model& model, const aiScene& scene);
	static void LoadModelMesh(Model& model, const aiScene& scene);
};