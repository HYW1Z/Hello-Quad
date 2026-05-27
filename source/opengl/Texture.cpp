/**
 * \file
 * \author Rudy Castan
 * \author Ginam Park
 * \date 2025 Spring
 * \par CS250 Computer Graphics II
 * \copyright DigiPen Institute of Technology
 */
#include "Texture.hpp"

#include "GL.hpp"
#include "assets/Path.hpp"
#include <GL/glew.h>
#include <stb_image.h>

namespace opengl
{

    Texture::~Texture() noexcept
    {
        delete_texture();
    }

    Texture::Texture(Texture&& other) noexcept : texture_handle(other.texture_handle), width(other.width), height(other.height)
    {
        other.texture_handle = 0;
        other.width          = 0;
        other.height         = 0;
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        std::swap(texture_handle, other.texture_handle);
        std::swap(width, other.width);
        std::swap(height, other.height);
        return *this;
    }

    bool Texture::LoadFromFileImage(std::filesystem::path image_filepath, bool flip_vertical) noexcept
    {
        if (!std::filesystem::exists(image_filepath))
        {
            // try prepending the asset directory path
            image_filepath = assets::get_base_path() / image_filepath;
            if (!std::filesystem::exists(image_filepath))
            {
                return false;
            }
        }
        int           pixel_width;
        int           pixel_height;
        int           files_channels_count  = 0;
        constexpr int desired_channel_count = 4;
        stbi_set_flip_vertically_on_load(flip_vertical);
        RGBA* const rgba_pixels = reinterpret_cast<RGBA*>(stbi_load(image_filepath.string().c_str(), &pixel_width, &pixel_height, &files_channels_count, desired_channel_count));
        if (rgba_pixels == nullptr)
            return false;
        const bool result = LoadFromMemory(pixel_width, pixel_height, rgba_pixels);
        stbi_image_free(rgba_pixels);
        return result;
    }

    bool Texture::LoadFromMemory(int image_width, int image_height, const RGBA* colors) noexcept
    {
        delete_texture();
        width  = image_width;
        height = image_height;

        GLuint handle = 0;
        GL::GenTextures(1, &handle);
        texture_handle = handle;
        GL::BindTexture(GL_TEXTURE_2D, texture_handle);
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping[S]);
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping[T]);
        GL::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);
        GL::BindTexture(GL_TEXTURE_2D, 0);

        return true;
    }

    void Texture::UseForSlot(unsigned texture_unit) const noexcept
    {
        GL::ActiveTexture(GL_TEXTURE0 + texture_unit);
        GL::BindTexture(GL_TEXTURE_2D, texture_handle);
    }

    void Texture::SetFiltering(Filtering how_to_filter) noexcept
    {
        if (filtering == how_to_filter || texture_handle == 0)
            return;

        filtering = how_to_filter;
        GL::BindTexture(GL_TEXTURE_2D, texture_handle);
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
        GL::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
    }

    void Texture::SetWrapping(Wrapping how_to_wrap, Coordinate coord) noexcept
    {
        if (texture_handle == 0)
            return;

        GLenum coords_to_set[2] = {};
        int    num_coords       = 1;
        switch (coord)
        {
            case Coordinate::S:
                coords_to_set[0] = GL_TEXTURE_WRAP_S;
                wrapping[S]      = how_to_wrap;
                break;
            case Coordinate::T:
                coords_to_set[0] = GL_TEXTURE_WRAP_T;
                wrapping[T]      = how_to_wrap;
                break;
            case Coordinate::Both:
                coords_to_set[0] = GL_TEXTURE_WRAP_S;
                coords_to_set[1] = GL_TEXTURE_WRAP_T;
                wrapping[S]      = how_to_wrap;
                wrapping[T]      = how_to_wrap;
                num_coords       = 2;
                break;
        }

        GL::BindTexture(GL_TEXTURE_2D, texture_handle);
        for (int i = 0; i < num_coords; ++i)
        {
            GL::TexParameteri(GL_TEXTURE_2D, coords_to_set[i], how_to_wrap);
        }
    }

    void Texture::delete_texture() noexcept
    {
        const GLuint handle = texture_handle;
        GL::DeleteTextures(1, &handle);
        texture_handle = 0;
        width          = 0;
        height         = 0;
    }
}
