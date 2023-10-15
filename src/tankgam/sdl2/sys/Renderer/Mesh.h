#pragma once

#include <span>
#include <istream>

#include <glad/gl.h>

struct Vertex;

class Mesh
{
public:
    Mesh(GladGLContext& gl, std::span<const Vertex> vertices);
    //input stream from a file or something
    Mesh(GladGLContext& gl, std::istream& istream);
    ~Mesh();
    
    Mesh(Mesh&& o) noexcept;
    Mesh& operator=(Mesh&& o) noexcept;
    
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    
    void draw(GLenum mode);
    
private:
    GladGLContext& gl;
    
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLsizei numVertices;
    GLsizei numIndicies;
    
    void uploadVertices(std::span<const Vertex> vertices);
};
