#include "helpers.hpp"

#include "glm/matrix.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void glfw_error(int code, const char * msg)
{
    std::cout << "GLFW error: " << code <<
        ", " << msg << std::endl;
}

int main(int argc, char * argv[])
{

    fsys::path exePath{ argv[0] };
    std::cout << exePath.generic_string() << std::endl;

    SmartGLFW glfw{ 3, 3 };
    SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL" };

    glfw.MakeContextCurrent(window);
    glfw.LoadOpenGL();

    glfwSetErrorCallback(&glfw_error);


    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader{ exePath.parent_path().append("vshader.vs").generic_string().c_str(),
        exePath.parent_path().append("fshader.fs").generic_string().c_str() };

    GLuint bufPos = 0,
        bufTex = 0,
        vao = 0;
    
    glGenVertexArrays(1, &vao);

    glGenBuffers(1, &bufPos);
    glGenBuffers(1, &bufTex);

    assert(bufPos && bufTex && vao && "Failed to generate buffers or vao!");

    {
        glBindVertexArray(vao);

        std::vector<glm::vec3> positions = glm_cube_positions();

        glBindBuffer(GL_ARRAY_BUFFER, bufPos);
        glVertexAttribPointer(0, decltype(positions)::value_type::length(),
            GL_FLOAT, false, 0, 0);

        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), 
            nullptr, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3),
            positions.data());

        std::vector<glm::vec2> tex_coords = glm_cube_texCoords();

        glBindBuffer(GL_ARRAY_BUFFER, bufTex);
        glVertexAttribPointer(0, decltype(tex_coords)::value_type::length(),
            GL_FLOAT, false, 0, 0);

        glBufferData(GL_ARRAY_BUFFER, tex_coords.size() * sizeof(glm::vec2),
            nullptr, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, tex_coords.size() * sizeof(glm::vec2),
            tex_coords.data());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

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

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

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

        // activate shader
        ourShader.use();

        // create transformations
        glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // pass them to the shaders (3 different ways)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        ourShader.setMat4("projection", projection);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glDeleteBuffers(1, &bufPos);
    glDeleteBuffers(1, &bufTex);
    glDeleteVertexArrays(1, &vao);

    return 0;
}
