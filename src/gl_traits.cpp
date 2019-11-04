#include "gl_traits.hpp"

// 

const gltHandle<glVAO::vao> *glt_buffers::currentVAO_ = nullptr;

/*

std::array<std::pair<glTargetTex, const void*>, gltActiveTexture::max_gl_textures>
	gltActiveTexture::textureUnits_{};
size_t gltActiveTexture::currentUnit_ = 0;




//templates instantiations

void gl_debug::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}
*/