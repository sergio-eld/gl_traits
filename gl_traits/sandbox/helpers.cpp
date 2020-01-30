#include "helpers.hpp"
#include "helpers.hpp"
//#include <Windows.h>

#include "helpers.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

SmartGLFW::SmartGLFW(int verMajor, int verMinor)
	: currentContext_(nullptr)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, verMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, verMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
}

void SmartGLFW::MakeContextCurrent(GLFWwindow * window)
{
	glfwMakeContextCurrent(window);
	currentContext_ = window;
}

void SmartGLFW::LoadOpenGL()
{
	assert(currentContext_ && "No current context has been set!");
	bool loaded = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	assert(loaded && "failed to load GLAD");
	if (!loaded)
		throw std::exception{ "Failed to initialize GLAD" };
}

SmartGLFW::~SmartGLFW()
{
	glfwTerminate();
}

SmartGLFWwindow::SmartGLFWwindow(unsigned int width, unsigned int height, const std::string & title)
	: window_(glfwCreateWindow(width, height, title.data(), nullptr, nullptr)),
    width_(width),
    height_(height)
{
	assert(window_ && "Window is nullptr! Check if GLFW is initialized");
	glfwSetFramebufferSizeCallback(window_, &SmartGLFWwindow::FrameBufferSizeCallback);
	//throw if window is nullptr
}

SmartGLFWwindow::operator GLFWwindow*() const
{
	return window_;
}

void SmartGLFWwindow::FrameBufferSizeCallback(GLFWwindow * window, int width, int height)
{
    // this is illegal (shush)
    reinterpret_cast<SmartGLFWwindow*>(window)->width_ = width;
    reinterpret_cast<SmartGLFWwindow*>(window)->height_ = height;

	glViewport(0, 0, width, height);
}

SmartGLFWwindow::~SmartGLFWwindow()
{
	glfwDestroyWindow(window_);
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

std::vector<posVec3> posVec3_cube_positions()
{
    std::vector<glm::vec3> positions = glm_cube_positions();
    std::vector<posVec3> out{ positions.size() };
    for (size_t i = 0; i != positions.size(); ++i)
        out[i].coord = positions[i];

    return out;
}

std::vector<posXYZf> posXYZf_cube_positions()
{
    std::vector<glm::vec3> positions = glm_cube_positions();
    std::vector<posXYZf> out{ positions.size() };
    for (size_t i = 0; i != positions.size(); ++i)
    {
        out[i].x = positions[i].x;
        out[i].y = positions[i].y;
        out[i].z = positions[i].z;
    }

    return out;
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

std::vector<vertex> cube_vertices()
{
	auto positions = glm_cube_positions();
	auto texCoords = glm_cube_texCoords();

	std::vector<vertex> out;

	for (size_t i = 0; i != positions.size(); ++i)
		out.push_back(vertex{ positions[i], texCoords[i] });
	return out;
}

Image::Image(const std::string& path)
	: data_(stbi_load(path.c_str(), &width_, &height_, &nrChannels_, 0))
{
}

Image::Image(const std::filesystem::path & p)
	: Image(p.generic_string())
{}

const unsigned char * Image::Data() const
{
	return data_;
}

int Image::Width() const
{
	return width_;
}

int Image::Height() const
{
	return height_;
}

int Image::NumChannels() const
{
	return nrChannels_;
}

Image::~Image()
{
	stbi_image_free(data_);
}
