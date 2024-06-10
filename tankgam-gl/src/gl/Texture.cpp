#include "gl/Texture.h"

#include <stdexcept>

#include <fmt/format.h>
#include "stb_image.h"

Texture::Texture(GladGLContext& gl, const uint8_t* data, int width, int height)
    : gl{ gl }, id{}, width{ width }, height{ height }
{
    loadData(data, width, height);
}

Texture::Texture(GladGLContext& gl, std::span<const uint8_t> buffer)
    : gl{ gl }, id{}, width{}, height{}
{
    stbi_set_flip_vertically_on_load(true);
    
    int n = 0;
    stbi_uc* data = stbi_load_from_memory(buffer.data(), static_cast<int>(buffer.size()),
                                          &width, &height, &n,
                                          STBI_rgb_alpha);
    if (!data)
    {
        if (stbi_failure_reason())
        {
            throw std::runtime_error{ fmt::format("Failed to load texture from buffer:\n{}", stbi_failure_reason()) };
        }

        throw std::runtime_error{ "Failed to load texture from buffer:\nUnknown Reason" };
    }
    
    loadData(data, width, height);
    
    stbi_image_free(data);
}

Texture::~Texture()
{
    gl.DeleteTextures(1, &id);
}

Texture::Texture(Texture&& o) noexcept
    : gl{ o.gl },
      id{ o.id }, width{ o.width }, height{ o.height }
{
    o.id = 0;
    
    o.width = 0;
    o.height = 0;
}

Texture& Texture::operator=(Texture&& o) noexcept
{
    if (this == &o)
    {
        return *this;
    }
    
    gl = o.gl;
    
    id = o.id;
    width = o.width;
    height = o.height;
    
    o.id = 0;
    o.width = 0;
    o.height = 0;
    
    return *this;
}

int Texture::getWidth() const
{
    return width;
}

int Texture::getHeight() const
{
    return height;
}

void Texture::bind()
{
    gl.BindTexture(GL_TEXTURE_2D, id);
}

void Texture::loadData(const uint8_t* data, int iwidth, int iheight)
{
    width = iwidth;
    height = iheight;
    
    gl.GenTextures(1, &id);
    
    gl.BindTexture(GL_TEXTURE_2D, id);
    
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    gl.TexImage2D(GL_TEXTURE_2D,
                  0, GL_RGBA,
                  iwidth, iheight, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE,
                  data);
}
