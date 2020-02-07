/*
This example is supposed to load a nanosuit model using one compound buffer for all the meshes
*/

#include "helpers.hpp"



#include "glm/matrix.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "glt_Common.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <numeric>
#include <optional>

constexpr const char nanosuit[]{ "resources/objects/nanosuit/nanosuit.obj" },
vshader[]{ "shaders/nanosuit_simple.vs" },
fshader[]{ "shaders/nanosuit_simple.fs" };

using Uniforms = glt::uniform_collection<std::tuple<model_mat4,
    view_mat4,
    projection_mat4,
    texture_diffuse_sampler2D>>;

// would be nice if I could cast compound class to VertexData
/*
struct VertexData
{
    glm::vec3 posXYZ,
        normal,
        texCoords;
};*/

using VertexData = glt::compound<attrPosXYZ_vec3, attrNormals_vec3, attrTexCoords_vec3>;

using ProgModel = glt::Program<VAO_nanosuit_simple, Uniforms>;
using BufferMesh = glt::Buffer<VertexData>;
using BufferElems = glt::Buffer<unsigned int>;
using BufferTexture = glt::Buffer<unsigned char>;

using RefTexture2Drgba = std::reference_wrapper<glt::Texture2Drgba>;

glt::TexFormat translate_texture_format(const Image& im);

bool InitProgram(ProgModel& prog, const fsys::path& path);

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


	std::optional<RefTexture2Drgba> FindTexture(fsys::path p)
	{
		std::map<fsys::path, glt::Texture2Drgba>::iterator found =
			textures_.find(p);

		if (found == textures_.cend())
			return std::nullopt;

		return RefTexture2Drgba(found->second);
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

			std::optional<RefTexture2Drgba> loaded = cached.FindTexture(p);
			if (loaded)
				continue;

			Image im{ p };

			glt::Texture2Drgba tex;
			tex.Bind();

			tex.SetImage(0, im.Width(), im.Height());
			tex.SubImage(0, im.Width(), im.Height(), translate_texture_format(im),
				glt::TexType::unsigned_byte, im.Data());

			tex.GenerateMipMap();
			tex.UnBind();

			cached.textures_.emplace(std::make_pair(p, std::move(tex)));
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

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&& other)
		: name(std::move(other.name)),
		materialIndx_(other.materialIndx_),
		bufMesh(std::move(other.bufMesh)),
		bufElems(std::move(other.bufElems)),
		vao(std::move(other.vao))
	{}


	Mesh(const aiMesh& mesh)
		: name({ mesh.mName.C_Str(), mesh.mName.length })
	{
		materialIndx_ = mesh.mMaterialIndex;

		LoadMesh(mesh);
		assert(check_loaded_mesh(mesh));

		LoadElements(mesh);
		assert(check_load_elems(mesh));

		assert(glt::AssertGL());
	}

	// TODO: add material?
	void Draw(ProgModel::ProgGuard &pg)
	{
		// TODO: setup textures?
		vao.Bind();
		pg.DrawElements(bufElems, glt::RenderMode::triangles, bufElems().Allocated());
		vao.UnBind();
	}

	void LoadMesh(const aiMesh& mesh)
	{
		vao.Bind();

		bufMesh.Bind(glt::BufferTarget::array);
		bufMesh.AllocateMemory(mesh.mNumVertices, glt::BufUsage::static_draw);

		assert(bufMesh().Allocated() == mesh.mNumVertices && "Ranges mismatch!");

		size_t vertIndx = 0;

		for (VertexData& vD : glt::MapGuard(bufMesh(), glt::MapAccessBit::write))
		{

			attrPosXYZ_vec3& posXYZ = vD.Get(glt::tag_s<0>());
			attrNormals_vec3& norm = vD.Get(glt::tag_s<1>());
			attrTexCoords_vec3& texCoord = vD.Get(glt::tag_s<2>());

			const aiVector3D &aiPosXYZ = mesh.mVertices[vertIndx],
				&aiNorm = mesh.mNormals[vertIndx];

			posXYZ.glt_value.x = aiPosXYZ.x;
			posXYZ.glt_value.y = aiPosXYZ.y;
			posXYZ.glt_value.z = aiPosXYZ.z;

			norm.glt_value.x = aiNorm.x;
			norm.glt_value.y = aiNorm.y;
			norm.glt_value.z = aiNorm.z;

			assert(glt::AssertGL());

			if (!mesh.mTextureCoords[0])
			{
				texCoord.glt_value.x = 0;
				texCoord.glt_value.y = 0;
				texCoord.glt_value.z = 0;

				++vertIndx;
				continue;
			}

			texCoord.glt_value.x = mesh.mTextureCoords[0][vertIndx].x;
			texCoord.glt_value.y = mesh.mTextureCoords[0][vertIndx].y;
			texCoord.glt_value.z = mesh.mTextureCoords[0][vertIndx].z;

			++vertIndx;
		}

		vao.AttributePointer(bufMesh()());
		vao.AttributePointer(bufMesh()(glt::tag_s<1>()));
		vao.AttributePointer(bufMesh()(glt::tag_s<2>()));

		vao.EnablePointers();

		bufMesh.UnBind();
		vao.UnBind();
	}

	void LoadElements(const aiMesh& mesh)
	{
		std::vector<std::reference_wrapper<aiFace>> faces{ mesh.mFaces,
			std::next(mesh.mFaces, mesh.mNumFaces) };

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

		bufElems.UnBind();
	}

	private:

	bool check_loaded_mesh(const aiMesh& mesh)
	{
		bufMesh.Bind(glt::BufferTarget::array);

		assert(bufMesh().Allocated() == mesh.mNumVertices && "Ranges mismatch!");

		size_t vertIndx = 0;

		for (const VertexData& vD : glt::MapGuard(bufMesh(), glt::MapAccessBit::read))
		{
			const attrPosXYZ_vec3& posXYZ = vD.Get(glt::tag_s<0>());
			const attrNormals_vec3& norm = vD.Get(glt::tag_s<1>());
			const attrTexCoords_vec3& texCoord = vD.Get(glt::tag_s<2>());

			const aiVector3D &aiPosXYZ = mesh.mVertices[vertIndx],
				&aiNorm = mesh.mNormals[vertIndx];

			if (posXYZ.glt_value.x != aiPosXYZ.x ||
				posXYZ.glt_value.y != aiPosXYZ.y ||
				posXYZ.glt_value.z != aiPosXYZ.z ||

				norm.glt_value.x != aiNorm.x ||
				norm.glt_value.y != aiNorm.y ||
				norm.glt_value.z != aiNorm.z)
					return false;
			
			++vertIndx;
		}

		bufMesh.UnBind();

		return true;
	}

	bool check_load_elems(const aiMesh& mesh)
	{
		std::vector<std::reference_wrapper<aiFace>> faces{ mesh.mFaces,
			std::next(mesh.mFaces, mesh.mNumFaces) };

		bufElems.Bind(glt::BufferTarget::element_array);

		assert(bufElems().Allocated() == mesh.mNumFaces * 3 && "Elements ranges mismatch!");
		{
			glt::MapGuard elemsMap{ bufElems(), glt::MapAccessBit::read };

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

		bufElems.UnBind();
		return true;
	}
};

// model with compound buffer
struct Model
{
	static inline TexturesCached cached_textures_{};

	fsys::path path_;
	std::string name_;

	std::vector<Mesh> meshes_;
	std::vector<Material> materials_;

	Model(const fsys::path& path)
		: path_(path)
    {
    }

	void Draw(ProgModel::ProgGuard &pg)
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

	static void LoadMaterials(Model& model, const aiScene& scene);
    static void LoadModelMesh(Model& model, const aiScene& scene);
};


int main(int argc, const char **argv)
{
    fsys::path path = fsys::path(argv[0]).parent_path();

    SmartGLFW glfw{ 3, 3 };
    SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "Nanosuit simple model" };

    glfw.MakeContextCurrent(window);
    glfw.LoadOpenGL();

    glEnable(GL_DEPTH_TEST);

    ProgModel program;

    if (!InitProgram(program, path))
        return -1;

    Model mNanosuit{ fsys::path(path).append(nanosuit) };

    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(mNanosuit.path_.generic_string(),
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cerr << importer.GetErrorString() << std::endl;
            return -1;
        }

		Model::LoadMaterials(mNanosuit, *scene);
        Model::LoadModelMesh(mNanosuit, *scene);
    } 
   
	auto& cachedTextures = Model::cached_textures_;

    glt::is_equivalent_v<attrPosXYZ_vec3, glm::vec3>;

	glm::mat4 model = glm::mat4(1.0f),
		view = glm::mat4(1.0f),
		projection = glm::mat4(1.0f);




    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        
        glClearColor(0.258824f, 0.435294f, 0.258824f, 1);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		{
			view = glm::translate(glm::mat4(1.0f), glm::vec3(0, -10, -20));
			projection = glm::perspective(glm::radians(45.0f),
				(float)window.Width() / (float)window.Height(),
				0.1f, 100.0f);

			ProgModel::ProgGuard pg = program.Guard();
			ProgModel::uniforms& Uniforms = pg.Uniforms();

			Uniforms.Set(glt::glsl_cast<model_mat4>(model));
			Uniforms.Set(glt::glsl_cast<view_mat4>(view));
			Uniforms.Set(glt::glsl_cast<projection_mat4>(projection));

			mNanosuit.Draw(pg);

		}


        glfwSwapBuffers(window);
    }


    return 0;
}

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

bool InitProgram(ProgModel & prog, const fsys::path& path)
{
    std::ifstream file{ fsys::path(path).append(vshader), std::fstream::in };

    if (!file)
    {
        std::cerr << "Failed to open " << vshader << std::endl;
        return false;
    }

    std::string sourceVShader{ std::istreambuf_iterator(file),
    std::istreambuf_iterator<char>() };

    file.close();

    glt::VertexShader vs{ sourceVShader };

    if (!vs)
    {
        std::cerr << "Failed to compile vertex shader!" << std::endl;
        return false;
    }

    file.open(fsys::path(path).append(fshader), std::ifstream::in);
    if (!file)
    {
        std::cerr << "Failed to open " << fshader << std::endl;
        return false;
    }

    std::string sourceFShader{ std::istreambuf_iterator(file),
    std::istreambuf_iterator<char>() };

    file.close();

    glt::FragmentShader fs{ sourceFShader };
    if (!fs)
    {
        std::cerr << "Failed to compile fragment shader!" << std::endl;
        return false;
    }

    return prog.Link(vs, fs);;
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
