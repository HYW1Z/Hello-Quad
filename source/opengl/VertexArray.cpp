/**
 * \file
 * \author Rudy Castan
 * \author Ginam Park
 * \date 2025 Spring
 * \par CS250 Computer Graphics II
 * \copyright DigiPen Institute of Technology
 */

#include "VertexArray.hpp"

#include "GL.hpp"
#include <GL/glew.h>
#include <gsl/gsl>

namespace opengl
{

    VertexArray::VertexArray(Primitive::Type the_primitive_pattern)
    {
        primitive_pattern = the_primitive_pattern;
        GLuint handle = 0;
        GL::GenVertexArrays(1, &handle);
        vertex_array_handle = handle;
    }

    VertexArray::~VertexArray()
    {
        GLuint handle = vertex_array_handle;
        GL::DeleteVertexArrays(1, &handle);
    }

    VertexArray::VertexArray(VertexArray&& temp) noexcept
        : vertex_array_handle(temp.vertex_array_handle), vertex_buffers(std::move(temp.vertex_buffers)), index_buffer(std::move(temp.index_buffer)), num_indices(temp.num_indices),
          indices_type(temp.indices_type), primitive_pattern(temp.primitive_pattern), num_vertices(temp.num_vertices)
    {
        temp.vertex_array_handle = 0;
        temp.num_indices         = 0;
        temp.indices_type        = IndexElement::None;
        temp.num_vertices        = 0;
    }

    VertexArray& VertexArray::operator=(VertexArray&& temp) noexcept
    {
        std::swap(vertex_array_handle, temp.vertex_array_handle);
        std::swap(vertex_buffers, temp.vertex_buffers);
        std::swap(index_buffer, temp.index_buffer);
        std::swap(num_indices, temp.num_indices);
        std::swap(indices_type, temp.indices_type);
        std::swap(primitive_pattern, temp.primitive_pattern);
        std::swap(num_vertices, temp.num_vertices);

        return *this;
    }

    void VertexArray::Use(bool bind) const
    {
        GL::BindVertexArray(bind ? vertex_array_handle : 0);
    }

    void VertexArray::AddVertexBuffer(VertexBuffer&& vertex_buffer, BufferLayout buffer_layout)
    {
        Use(true);
        vertex_buffer.Use(true);

        // Calculate stride
        GLsizei stride = 0;
        for (const auto& attr : buffer_layout.Attributes)
        {
            stride += attr.SizeBytes;
        }

        auto offset = static_cast<GLsizei>(buffer_layout.BufferStartingByteOffset);
        GLuint attribute_index = 0;
        for (const auto& attr : buffer_layout.Attributes)
        {
            if (attr == Attribute::None)
            {
                offset += attr.SizeBytes;
                continue;
            }

            GL::EnableVertexAttribArray(attribute_index);

            const auto gl_type        = static_cast<GLenum>(attr.GLType);
            const auto component_count = static_cast<GLint>(attr.ComponentCount);
            const auto normalize       = static_cast<GLboolean>(attr.Normalize ? GL_TRUE : GL_FALSE);

            if (attr.IntAttribute)
            {
                GL::VertexAttribIPointer(attribute_index, component_count, gl_type, stride, reinterpret_cast<const void*>(static_cast<intptr_t>(offset)));
            }
            else
            {
                GL::VertexAttribPointer(attribute_index, component_count, gl_type, normalize, stride, reinterpret_cast<const void*>(static_cast<intptr_t>(offset)));
            }

            GL::VertexAttribDivisor(attribute_index, attr.Divisor);

            ++attribute_index;
            offset += attr.SizeBytes;
        }

        Use(false);
        vertex_buffer.Use(false);

        vertex_buffers.emplace_back(std::move(vertex_buffer));
    }

    void VertexArray::SetIndexBuffer(IndexBuffer&& the_indices)
    {
        num_indices  = the_indices.GetCount();
        indices_type = the_indices.GetElementType();

        Use(true);
        the_indices.Use(true);

        index_buffer = std::move(the_indices);
    }

    void DrawIndexed(const VertexArray& vertex_array) noexcept
    {
        GL::DrawElements(
            vertex_array.GetPrimitivePattern(),
            vertex_array.GetIndicesCount(),
            vertex_array.GetIndicesType(),
            nullptr
        );
    }

    void DrawVertices(const VertexArray& vertex_array) noexcept
    {
        GL::DrawArrays(
            vertex_array.GetPrimitivePattern(),
            0,
            vertex_array.GetVertexCount()
        );
    }
}
