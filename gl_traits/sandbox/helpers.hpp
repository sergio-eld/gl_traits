#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <vector>

namespace fsys = std::filesystem;

#define GL_TRAITS_STATIC
#include "gl_traits.hpp"


#include "GLFW/glfw3.h"
#include "learnopengl/shader_m.h"

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
	void LoadOpenGL();

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

// compound user-defined type
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

struct vertexR
{
	glm::vec2 textureCoords;
	glm::vec3 posCoords;

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

struct posVec3
{
    glm::vec3 coord;

    bool operator==(const posVec3& other) const
    {
        return coord == other.coord;
    }

    bool operator!=(const posVec3& other) const
    {
        return !(*this == other);
    }
};

struct posXYZf
{
    float x,
        y,
        z;

    bool operator==(const posXYZf& other) const
    {
        return std::tie(x, y, z) ==
            std::tie(other.x, other.y, other.z);
    }

    bool operator!=(const posXYZf& other) const
    {
        return !(*this == other);
    }
};


std::vector<glm::vec3> glm_cube_positions();
std::vector<posVec3> posVec3_cube_positions();
std::vector<posXYZf> posXYZf_cube_positions();

std::vector<glm::vec2> glm_cube_texCoords();
std::vector<vertex> cube_vertices();
