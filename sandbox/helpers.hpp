#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <vector>

#define GL_TRAITS_STATIC
#include "gl_traits.hpp"

#include "GLFW/glfw3.h"
#include "learnopengl/shader_m.h"

void processInput(GLFWwindow *window);

// settings
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

extern const std::filesystem::path path;


std::string GetFullPath(const std::string& pathRelative);

class SmartGLFW
{
	GLFWwindow* currentContext_;

public:
	SmartGLFW(int verMajor, int verMinor);

    SmartGLFW(const SmartGLFW&) = delete;
    SmartGLFW& operator=(const SmartGLFW&) = delete;

	void MakeContextCurrent(GLFWwindow* window);
	void LoadOpenGL();

	~SmartGLFW();
};

class SmartGLFWwindow
{
	GLFWwindow* window_;

public:
	SmartGLFWwindow(unsigned int width, unsigned int height,
		const std::string& title);

	operator GLFWwindow*() const;

	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

	~SmartGLFWwindow();
};

class Image
{
	int width_, height_, nrChannels_;
	unsigned char *data_;

public:
	Image(const std::string& path);

	const unsigned char* Data() const;
	int Width() const;
	int Height() const;
	int NumChannels() const;

	~Image();
};

// compound user-defined type
// TODO: add case for inversed parameters (first field - textures). Must not compile
struct vertex
{
	glm::vec3 posCoords;
	glm::vec2 textureCoords;

	bool operator==(const vertex& other) const
	{
		return posCoords == other.posCoords &&
			textureCoords == other.textureCoords;
	}

	bool operator!=(const vertex& other) const
	{
		return !operator==(other);
	}

};

std::vector<glm::vec3> glm_cube_positions();
std::vector<glm::vec2> glm_cube_texCoords();
std::vector<vertex> cube_vertexes();
