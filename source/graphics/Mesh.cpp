/**
 * \file
 * \author Rudy Castan
 * \author Ginam Park
 * \date 2024 Spring
 * \par CS250 Computer Graphics II
 * \copyright DigiPen Institute of Technology
 */
#include "Mesh.hpp"
#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <gsl/gsl>
#include <numbers>

namespace
{
    std::vector<graphics::MeshVertex> create_plane_vertices(int stacks, int slices);
    std::vector<unsigned>             build_index_buffer(int stacks, int slices);
    std::vector<unsigned>             convert_to_lines_pattern(const std::vector<unsigned>& indices);
}

namespace graphics
{
    Geometry create_plane(int stacks, int slices)
    {
        auto vertices = create_plane_vertices(stacks, slices);
        auto indices  = build_index_buffer(stacks, slices);
        return Geometry{ std::move(vertices), std::move(indices) };
    }

    Geometry create_cube(int stacks, int slices)
    {
        const auto plane_vertices = create_plane_vertices(stacks, slices);
        const auto plane_indices  = build_index_buffer(stacks, slices);

        enum Axis
        {
            X,
            Y
        };

        constexpr glm::vec3 AxisVectors[2] = { glm::vec3(1, 0, 0), glm::vec3(0, 1, 0) };

        constexpr glm::vec3 translate_array[] = {
            glm::vec3(+0.0f, +0.0f, +0.5f), // Z+
            glm::vec3(+0.0f, +0.0f, -0.5f), // Z-
            glm::vec3(+0.5f, +0.0f, +0.0f), // X+
            glm::vec3(-0.5f, +0.0f, +0.0f), // X-
            glm::vec3(+0.0f, +0.5f, +0.0f), // Y+
            glm::vec3(+0.0f, -0.5f, +0.0f), // Y-
        };

        struct rotation
        {
            Axis  axis;
            float angle;
        };

        constexpr rotation rotate_array[] = {
            { Axis::Y,                 0.0f }, // Z+
            { Axis::Y, glm::radians(180.0f) }, // Z-
            { Axis::Y,  glm::radians(90.0f) }, // X+
            { Axis::Y, glm::radians(-90.0f) }, // X-
            { Axis::X, glm::radians(-90.0f) }, // Y+
            { Axis::X,  glm::radians(90.0f) }  // Y-
        };

        std::vector<MeshVertex> vertices;
        std::vector<unsigned>   indices;
        vertices.reserve(plane_vertices.size() * 6u);
        indices.reserve(plane_indices.size() * 6u);
        for (unsigned i = 0; i < 6; ++i)
        {
            constexpr glm::mat4 identity(1.0f);
            const auto          axis             = AxisVectors[rotate_array[i].axis];
            const auto          angle            = rotate_array[i].angle;
            const auto          rotation_matrix  = glm::rotate(identity, angle, axis);
            const auto          transform_matrix = glm::translate(identity, translate_array[i]) * rotation_matrix;
            for (const auto plane_vertex : plane_vertices)
            {
                MeshVertex v;
                v.position = glm::vec3(transform_matrix * glm::vec4(plane_vertex.position, 1.0));
                v.normal   = glm::vec3(rotation_matrix * glm::vec4(plane_vertex.normal, 1.0));
                v.uv       = plane_vertex.uv;
                vertices.push_back(v);
            }
            for (const auto plane_index : plane_indices)
            {
                indices.push_back(plane_index + static_cast<unsigned>(plane_vertices.size()) * i);
            }
        }
        return Geometry{ std::move(vertices), std::move(indices) };
    }

    Geometry create_sphere(int stacks, int slices)
    {
        std::vector<MeshVertex> vertices;
        const auto              numVertices = static_cast<size_t>((stacks + 1) * (slices + 1));
        vertices.reserve(numVertices);
        for (int stack = 0; stack <= stacks; ++stack)
        {
            const float row      = static_cast<float>(stack) / static_cast<float>(stacks);
            const float beta     = PI * (row - 0.5f);
            const float sin_beta = std::sin(beta);
            const float cos_beta = std::cos(beta);
            for (int slice = 0; slice <= slices; ++slice)
            {
                constexpr float radius = 0.5f;
                const float     col    = static_cast<float>(slice) / static_cast<float>(slices);
                const float     alpha  = col * 2.0f * PI;
                MeshVertex      v;
                v.position = glm::vec3(radius * std::sin(alpha) * cos_beta, radius * sin_beta, radius * std::cos(alpha) * cos_beta);
                v.normal   = v.position / radius;
                v.uv       = glm::vec2(col, row);
                vertices.push_back(v);
            }
        }
        auto indices = build_index_buffer(stacks, slices);
        return Geometry{ std::move(vertices), std::move(indices) };
    }

    Geometry create_torus(int stacks, int slices, float start_angle, float end_angle)
    {
        std::vector<MeshVertex> vertices;
        const auto              numVertices = static_cast<size_t>((stacks + 1) * (slices + 1));
        vertices.reserve(numVertices);
        constexpr float R = 0.35f;
        constexpr float r = 0.15f;
        for (int stack = 0; stack <= stacks; ++stack)
        {
            const float row       = static_cast<float>(stack) / static_cast<float>(stacks);
            const float alpha     = glm::mix(start_angle, end_angle, row);
            const float sin_alpha = std::sin(alpha);
            const float cos_alpha = std::cos(alpha);
            for (int slice = 0; slice <= slices; ++slice)
            {
                const float col      = static_cast<float>(slice) / static_cast<float>(slices);
                const float beta     = glm::mix(2.0f * PI, 0.0f, col);
                const float cos_beta = std::cos(beta);
                MeshVertex  v;
                v.position.x     = (R + r * cos_beta) * sin_alpha;
                v.position.y     = r * std::sin(beta);
                v.position.z     = (R + r * cos_beta) * cos_alpha;
                glm::vec3 center = glm::vec3(R * sin_alpha, 0.0f, R * cos_alpha);
                v.normal         = (v.position - center) / r;
                v.uv             = glm::vec2(col, row);
                vertices.push_back(v);
            }
        }
        auto indices = build_index_buffer(stacks, slices);
        return Geometry{ std::move(vertices), std::move(indices) };
    }

    void add_cap(std::vector<MeshVertex>& vertices, std::vector<unsigned>& indices, float center_y, int slices)
    {
        constexpr float R           = 0.5f;
        const auto      centerIndex = static_cast<unsigned>(vertices.size());
        MeshVertex      vertex;

        const float textureCoordScale = (center_y > 0.0f) ? 1.0f : -1.0f;

        vertex.normal   = (center_y > 0.0f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, -1.0f, 0.0f);
        vertex.position = glm::vec3(0.0f, center_y, 0.0f);
        vertex.uv       = glm::vec2(0.5f, 0.5f);
        vertices.push_back(vertex);

        for (int slice = 0; slice <= slices; ++slice)
        {
            const float col      = static_cast<float>(slice) / static_cast<float>(slices);
            const float alpha    = col * PI * 2.0f;
            const float sinAlpha = std::sin(alpha);
            const float cosAlpha = std::cos(alpha);
            vertex.position.x    = R * sinAlpha;
            vertex.position.z    = R * cosAlpha;
            vertex.uv            = glm::vec2(textureCoordScale * 0.5f * cosAlpha + 0.5f, 0.5f * sinAlpha + 0.5f);
            vertices.push_back(vertex);
        }

        unsigned k                  = centerIndex + 1;
        const unsigned secondOffset = (center_y > 0.0f) ? 0u : 1u;
        const unsigned thirdOffset  = (center_y > 0.0f) ? 1u : 0u;
        for (int i = 0; i < slices - 1; ++i)
        {
            indices.push_back(centerIndex);
            indices.push_back(k + secondOffset);
            indices.push_back(k + thirdOffset);
            ++k;
        }
        if (center_y > 0.0f)
        {
            indices.push_back(centerIndex);
            indices.push_back(k);
            indices.push_back(centerIndex + 1);
        }
        else
        {
            indices.push_back(centerIndex);
            indices.push_back(centerIndex + 1);
            indices.push_back(k);
        }
    }

    Geometry create_cylinder(int stacks, int slices)
    {
        std::vector<MeshVertex> vertices;
        const auto              numVertices = static_cast<size_t>((stacks + 1) * (slices + 1) + (slices * 2 + 2));
        vertices.reserve(numVertices);
        constexpr float R = 0.5f;
        for (int stack = 0; stack <= stacks; ++stack)
        {
            const float row = static_cast<float>(stack) / static_cast<float>(stacks);
            for (int slice = 0; slice <= slices; ++slice)
            {
                const float col      = static_cast<float>(slice) / static_cast<float>(slices);
                const float alpha    = col * PI * 2.0f;
                MeshVertex  vertex;
                const float sinAlpha = std::sin(alpha);
                const float cosAlpha = std::cos(alpha);
                vertex.position.x    = R * sinAlpha;
                vertex.position.y    = row - 0.5f;
                vertex.position.z    = R * cosAlpha;
                vertex.normal        = glm::vec3(sinAlpha, 0.0f, cosAlpha);
                vertex.uv            = glm::vec2(col, row);
                vertices.push_back(vertex);
            }
        }

        auto indices = build_index_buffer(stacks, slices);
        add_cap(vertices, indices, 0.5f, slices);
        add_cap(vertices, indices, -0.5f, slices);

        return Geometry{ std::move(vertices), std::move(indices) };
    }

    Geometry create_cone(int stacks, int slices)
    {
        std::vector<MeshVertex> vertices;
        const auto              numVertices = static_cast<size_t>((stacks + 1) * (slices + 1) + (slices + 1));
        vertices.reserve(numVertices);
        constexpr float R            = 0.5f;
        constexpr float TopRadius    = 0.0f;
        constexpr float BottomRadius = R;
        constexpr float TopYValue    = 0.5f;
        constexpr float BottomYValue = -0.5f;
        constexpr float Rise         = TopYValue - BottomYValue;
        constexpr float Run          = TopRadius - BottomRadius;
        constexpr float Slope        = Rise / Run;
        constexpr float TangentSlope = -1.0f / Slope;
        for (int stack = 0; stack <= stacks; ++stack)
        {
            const float row = static_cast<float>(stack) / static_cast<float>(stacks);
            const float h   = row - 0.5f;
            for (int slice = 0; slice <= slices; ++slice)
            {
                const float col      = static_cast<float>(slice) / static_cast<float>(slices);
                const float alpha    = col * PI * 2.0f;
                const float sinAlpha = std::sin(alpha);
                const float cosAlpha = std::cos(alpha);
                MeshVertex  vertex;
                vertex.position.x = R * (0.5f - h) * sinAlpha;
                vertex.position.y = h;
                vertex.position.z = R * (0.5f - h) * cosAlpha;
                if (stack != stacks)
                    vertex.normal = glm::normalize(glm::vec3(sinAlpha, TangentSlope, cosAlpha));
                else
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                vertex.uv = glm::vec2(col, row);
                vertices.push_back(vertex);
            }
        }

        auto indices = build_index_buffer(stacks, slices);
        add_cap(vertices, indices, -0.5f, slices);

        return Geometry{ std::move(vertices), std::move(indices) };
    }

    SubMesh to_submesh_as_triangles(const Geometry& geometry, Material* material)
    {
        SubMesh sub_mesh;
        sub_mesh.VertexArrayObj.SetPrimitivePattern(opengl::Primitive::Triangles);
        sub_mesh.VertexArrayObj.AddVertexBuffer(opengl::VertexBuffer(std::span{ geometry.Vertices }), get_meshvertex_layout());
        sub_mesh.VertexArrayObj.SetIndexBuffer(opengl::IndexBuffer(std::span{ geometry.Indicies }));
        sub_mesh.Material = material;
        return sub_mesh;
    }

    SubMesh to_submesh_as_lines(const Geometry& geometry, Material* material)
    {
        std::vector<unsigned> lines_indices = convert_to_lines_pattern(geometry.Indicies);
        SubMesh               sub_mesh;
        sub_mesh.VertexArrayObj.SetPrimitivePattern(opengl::Primitive::Lines);
        sub_mesh.VertexArrayObj.AddVertexBuffer(opengl::VertexBuffer(std::span{ geometry.Vertices }), get_meshvertex_layout());
        sub_mesh.VertexArrayObj.SetIndexBuffer(opengl::IndexBuffer(std::span{ lines_indices }));
        sub_mesh.Material = material;
        return sub_mesh;
    }
}

namespace
{
    std::vector<graphics::MeshVertex> create_plane_vertices(int stacks, int slices)
    {
        std::vector<graphics::MeshVertex> vertices;
        const auto                        numVertices = static_cast<size_t>((stacks + 1) * (slices + 1));
        vertices.reserve(numVertices);

        for (int stack = 0; stack <= stacks; ++stack)
        {
            const float row = static_cast<float>(stack) / static_cast<float>(stacks);
            for (int slice = 0; slice <= slices; ++slice)
            {
                const float          col = static_cast<float>(slice) / static_cast<float>(slices);
                graphics::MeshVertex v;
                v.position = glm::vec3(col - 0.5f, row - 0.5f, 0.0f);
                v.normal   = glm::vec3(0.0f, 0.0f, 1.0f);
                v.uv       = glm::vec2(col, row);
                vertices.push_back(v);
            }
        }

        return vertices;
    }

    std::vector<unsigned> build_index_buffer(int stacks, int slices)
    {
        std::vector<unsigned> indices;
        const auto            numIndices = static_cast<size_t>(stacks * slices * 2 * 3);
        indices.reserve(numIndices);
        const auto stride = static_cast<unsigned>(slices + 1);

        for (int i = 0; i < stacks; ++i)
        {
            const auto currRow = static_cast<unsigned>(i) * stride;

            for (int j = 0; j < slices; ++j)
            {
                unsigned p0 = currRow + static_cast<unsigned>(j);
                unsigned p1 = p0 + 1;
                unsigned p2 = p1 + stride;

                indices.push_back(p0);
                indices.push_back(p1);
                indices.push_back(p2);

                unsigned p3 = p2;
                unsigned p4 = p3 - 1;
                unsigned p5 = p0;

                indices.push_back(p3);
                indices.push_back(p4);
                indices.push_back(p5);
            }
        }

        return indices;
    }

    std::vector<unsigned> convert_to_lines_pattern(const std::vector<unsigned>& indices)
    {
        std::vector<unsigned> linesIndices;
        size_t                i = 0;

        if (indices.size() > 6)
        {
            const size_t limit = (indices.size() % 6 == 0) ? indices.size() : indices.size() - 5;

            while (i < limit)
            {
                unsigned p0 = indices[i];
                unsigned p1 = indices[i + 1];
                unsigned p2 = indices[i + 2];
                unsigned p3 = indices[i + 3];
                unsigned p4 = indices[i + 4];
                unsigned p5 = indices[i + 5];

                if (p1 != (p0 + 1) || p3 != p2 || p4 != (p3 - 1) || p5 != p0)
                    break;

                linesIndices.push_back(p0);
                linesIndices.push_back(p1);
                linesIndices.push_back(p1);
                linesIndices.push_back(p2);
                linesIndices.push_back(p2);
                linesIndices.push_back(p4);
                linesIndices.push_back(p4);
                linesIndices.push_back(p0);

                i += 6;
            }
        }

        if (i < indices.size() && indices.size() - i >= 3)
        {
            const size_t limit = (indices.size() % 3 == 0) ? indices.size() : indices.size() - 2;

            while (i < limit)
            {
                unsigned p0 = indices[i];
                unsigned p1 = indices[i + 1];
                unsigned p2 = indices[i + 2];

                linesIndices.push_back(p0);
                linesIndices.push_back(p1);
                linesIndices.push_back(p1);
                linesIndices.push_back(p2);
                linesIndices.push_back(p2);
                linesIndices.push_back(p0);

                i += 3;
            }
        }

        return linesIndices;
    }
}
