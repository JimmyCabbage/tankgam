#include "gl/Mesh.h"

#include <vector>
#include <unordered_map>

#include <fmt/format.h>
#include "tiny_obj_loader.h"

#include "gl/Vertex.h"

Mesh::Mesh(GladGLContext& gl, std::span<const Vertex> vertices)
    : gl{ gl },
      vao{ 0 }, vbo{ 0 }, ebo{ 0 }
{
    uploadVertices(vertices);
}

Mesh::Mesh(GladGLContext& gl, std::istream& istream)
    : gl{ gl },
      vao{ 0 }, vbo{ 0 }, ebo{ 0 }
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;
    
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &istream))
    {
        throw std::runtime_error{ fmt::format("Failed to load obj: {}", warn + err) };
    }
    
    std::vector<Vertex> vertices;
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};
            
            vertex.position =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
            };
            
            vertex.color = glm::vec3{ 1.0f };
            
            if (index.normal_index != -1)
            {
                vertex.normal =
                {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            
            vertex.texCoord =
            {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1],
            };
            
            vertices.push_back(vertex);
        }
    }
    
    uploadVertices(vertices);
}

Mesh::~Mesh()
{
    gl.DeleteBuffers(1, &ebo);
    gl.DeleteBuffers(1, &vbo);
    gl.DeleteVertexArrays(1, &vao);
}

Mesh::Mesh(Mesh&& o) noexcept
    : gl{ o.gl },
      vao{ o.vao }, vbo{ o.vbo }, ebo{ o.ebo },
      numVertices{ o.numVertices }, numIndicies{ o.numIndicies }
{
    o.vao = 0;
    o.vbo = 0;
    o.ebo = 0;
    o.numVertices = 0;
    o.numIndicies = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept
{
    if (this == &o)
    {
        return *this;
    }
    
    gl = o.gl;
    
    vao = o.vao;
    vbo = o.vbo;
    ebo = o.ebo;
    numVertices = o.numVertices;
    numIndicies = o.numIndicies;
    
    o.vao = 0;
    o.vbo = 0;
    o.ebo = 0;
    o.numVertices = 0;
    o.numIndicies = 0;
    
    return *this;
}

void Mesh::draw(GLenum mode)
{
    if (vao)
    {
        gl.BindVertexArray(vao);
        gl.DrawElements(mode, numIndicies, GL_UNSIGNED_INT, nullptr);
        //gl.DrawArrays(mode, 0, numVertices);
    }
}

void Mesh::uploadVertices(std::span<const Vertex> vertices)
{
    std::vector<Vertex> realVertices;
    std::vector<GLuint> indices;
    {
        std::unordered_map<Vertex, GLuint> uniqueVertices;
        for (Vertex vertex : vertices)
        {
            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<GLuint>(realVertices.size());
                realVertices.push_back(vertex);
            }
            
            indices.push_back(uniqueVertices[vertex]);
        }
    }
    
    numVertices = static_cast<GLsizei>(realVertices.size());
    numIndicies = static_cast<GLsizei>(indices.size());
    
    gl.GenVertexArrays(1, &vao);
    gl.BindVertexArray(vao);
    
    gl.GenBuffers(1, &vbo);
    gl.BindBuffer(GL_ARRAY_BUFFER, vbo);
    gl.BufferData(GL_ARRAY_BUFFER,
                  static_cast<GLsizeiptr>(sizeof(Vertex) * realVertices.size()),
                  realVertices.data(),
                  GL_STATIC_DRAW);
    
    gl.GenBuffers(1, &ebo);
    gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    gl.BufferData(GL_ELEMENT_ARRAY_BUFFER,
                  static_cast<GLsizeiptr>(sizeof(GLuint) * indices.size()),
                  indices.data(),
                  GL_STATIC_DRAW);
    
    gl.VertexAttribPointer(0, 3,
                           GL_FLOAT, GL_FALSE,
                           sizeof(Vertex),
                           reinterpret_cast<void*>(offsetof(Vertex, position)));
    gl.EnableVertexAttribArray(0);
    
    gl.VertexAttribPointer(1, 3,
                           GL_FLOAT, GL_FALSE,
                           sizeof(Vertex),
                           reinterpret_cast<void*>(offsetof(Vertex, color)));
    gl.EnableVertexAttribArray(1);
    
    gl.VertexAttribPointer(2, 3,
                           GL_FLOAT, GL_FALSE,
                           sizeof(Vertex),
                           reinterpret_cast<void*>(offsetof(Vertex, normal)));
    gl.EnableVertexAttribArray(2);
    
    gl.VertexAttribPointer(3, 2,
                           GL_FLOAT, GL_FALSE,
                           sizeof(Vertex),
                           reinterpret_cast<void*>(offsetof(Vertex, texCoord)));
    gl.EnableVertexAttribArray(3);
}
