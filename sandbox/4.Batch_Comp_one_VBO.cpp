﻿#include "helpers.hpp"

#include "glm/matrix.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

int main(int argc, char * argv[])
{
	fsys::path exePath{ argv[0] };
	std::cout << exePath.generic_string() << std::endl;

	SmartGLFW glfw{ 3, 3 };
	SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL" };

	glfw.MakeContextCurrent(window);
	glfw.LoadOpenGL();

	// build and compile our shader program
	// ------------------------------------
	Shader ourShader{ exePath.parent_path().append("vshader.vs").generic_string().c_str(),
		exePath.parent_path().append("fshader.fs").generic_string().c_str() };

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

	//vboVert().SubData(vertices.data(), vertices.size());

	vboVert.UnBind();

	// TODO: use EnablePointers()
	vao.EnablePointers();


	/////////////////////////////////////////////////////////////////////
	// The rest part is identical to other use cases
	/////////////////////////////////////////////////////////////////////

	glEnable(GL_DEPTH_TEST);

	// load and create textures 
	unsigned int texture1, texture2;
	{
		Image tex1{ exePath.parent_path().append("resources/textures/container.jpg").generic_string() },
			tex2{ exePath.parent_path().append("resources/textures/awesomeface.png").generic_string() };

		assert(tex1.Data() && tex2.Data());

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

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex1.Width(), tex1.Height(),
			0, GL_RGB, GL_UNSIGNED_BYTE, tex1.Data());
		glGenerateMipmap(GL_TEXTURE_2D);

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

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex2.Width(), tex2.Height(),
			0, GL_RGBA, GL_UNSIGNED_BYTE, tex2.Data());
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	ourShader.use();
	ourShader.setInt("texture1", 0);
	ourShader.setInt("texture2", 1);

	// retrieve the matrix uniform locations
	unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
	unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");

	// create transformations
	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		// activate shader
		ourShader.use();

		model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
		view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		ourShader.setMat4("projection", projection);

		// render box
		//vao.Bind();
		// glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	return 0;

}