/*
This example is supposed to load a nanosuit model using one compound buffer for all the meshes
*/

#include "helpers.hpp"

#include "assimp_model.h"

#include "glm/matrix.hpp"
#include "glm/gtc/matrix_transform.hpp"


constexpr const char nanosuit[]{ "resources/objects/nanosuit/nanosuit.obj" },
vshader[]{ "shaders/nanosuit_simple.vs" },
fshader[]{ "shaders/nanosuit_simple.fs" };



bool InitProgram(ProgModel& prog, const fsys::path& path);


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

