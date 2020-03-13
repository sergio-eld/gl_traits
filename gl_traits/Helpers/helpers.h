#pragma once

// TODO: remove non-used headers
#include <string>
#include <filesystem>
#include <iostream>
#include <vector>

namespace fsys = std::filesystem;

#include "GLFW/glfw3.h"


void processInput(GLFWwindow *window);

// settings
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;


class SmartGLFW
{
	GLFWwindow* currentContext_;

public:
	SmartGLFW(int verMajor, int verMinor);

    SmartGLFW(const SmartGLFW&) = delete;
    SmartGLFW& operator=(const SmartGLFW&) = delete;

	void MakeContextCurrent(GLFWwindow* window);
    GLFWglproc GetOpenGLLoader();

	~SmartGLFW();
};

class SmartGLFWwindow
{
	GLFWwindow* window_;

    unsigned int width_,
        height_;

public:
	SmartGLFWwindow(unsigned int width, unsigned int height,
		const std::string& title);

	operator GLFWwindow*() const;

    unsigned int Width() const
    {
        return width_;
    }

    unsigned int Height() const
    {
        return height_;
    }

	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

	~SmartGLFWwindow();
};

class Image
{
	int width_ = 0, 
		height_ = 0, 
		nrChannels_ = 0;

	unsigned char *data_;

public:
	Image(const std::string& path);
	Image(const std::filesystem::path& p);

	const unsigned char* Data() const;
	int Width() const;
	int Height() const;
	int NumChannels() const;

	~Image();
};
