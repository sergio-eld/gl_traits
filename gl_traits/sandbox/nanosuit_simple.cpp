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

using ProgModel = glt::Program<VAO_nanosuit_simple, Uniforms>;
using BufferMesh = glt::Buffer<attrPosXYZ_vec3, attrNormals_vec3, attrTexCoords_vec3>;
using BufferElems = glt::Buffer<unsigned int>;

struct Mesh
{
    size_t startIndex,
        elemsCount;
};

struct Model
{
    BufferMesh bufMeshes;
    BufferElems bufElems;

    ProgModel::vao vao;

    std::vector<Mesh> meshes;
};

bool InitProgram(ProgModel& prog, const fsys::path& path);
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


    Model mNanosuit;

    try
    {
        LoadModelMesh(mNanosuit, fsys::path(path).append(nanosuit));
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
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
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path.generic_string(),
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        throw std::exception(importer.GetErrorString());

    std::vector<aiMesh*> meshes{ scene->mMeshes, std::next(scene->mMeshes, scene->mNumMeshes) };
    std::vector<aiMaterial*> materials{ scene->mMaterials, std::next(scene->mMaterials, scene->mNumMaterials) };

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



}
