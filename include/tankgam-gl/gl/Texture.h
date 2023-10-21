#pragma once

#include <span>

#include <glad/gl.h>

class Texture
{
public:
    //formatted RGBA data
    Texture(GladGLContext& gl, const uint8_t* data, int width, int height);
    //raw texture data
    Texture(GladGLContext& gl, std::span<const uint8_t> buffer);
    ~Texture();
    
    Texture(Texture&& o) noexcept;
    Texture& operator=(Texture&& o) noexcept;
    
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    
    int getWidth() const;
    
    int getHeight() const;
    
    void bind();
    
private:
    GladGLContext& gl;
    
    GLuint id;
    int width;
    int height;
    
    void loadData(const uint8_t* data, int width, int height);
};
