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

bool InitProgram(ProgModel& prog, const fsys::path& path);


struct Texture
{
	fsys::path path_;
	glt::Texture2D<glt::TexInternFormat::rgba> texture;
};

struct Mesh
{
    std::string name;
	
	BufferMesh bufMesh;
	BufferElems bufElems;

	ProgModel::vao vao;

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&& other)
		: name(std::move(other.name)),
		bufMesh(std::move(other.bufMesh)),
		bufElems(std::move(other.bufElems)),
		vao(std::move(other.vao))
	{}


	Mesh(const aiMesh& mesh)
		: name({ mesh.mName.C_Str(), mesh.mName.length })
	{
		LoadMesh(mesh);
		assert(check_loaded_mesh(mesh));

		LoadElements(mesh);
		assert(check_load_elems(mesh));

		assert(glt::AssertGL());
	}

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
	fsys::path path_;
	std::string name_;

	std::vector<Mesh> meshes_;
	std::map<fsys::path, BufferTexture> texBuffers_;

	std::vector<Texture> textures_;

	Model(const fsys::path& path)
		: path_(path)
    {
    }

	void Draw(ProgModel::ProgGuard &pg)
	{
		for (Mesh& m : meshes_)
			m.Draw(pg);
	}

	static void LoadMaterials(Model& model, const aiScene& scene);
    static void LoadModelMesh(Model& model, const aiScene& scene);
	static void LoadModelTextures(Model& model, const aiScene& scene);
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
		Model::LoadModelTextures(mNanosuit, *scene);

    } 
   
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
			view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3));
			projection = glm::perspective(glm::radians(45.0f),
				(float)window.Width() / (float)window.Height(),
				0.1f, 100.0f);

			ProgModel::ProgGuard pg = program.Guard();
			ProgModel::uniforms& Uniforms = pg.Uniforms();

			Uniforms.Set(glt::glsl_cast<view_mat4>(view));
			Uniforms.Set(glt::glsl_cast<projection_mat4>(projection));

			mNanosuit.Draw(pg);

		}


        glfwSwapBuffers(window);
    }


    return 0;
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

		std::vector<aiMaterialProperty*> props{ mat.mProperties,
			std::next(mat.mProperties, mat.mNumProperties) };

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

void Model::LoadModelTextures(Model & model, const aiScene & scene)
{

	



}
