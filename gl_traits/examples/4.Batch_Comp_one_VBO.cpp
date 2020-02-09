#include "gl_traits.hpp"
#include "helpers.h"
#include "cube_geometry.hpp"

#include "glt_Common.h"
#include "glt_CommonValidate.h"

#include <fstream>

#include "glm/matrix.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


using unif_collect = glt::uniform_collection<std::tuple<model_mat4,
    view_mat4,
    projection_mat4,
    texture1_sampler2D,
    texture2_sampler2D>>;

const char vertexSrcFile[] = "shaders/vshader.vs",
fragmentSrcFile[] = "shaders/fshader.fs";

int main(int argc, char * argv[])
{
	fsys::path exePath{ argv[0] };
	std::cout << exePath.generic_string() << std::endl;

	SmartGLFW glfw{ 3, 3 };
	SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL" };

	glfw.MakeContextCurrent(window);
    glt::LoadOpenGL(glfw.GetOpenGLLoader());

    glt::VertexShader vs{};
    glt::FragmentShader fs{};
    glt::Program<VAO_vshader, unif_collect> prog{};

    {
        std::fstream file{ exePath.parent_path().append(vertexSrcFile), std::fstream::in };
        if (!file)
        {
            std::cerr << "Failed to open " << vertexSrcFile << std::endl;
            return -1;
        }
        std::string vShaderSource{ std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>() };

        file.close();

        file.open(exePath.parent_path().append(fragmentSrcFile), std::fstream::in);
        if (!file)
        {
            std::cerr << "Failed to open " << fragmentSrcFile << std::endl;
            return -1;
        }

        std::string fShaderSource{ std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>() };

        if (!vs.Compile(vShaderSource))
            std::cerr << "Failed to compile vertex shader!" << std::endl;

        if (!fs.Compile(fShaderSource))
            std::cerr << "Failed to compile vertex shader!" << std::endl;

        if (!vs && !fs)
        {
            assert(vs && "Failed to compile vertex shader!");
            assert(fs && "Failed to compile fragment shader!");

            std::cerr << "Failed to compile shaders!" << std::endl;
            return -1;
        }
    }

    if (!prog.Link(vs, fs))
    {
        assert(prog && "Failed to link shader program!");
        std::cerr << "Failed to link shader program!" << std::endl;
        return -1;
    }

	glt::VAO<glm::vec3, glm::vec2> vao{};
	vao.Bind();

	std::vector<glm::vec3> positions = glm_cube_positions();
	std::vector<glm::vec2> tex_coords = glm_cube_texCoords();

	glt::Buffer<glm::vec2, glt::compound<float, glm::vec3, int>> vboVert{};
	vboVert.Bind(glt::BufferTarget::array);

	vboVert.AllocateMemory(tex_coords.size(), positions.size(),
		glt::BufUsage::static_draw);

	vao.AttributePointer(glt::tag_s<0>(), 
		vboVert(glt::tag_s<1>()).AttribPointer(glt::tag_s<1>()));
	vao.AttributePointer(glt::tag_s<1>(), vboVert().AttribPointer());
	
	vboVert().SubData(tex_coords.data(), tex_coords.size());

	struct dummy
	{
		float f;
		glm::vec3 xyz;
		int i;
	};

	for (dummy& d : glt::MapGuard(vboVert(glt::tag_s<1>()), glt::MapAccessBit::write))
	{
		static std::vector<glm::vec3>::const_iterator posIter = positions.cbegin();
		d = dummy{ float(), *posIter++, int() };
	}

	for (const dummy& d : glt::MapGuard(vboVert(glt::tag_s<1>()), glt::MapAccessBit::read))
	{
		static std::vector<glm::vec3>::const_iterator posIter = positions.cbegin();
		assert(d.xyz == *posIter && "Failed to load position coords!");
		++posIter;
	}

	vboVert.UnBind();

	vao.EnablePointers();

	glEnable(GL_DEPTH_TEST);

	glt::Texture2D<glt::TexInternFormat::rgba> texture1,
		texture2;

	// TODO: load one of the textures using glt Buffer
	{
		Image tex1{ exePath.parent_path().append("resources/textures/container.jpg").generic_string() },
			tex2{ exePath.parent_path().append("resources/textures/awesomeface.png").generic_string() };

		assert(tex1.Data() && tex2.Data());

		glActiveTexture(GL_TEXTURE0);
		texture1.Bind();

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		texture1.SetImage(0, tex1.Width(), tex1.Height());
		texture1.SubImage(0, tex1.Width(), tex1.Height(), glt::TexFormat::rgb,
			glt::TexType::unsigned_byte, tex1.Data());

		texture1.GenerateMipMap();


		glActiveTexture(GL_TEXTURE1);
		texture2.Bind();

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		texture2.SetImage(0, tex2.Width(), tex2.Height());
		texture2.SubImage(0, tex2.Width(), tex2.Height(), glt::TexFormat::rgba,
			glt::TexType::unsigned_byte, tex2.Data());

		texture2.GenerateMipMap();

		glActiveTexture(GL_TEXTURE3);
		texture2.UnBind();
	}
    
    prog.Use();
    prog.Set(texture1_sampler2D{ 0 });
    prog.Set(texture2_sampler2D{ 1 });
    prog.UnUse();

	// create transformations
	glm::mat4 model = glm::mat4(1.0f); 
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);


	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        projection = glm::perspective(glm::radians(45.0f), (float)window.Width() / (float)window.Height(), 0.1f, 100.0f);

        {
            decltype(prog)::ProgGuard guard = prog.Guard();

            guard.Uniforms().Set(glt::glsl_cast<model_mat4>(model));
            guard.Uniforms().Set(glt::glsl_cast<view_mat4>(view));
            guard.Uniforms().Set(glt::glsl_cast<projection_mat4>(projection));

            decltype(prog)::uniforms& uniforms = guard.Uniforms();

            assert(uniforms.Uniform(glt::tag_t<model_mat4>()).Get() == model);
            assert(uniforms.Uniform(glt::tag_t<view_mat4>()).Get() == view);
            assert(uniforms.Uniform(glt::tag_t<projection_mat4>()).Get() == projection);
            assert(uniforms.Uniform(glt::tag_t<texture1_sampler2D>()).Get() == 0);
            assert(uniforms.Uniform(glt::tag_t<texture2_sampler2D>()).Get() == 1);

            guard.DrawTriangles(vao, 0, positions.size());

        }

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;

}