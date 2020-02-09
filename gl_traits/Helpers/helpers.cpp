#include "helpers.h"

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

GLFWglproc SmartGLFW::GetOpenGLLoader()
{
    return (GLFWglproc)&glfwGetProcAddress;
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
