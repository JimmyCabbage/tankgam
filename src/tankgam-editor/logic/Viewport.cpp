#include "Viewport.h"

#include <string_view>

#include <gl/Vertex.h>

Viewport::Viewport(Editor& editor)
    : editor{ editor }, gl{ nullptr }
{
}

Viewport::~Viewport() = default;

static std::array<Vertex, 12> generateBorders();

void Viewport::initGL(GladGLContext& glf, int width, int height)
{
    gl = &glf;
    this->width = width;
    this->height = height;
    
    gl->Enable(GL_DEPTH_TEST);
    gl->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    createShaders();
    
    borderMesh = std::make_unique<Mesh>(*gl, generateBorders());
    
    render();
}

void Viewport::createShaders()
{
    constexpr std::string_view DEFAULT_PROJ_VERT = R"(#version 330 core
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec3 aColor;
                layout (location = 2) in vec3 aNormal;
                layout (location = 3) in vec2 aTexCoord;

                out vec3 vPosition;
                out vec3 vColor;
                out vec3 vNormal;
                out vec2 vTexCoord;

                uniform mat4 uProjView;

                void main()
                {
                    gl_Position = uProjView * vec4(aPos, 1.0);
                    vPosition = aPos;
                    vColor = aColor;
                    vNormal = aNormal;
                    vTexCoord = aTexCoord;
                })";
    
    constexpr std::string_view DEFAULT_PROJ_FRAG = R"(#version 330 core
                in vec3 vPos;
                in vec3 vColor;
                in vec3 vNormal;
                in vec2 vTexCoord;

                layout (location = 0) out vec4 outFragColor;

                void main()
                {
                    outFragColor = vec4(vColor, 1.0f);
                })";
    
    defaultShader = std::make_unique<Shader>(*gl, DEFAULT_PROJ_VERT, DEFAULT_PROJ_FRAG);
    
    constexpr std::string_view BRUSH_PROJ_FRAG = R"(#version 420 core
                in vec3 vPos;
                in vec3 vColor;
                in vec3 vNormal;
                in vec2 vTexCoord;

                layout (location = 0) out vec4 outFragColor;

                layout (binding = 0) uniform sampler2D diffuseTexture;

                void main()
                {
                    outFragColor = vec4(texture(diffuseTexture, vTexCoord).rgb, 1.0f);
                })";
    
    brushShader = std::make_unique<Shader>(*gl, DEFAULT_PROJ_VERT, BRUSH_PROJ_FRAG);
    
    constexpr std::string_view NO_PROJ_VERT = R"(#version 330 core
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec3 aColor;
                layout (location = 2) in vec3 aNormal;
                layout (location = 3) in vec2 aTexCoord;

                out vec3 vPosition;
                out vec3 vColor;
                out vec3 vNormal;
                out vec2 vTexCoord;

                void main()
                {
                    gl_Position = vec4(aPos, 1.0);
                    vPosition = aPos;
                    vColor = aColor;
                    vNormal = aNormal;
                    vTexCoord = aTexCoord;
                })";
    
    noProjShader = std::make_unique<Shader>(*gl, NO_PROJ_VERT, DEFAULT_PROJ_FRAG);
}

static std::array<Vertex, 12> generateBorders()
{
    constexpr glm::vec4 color{ 0.75f, 0.75f, 0.75f, 1.0f };
    constexpr float borderSize = 0.005f;
    
    //horizontal corners
    constexpr glm::vec3 hcTopLeft{ -1.0f, borderSize, 0.0f };
    constexpr glm::vec3 hcBottomLeft{ -1.0f, -borderSize, 0.0f };
    constexpr glm::vec3 hcTopRight{ 1.0f, borderSize, 0.0f };
    constexpr glm::vec3 hcBottomRight{ 1.0f, -borderSize, 0.0f };
    
    //vertical corners
    constexpr glm::vec3 vcTopLeft{ -borderSize, 1.0f, 0.0f };
    constexpr glm::vec3 vcBottomLeft{ -borderSize, -1.0f, 0.0f };
    constexpr glm::vec3 vcTopRight{ borderSize, 1.0f, 0.0f };
    constexpr glm::vec3 vcBottomRight{ borderSize, -1.0f, 0.0f };
    
    constexpr std::array<Vertex, 12> verticies
    {
        //tri1 horizontal
        Vertex{ .position = { hcTopLeft }, .color = color },
        Vertex{ .position = { hcTopRight }, .color = color },
        Vertex{ .position = { hcBottomRight }, .color = color },
        //tri2 horizontal
        Vertex{ .position = { hcBottomLeft }, .color = color },
        Vertex{ .position = { hcTopLeft }, .color = color },
        Vertex{ .position = { hcBottomRight }, .color = color },
        //tri1 horizontal
        Vertex{ .position = { vcTopLeft }, .color = color },
        Vertex{ .position = { vcTopRight }, .color = color },
        Vertex{ .position = { vcBottomRight }, .color = color },
        //tri2 horizontal
        Vertex{ .position = { vcBottomLeft }, .color = color },
        Vertex{ .position = { vcTopLeft }, .color = color },
        Vertex{ .position = { vcBottomRight }, .color = color },
    };
    
    return verticies;
}

void Viewport::removeGL()
{
    borderMesh.reset();
    
    noProjShader.reset();
    brushShader.reset();
    defaultShader.reset();
    
    gl = nullptr;
}

void Viewport::render()
{
    if (!gl)
    {
        return;
    }
    
    gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    gl->Viewport(0, 0, width, height);
    
    noProjShader->use();
    borderMesh->draw(GL_TRIANGLES);
}

void Viewport::changeSize(int width, int height)
{
    this->width = width;
    this->height = height;
}
