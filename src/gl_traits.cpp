#include "gl_traits.hpp"

std::array<std::pair<glTargetTex, const void*>, gltActiveTexture::max_gl_textures>
	gltActiveTexture::textureUnits_{};
size_t gltActiveTexture::currentUnit_ = 0;

VAO_base::VAO_base()
{
    glGenVertexArrays(1, &handle_);
    assert(handle_ && "Failed to generate VAO!");
}

VAO_base::VAO_base(VAO_base && other)
    : handle_(other.handle_)
{
    other.handle_ = 0;
}

VAO_base & VAO_base::operator=(VAO_base &&other)
{
    handle_ = other.handle_;
    other.handle_ = 0;
    return *this;
}

bool VAO_base::IsCurrent() const
{
    if (!handle_)
        return false;
    return vaoCurrent_ == handle_;
}

void VAO_base::Bind()
{
    glBindVertexArray(handle_);
    vaoCurrent_ = handle_;
}

void VAO_base::UnBind()
{
    assert(IsCurrent() && "Trying to unbind non-active VAO!");
    glBindVertexArray(0);
}

VAO_base::~VAO_base()
{
    if (IsCurrent())
        UnBind();
    if (handle_)
        glDeleteVertexArrays(1, &handle_);
}

GLuint VAO_base::vaoCurrent_ = 0;
