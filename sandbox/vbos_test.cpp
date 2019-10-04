

#define GL_TRAITS_STATIC
#include "gl_traits.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GLFW/glfw3.h"
#include "learnopengl/shader_m.h"

#include <filesystem>
//#include "learnopengl/filesystem.h"

#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);



// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

struct vertex
{
	glm::vec3 posCoords;
	glm::vec2 textureCoords;

};

std::vector<glm::vec3> glm_cube_positions();
std::vector<glm::vec2> glm_cube_texCoords();

int main()
{
	std::filesystem::path path{ GetFullPath("") };

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader{ (path.generic_string() + "vshader.vs").c_str(), (path.generic_string() + "fshader.fs").c_str() };

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  /*0.0f, 0.0f,*/
		 0.5f, -0.5f, -0.5f,  /*1.0f, 0.0f,*/
		 0.5f,  0.5f, -0.5f,  /*1.0f, 1.0f,*/
		 0.5f,  0.5f, -0.5f,  /*1.0f, 1.0f,*/
		-0.5f,  0.5f, -0.5f,  /*0.0f, 1.0f,*/
		-0.5f, -0.5f, -0.5f,  /*0.0f, 0.0f,*/

		-0.5f, -0.5f,  0.5f,  /*0.0f, 0.0f,*/
		 0.5f, -0.5f,  0.5f,  /*1.0f, 0.0f,*/
		 0.5f,  0.5f,  0.5f,  /*1.0f, 1.0f,*/
		 0.5f,  0.5f,  0.5f,  /*1.0f, 1.0f,*/
		-0.5f,  0.5f,  0.5f,  /*0.0f, 1.0f,*/
		-0.5f, -0.5f,  0.5f,  /*0.0f, 0.0f,*/

		-0.5f,  0.5f,  0.5f,  /*1.0f, 0.0f,*/
		-0.5f,  0.5f, -0.5f,  /*1.0f, 1.0f,*/
		-0.5f, -0.5f, -0.5f,  /*0.0f, 1.0f,*/
		-0.5f, -0.5f, -0.5f,  /*0.0f, 1.0f,*/
		-0.5f, -0.5f,  0.5f,  /*0.0f, 0.0f,*/
		-0.5f,  0.5f,  0.5f,  /*1.0f, 0.0f,*/

		 0.5f,  0.5f,  0.5f,  /*1.0f, 0.0f,*/
		 0.5f,  0.5f, -0.5f,  /*1.0f, 1.0f,*/
		 0.5f, -0.5f, -0.5f,  /*0.0f, 1.0f,*/
		 0.5f, -0.5f, -0.5f,  /*0.0f, 1.0f,*/
		 0.5f, -0.5f,  0.5f,  /*0.0f, 0.0f,*/
		 0.5f,  0.5f,  0.5f,  /*1.0f, 0.0f,*/

		-0.5f, -0.5f, -0.5f,  /*0.0f, 1.0f,*/
		 0.5f, -0.5f, -0.5f,  /*1.0f, 1.0f,*/
		 0.5f, -0.5f,  0.5f,  /*1.0f, 0.0f,*/
		 0.5f, -0.5f,  0.5f,  /*1.0f, 0.0f,*/
		-0.5f, -0.5f,  0.5f,  /*0.0f, 0.0f,*/
		-0.5f, -0.5f, -0.5f,  /*0.0f, 1.0f,*/

		-0.5f,  0.5f, -0.5f,  /*0.0f, 1.0f,*/
		 0.5f,  0.5f, -0.5f,  /*1.0f, 1.0f,*/
		 0.5f,  0.5f,  0.5f,  /*1.0f, 0.0f,*/
		 0.5f,  0.5f,  0.5f,  /*1.0f, 0.0f,*/
		-0.5f,  0.5f,  0.5f,  /*0.0f, 0.0f,*/
		-0.5f,  0.5f, -0.5f  /*0.0f, 1.0f*/
	};

	

	float texCoords[] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,

		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,

		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		 1.0f, 0.0f,
		 1.0f, 1.0f,
		 0.0f, 1.0f,
		 0.0f, 1.0f,
		 0.0f, 0.0f,
		 1.0f, 0.0f,

		0.0f, 1.0f,
		 1.0f, 1.0f,
		1.0f, 0.0f,
		 1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,

		0.0f, 1.0f,
		 1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
	};

	// TODO: add queries of active buffer to glt_buffers
	GLint res;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &res);

	// batched typesafe vbo (use case 2.1)
	glVBO<glm::vec3, glm::vec2> batchedVBO{
		glt_buffers::GenBuffer<glTargetBuf::array_buffer>() };

	batchedVBO.Bind();
	batchedVBO.AllocateMemory(36, 36, gltBufUse::static_draw);

	size_t offset1 = batchedVBO.GetOffset<1>();
	batchedVBO.BufferData<0>(glm_cube_positions());
	batchedVBO.BufferData<1>(glm_cube_texCoords());

	// use case (batched buffers)
	gltHandle<glVAO::vao> vao_t = glt_buffers::GenVAO();
	glt_buffers::BindVAO(vao_t);
	
	// position attribute

	glt_buffers::VertexAttribPointer(glm::vec3(), vao_t, 0, 0, 0);
	glt_buffers::EnableVertexAttribArray(vao_t, 0);

	// texture coord attribute
	glt_buffers::VertexAttribPointer(glm::vec2(), vao_t, 1, 0, sizeof(vertices));
	glt_buffers::EnableVertexAttribArray(vao_t, 1);
	//glt_buffers::VertexAttribPointer(glslt_compound<glm::vec3, glm::vec2>(), vao_t, 0, 0,
	//	std::make_tuple(false, false));


	// load and create a texture 
	// -------------------------
	unsigned int texture1, texture2;
	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

	
	unsigned char *data = stbi_load((path.generic_string() + "resources/textures/container.jpg").c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	// texture 2
	// ---------
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	data = stbi_load((path.generic_string() + "resources/textures/awesomeface.png").c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		// note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use();
	ourShader.setInt("texture1", 0);
	ourShader.setInt("texture2", 1);


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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		// activate shader
		ourShader.use();

		// create transformations
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		// retrieve the matrix uniform locations
		unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		ourShader.setMat4("projection", projection);

		// render box
		glBindVertexArray(vao_t);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

std::vector<glm::vec3> glm_cube_positions()
{
	return std::vector<glm::vec3>{
		glm::vec3{ -0.5f, -0.5f, -0.5f },
			glm::vec3{ 0.5f, -0.5f, -0.5f },
			glm::vec3{ 0.5f,  0.5f, -0.5f },
			glm::vec3{ 0.5f,  0.5f, -0.5f },
			glm::vec3{ -0.5f,  0.5f, -0.5f },
			glm::vec3{ -0.5f, -0.5f, -0.5f },

			glm::vec3{ -0.5f, -0.5f,  0.5f },
			glm::vec3{ 0.5f, -0.5f,  0.5f },
			glm::vec3{ 0.5f,  0.5f,  0.5f },
			glm::vec3{ 0.5f,  0.5f,  0.5f },
			glm::vec3{ -0.5f,  0.5f,  0.5f },
			glm::vec3{ -0.5f, -0.5f,  0.5f },

			glm::vec3{ -0.5f,  0.5f,  0.5f },
			glm::vec3{ -0.5f,  0.5f, -0.5f },
			glm::vec3{ -0.5f, -0.5f, -0.5f },
			glm::vec3{ -0.5f, -0.5f, -0.5f },
			glm::vec3{ -0.5f, -0.5f,  0.5f },
			glm::vec3{ -0.5f,  0.5f,  0.5f },

			glm::vec3{ 0.5f,  0.5f,  0.5f },
			glm::vec3{ 0.5f,  0.5f, -0.5f },
			glm::vec3{ 0.5f, -0.5f, -0.5f },
			glm::vec3{ 0.5f, -0.5f, -0.5f },
			glm::vec3{ 0.5f, -0.5f,  0.5f },
			glm::vec3{ 0.5f,  0.5f,  0.5f },

			glm::vec3{ -0.5f, -0.5f, -0.5f },
			glm::vec3{ 0.5f, -0.5f, -0.5f },
			glm::vec3{ 0.5f, -0.5f,  0.5f },
			glm::vec3{ 0.5f, -0.5f,  0.5f },
			glm::vec3{ -0.5f, -0.5f,  0.5f },
			glm::vec3{ -0.5f, -0.5f, -0.5f },

			glm::vec3{ -0.5f,  0.5f, -0.5f },
			glm::vec3{ 0.5f,  0.5f, -0.5f },
			glm::vec3{ 0.5f,  0.5f,  0.5f },
			glm::vec3{ 0.5f,  0.5f,  0.5f },
			glm::vec3{ -0.5f,  0.5f,  0.5f },
			glm::vec3{ -0.5f,  0.5f, -0.5f }
	};
}

std::vector<glm::vec2> glm_cube_texCoords()
{
	return std::vector<glm::vec2>{

			glm::vec2{ 0.0f, 0.0f },
			glm::vec2{ 1.0f, 0.0f },
			glm::vec2{ 1.0f, 1.0f },
			glm::vec2{ 1.0f, 1.0f },
			glm::vec2{ 0.0f, 1.0f },
			glm::vec2{ 0.0f, 0.0f },

			glm::vec2{ 0.0f, 0.0f },
			glm::vec2{ 1.0f, 0.0f },
			glm::vec2{ 1.0f, 1.0f },
			glm::vec2{ 1.0f, 1.0f },
			glm::vec2{ 0.0f, 1.0f },
			glm::vec2{ 0.0f, 0.0f },

			glm::vec2{ 1.0f, 0.0f },
			glm::vec2{ 1.0f, 1.0f },
			glm::vec2{ 0.0f, 1.0f },
			glm::vec2{ 0.0f, 1.0f },
			glm::vec2{ 0.0f, 0.0f },
			glm::vec2{ 1.0f, 0.0f },

			glm::vec2{ 1.0f, 0.0f },
			glm::vec2{ 1.0f, 1.0f },
			glm::vec2{ 0.0f, 1.0f },
			glm::vec2{ 0.0f, 1.0f },
			glm::vec2{ 0.0f, 0.0f },
			glm::vec2{ 1.0f, 0.0f },

			glm::vec2{ 0.0f, 1.0f },
			glm::vec2{ 1.0f, 1.0f },
			glm::vec2{ 1.0f, 0.0f },
			glm::vec2{ 1.0f, 0.0f },
			glm::vec2{ 0.0f, 0.0f },
			glm::vec2{ 0.0f, 1.0f },

			glm::vec2{ 0.0f, 1.0f },
			glm::vec2{ 1.0f, 1.0f },
			glm::vec2{ 1.0f, 0.0f },
			glm::vec2{ 1.0f, 0.0f },
			glm::vec2{ 0.0f, 0.0f },
			glm::vec2{ 0.0f, 1.0f }
	};
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}