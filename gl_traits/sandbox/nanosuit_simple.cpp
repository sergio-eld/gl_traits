/*
This example is supposed to load a nanosuit model using one compound buffer for all the meshes
*/

#include "helpers.hpp"

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

bool InitProgram(ProgModel& prog, const fsys::path& path);



struct Mesh
{
    std::string name;

    size_t startIndex,
        elemsCount;
};

// model with compound buffer
struct Model
{
    fsys::path path_;

    std::string name_;

    BufferMesh bufMeshes;
    BufferElems bufElems;

    ProgModel::vao vao;

    std::vector<Mesh> meshes;

    Model(const fsys::path& path)
        : path_(path)
    {

    }

    static void LoadModelMesh(Model& model, const aiScene& scene);

};

void LoadModelMesh(Model& model, const fsys::path& path);

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

        Model::LoadModelMesh(mNanosuit, *scene);

    } 
   
    glt::is_equivalent_v<attrPosXYZ_vec3, glm::vec3>;


    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        
        glClearColor(0.258824f, 0.435294f, 0.258824f, 1);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


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

void LoadModelMesh(Model& model, const fsys::path& path)
{
    
    /*
    std::vector<aiMaterial*> materials{ scene->mMaterials, std::next(scene->mMaterials, scene->mNumMaterials) };
    std::vector<aiNode*> childNodes{ scene->mRootNode->mChildren, 
        std::next(scene->mRootNode->mChildren, scene->mRootNode->mNumChildren) };

    size_t vertices = 0,
        texCoords = 0,
        indices = 0;

    GLenum(GL_OUT_OF_MEMORY);

    vertices = std::accumulate(meshes.cbegin(), meshes.cend(), vertices,
        [](size_t first, aiMesh *mesh)
    {
        return first + mesh->mNumVertices;
    });

    texCoords = std::accumulate(meshes.cbegin(), meshes.cend(), texCoords,
        [](size_t first, aiMesh *mesh)
    {
        return first + mesh->mNumUVComponents[0];
    });

    for (aiMesh *mesh : meshes)
    {
        Mesh m{ indices, mesh->mNumFaces * 3 };
        indices += m.elemsCount;
        model.meshes.push_back(m);

        // all are triangles
        // std::vector<std::reference_wrapper<aiFace>> faces{ mesh->mFaces, std::next(mesh->mFaces, mesh->mNumFaces) };
    }

    model.bufMeshes.Bind(glt::BufferTarget::array);
    model.bufElems.Bind(glt::BufferTarget::element_array);

    model.bufMeshes.AllocateMemory(vertices, vertices, texCoords, glt::BufUsage::static_draw);
    model.bufElems.AllocateMemory(indices, glt::BufUsage::static_draw);
    
    GLenum error = glGetError();

    assert(!error && "OpenGL error received!");
    if (error)
        throw std::exception(std::to_string(error).c_str());

*/

}

void Model::LoadModelMesh(Model& model, const aiScene& scene)
{
    std::vector<aiMesh*> meshes{ scene.mMeshes, std::next(scene.mMeshes, scene.mNumMeshes) };

    size_t vertices = 0,
        elems = 0;

    for (aiMesh * const m : meshes)
    {
        vertices += m->mNumVertices;

        size_t elemsTemp = elems;
        elems += m->mNumFaces * 3;

        model.meshes.push_back(Mesh{ 
            std::string(m->mName.C_Str(), m->mName.length), 
            elemsTemp,
            elems 
            });

    }

    ProgModel::vao& vao = model.vao;
    vao.Bind();

    BufferMesh& bufMesh = model.bufMeshes;
    BufferElems& bufElems = model.bufElems;

    bufMesh.Bind(glt::BufferTarget::array);
    bufMesh.AllocateMemory(vertices, glt::BufUsage::static_draw);

    bufElems.Bind(glt::BufferTarget::element_array);
    bufElems.AllocateMemory(elems, glt::BufUsage::static_draw);

    {
        glt::MapGuard mapBufMesh{ bufMesh(), glt::MapAccessBit::write };
        glt::MapGuard mapBufElems{ bufElems(), glt::MapAccessBit::write };

        decltype(mapBufMesh)::iterator iterBufMesh = mapBufMesh.begin();
        decltype(mapBufElems)::iterator iterBufElems = mapBufElems.begin();

        for (aiMesh * mesh : meshes)
        {
            for (size_t v = 0; v != mesh->mNumVertices; ++v)
            {
                attrPosXYZ_vec3& posXYZ = (*iterBufMesh).Get(glt::tag_s<0>());
                attrNormals_vec3& normal = (*iterBufMesh).Get(glt::tag_s<1>());
                attrTexCoords_vec3& texture = (*iterBufMesh).Get(glt::tag_s<2>());

                posXYZ.glt_value.x = mesh->mVertices[v].x;
                posXYZ.glt_value.y = mesh->mVertices[v].y;
                posXYZ.glt_value.z = mesh->mVertices[v].z;

                normal.glt_value.x = mesh->mNormals[v].x;
                normal.glt_value.y = mesh->mNormals[v].y;
                normal.glt_value.z = mesh->mNormals[v].z;

                if (mesh->mTextureCoords[0])
                {
                    texture.glt_value.x = mesh->mTextureCoords[0]->x;
                    texture.glt_value.y = mesh->mTextureCoords[0]->y;
                    texture.glt_value.z = mesh->mTextureCoords[0]->z;
                }
                else
                {
                    texture.glt_value.x = 0;
                    texture.glt_value.y = 0;
                    texture.glt_value.z = 0;
                }

                ++iterBufMesh;
                assert(glt::AssertGL());
            }

            std::vector<std::reference_wrapper<aiFace>> faces{ mesh->mFaces, 
                std::next(mesh->mFaces, mesh->mNumFaces) };

            for (const aiFace& f : faces)
            {
                for (size_t i = 0; i != f.mNumIndices; ++i)
                {
                    unsigned int& elemIndx = *iterBufElems;
                    elemIndx = f.mIndices[i]; // add model's Mesh offset (starting index)

                    ++iterBufElems;
                }
            }
        }

        // ranges eqal
        assert(iterBufMesh == mapBufMesh.end());
        assert(iterBufElems == mapBufElems.end());
    }


    vao.AttributePointer(bufMesh()());
    vao.AttributePointer(bufMesh()(glt::tag_s<1>()));
    vao.AttributePointer(bufMesh()(glt::tag_s<2>()));

    vao.EnablePointers();

    bufMesh.UnBind();
    bufElems.UnBind();

    vao.UnBind();

}
