/**
 * \file
 * \author Rudy Castan
 * \author Ginam Park
 * \date 2025 Spring
 * \par CS250 Computer Graphics II
 * \copyright DigiPen Institute of Technology
 */

#include "IndexBuffer.hpp"

#include "GL.hpp"
#include <GL/glew.h>

namespace
{
    opengl::Handle create_indices_buffer(const void* data, GLsizei size_bytes)
    {
        GLuint handle = 0;
        GL::GenBuffers(1, &handle);
        GL::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        GL::BufferData(GL_ELEMENT_ARRAY_BUFFER, size_bytes, data, GL_STATIC_DRAW);
        GL::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return opengl::Handle{handle};
    }
}

namespace opengl
{

    IndexBuffer::IndexBuffer(std::span<const unsigned> indices) : element_type(IndexElement::UInt), count(static_cast<GLsizei>(indices.size()))
    {
        indices_handle = create_indices_buffer(indices.data(), static_cast<GLsizei>(indices.size_bytes()));
    }

    IndexBuffer::IndexBuffer(std::span<const unsigned short> indices) : element_type(IndexElement::UShort), count(static_cast<GLsizei>(indices.size()))
    {
        indices_handle = create_indices_buffer(indices.data(), static_cast<GLsizei>(indices.size_bytes()));
    }

    IndexBuffer::IndexBuffer(std::span<const unsigned char> indices) : element_type(IndexElement::UByte), count(static_cast<GLsizei>(indices.size()))
    {
        indices_handle = create_indices_buffer(indices.data(), static_cast<GLsizei>(indices.size_bytes()));
    }

    IndexBuffer::~IndexBuffer()
    {
        const GLuint handle = indices_handle;
        GL::DeleteBuffers(1, &handle);
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& temp) noexcept : element_type(temp.element_type), count(temp.count), indices_handle(temp.indices_handle)
    {
        temp.element_type   = IndexElement::None;
        temp.count          = 0;
        temp.indices_handle = 0;
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& temp) noexcept
    {
        std::swap(element_type, temp.element_type);
        std::swap(count, temp.count);
        std::swap(indices_handle, temp.indices_handle);

        return *this;
    }

    void IndexBuffer::Use(bool bind) const
    {
        GL::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, bind ? indices_handle : 0);
    }
}
