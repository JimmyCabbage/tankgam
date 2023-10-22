#include "Viewport.h"

#include <string_view>

#include <gl/Vertex.h>

#include "Common.h"

Viewport::Viewport(Editor& editor)
    : editor{ editor }, gl{ nullptr },
      width{ 0 }, height{ 0 }, viewportWidth{ 0 }, viewportHeight{ 0 },
      currentViewport{ nullptr }
{
}

Viewport::~Viewport() = default;

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

void Viewport::initGL(GladGLContext& glf, int width, int height)
{
    gl = &glf;
    this->width = width;
    this->height = height;
    viewportWidth = width / 2;
    viewportHeight = height / 2;
    
    gl->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    createShaders();
    
    borderMesh = std::make_unique<Mesh>(*gl, generateBorders());
    
    createViewport(ViewportType::Top, { 0, 0 });
    createViewport(ViewportType::Front, { width, 0 });
    createViewport(ViewportType::Side, { 0, height });
    createViewport(ViewportType::Projection, { width, height });
    currentViewport = viewportDatas.data();
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

void Viewport::zoomInCamera()
{
    if (!currentViewport)
    {
        return;
    }
    
    zoomCamera(currentViewport->zoom / 5.0f);
}

void Viewport::zoomOutCamera()
{
    if (!currentViewport)
    {
        return;
    }
    
    zoomCamera(-currentViewport->zoom / 5.0f);
}

void Viewport::zoomCamera(float amount)
{
    if (!currentViewport)
    {
        return;
    }
    
    currentViewport->zoom = std::clamp(currentViewport->zoom + amount, 1.0f, 1000.0f);
}

static ViewportCamera setupCamera(Viewport::ViewportType type)
{
    constexpr float initialDistance = 10000.0f;
    
    constexpr float middleGrid = 0.0f;
    
    using Type = Viewport::ViewportType;
    switch (type)
    {
    case Type::Top:
        return ViewportCamera{ { middleGrid, initialDistance, middleGrid }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } };
    case Type::Front:
        return ViewportCamera{ { middleGrid, middleGrid, -initialDistance }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } };
    case Type::Side:
        return ViewportCamera{ { initialDistance, middleGrid, middleGrid }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
    case Type::Projection:
        return ViewportCamera{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f } };
    default:
        throw std::runtime_error("Unknown ViewportType enum");
    }
}

static constexpr int getSkipAxis(Viewport::ViewportType type)
{
    using Type = Viewport::ViewportType;
    switch (type)
    {
    case Type::Top:
        return 1;	//return y axis
    case Type::Front:
        return 2;	//return z axis
    case Type::Side:
        return 0;	//return x axis
    default:
        throw std::runtime_error("Tried to get skip axis of invalid viewport type");
    }
}

static std::vector<Vertex> generateGrid(Viewport::ViewportType type)
{
    std::vector<Vertex> verticies;
    
    //make center xyz thing for 3d projection
    if (type == Viewport::ViewportType::Projection)
    {
        for (int i = 0; i < 3; i++)
        {
            Vertex begin{};
            begin.color = glm::vec3{ 0.0f, 0.0f, 0.0f };
            begin.color[i] = 1.0f;
            
            Vertex end{};
            end.position[i] = static_cast<float>(GRID_UNIT);
            end.color = glm::vec3{ 0.0f, 0.0f, 0.0f };
            end.color[i] = 1.0f;
            
            verticies.push_back(begin);
            verticies.push_back(end);
        }
    }
        
    //make grid for 2d views
    else
    {
        for (int m = 1; m <= 2; m++)
        {
            constexpr int halfLines = (NUM_LINES / 2);
            for (int i = -halfLines; i <= halfLines; i++)
            {
                //TODO:
                //holy hell use better variable names
                //select the axis to generate the lines on
                int skipAxis = getSkipAxis(type);
                int j = (skipAxis + 1) % 3;
                int k = (skipAxis + 2) % 3;
                
                //swap the axises on the second loop to generate the other direction
                if (m == 2)
                {
                    int temp = j;
                    j = k;
                    k = temp;
                }
                
                Vertex begin{ .color = { 0.32f, 0.32f, 0.32f } };
                Vertex end{ .color = { 0.32f, 0.32f, 0.32f } };
                
                //color center red
                if (i == 0)
                {
                    begin.color = { 0.6f, 0.0f, 0.0f };
                    end.color = { 0.6f, 0.0f, 0.0f };
                }
                    //color every 50th line teal-ish
                else if ((i % 50) == 0)
                {
                    begin.color = { 0.0f, 0.4f, 0.4f };
                    end.color = { 0.0f, 0.4f, 0.4f };
                }
                    //color every 10th line extra white
                else if ((i % 10) == 0)
                {
                    begin.color = { 0.95f, 0.95f, 0.95f };
                    end.color = { 0.95f, 0.95f, 0.95f };
                }
                
                begin.position[j] = float(i * GRID_UNIT);
                end.position[j] = float(i * GRID_UNIT);
                
                constexpr int length = NUM_LINES * GRID_UNIT;
                begin.position[k] = -(length / 2);
                end.position[k] = (length / 2);
                
                verticies.push_back(begin);
                verticies.push_back(end);
            }
        }
    }
    
    return verticies;
}

void Viewport::createViewport(ViewportType type, glm::ivec2 offset)
{
    ViewportData viewport{ type, offset, 100.0f, glm::mat4{ 1.0f } };
    viewport.camera = setupCamera(type);
    viewport.gridMesh = std::make_unique<Mesh>(*gl, generateGrid(type));
    
    viewportDatas.push_back(std::move(viewport));
}

void Viewport::removeGL()
{
    currentViewport = nullptr;
    viewportDatas.clear();
    
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
    
    //change the viewport sizes
    //todo: why is this hardcoded find a better way to handle resizing
    for (auto& viewport : viewportDatas)
    {
        if (viewport.type == ViewportType::Top)
        {
            viewport.offset = {0, 0};
        }
        else if (viewport.type == ViewportType::Front)
        {
            viewport.offset = { viewportWidth, 0 };
        }
        else if (viewport.type == ViewportType::Side)
        {
            viewport.offset = { 0, viewportHeight };
        }
        else if (viewport.type == ViewportType::Projection)
        {
            viewport.offset = { viewportWidth, viewportHeight };
        }
    }
    
    gl->Enable(GL_DEPTH_TEST);
    gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //draw all viewports
    for (auto& viewport : viewportDatas)
    {
        gl->Viewport(viewport.offset.x, -viewport.offset.y + viewportHeight, viewportWidth, viewportHeight);
        
        glm::mat4 projViewMatrix{ 1.0f };
        if (viewport.type == ViewportType::Projection)
        {
            projViewMatrix = glm::perspective(glm::radians(90.0f), float(viewportWidth) / float(viewportHeight), 0.1f, 10000.0f);
        }
        else
        {
            const float zoom = viewport.zoom / 100.0f;
            
            const float left = -float(viewportWidth);
            const float right = float(viewportWidth);
            const float down = -float(viewportHeight);
            const float up = float(viewportHeight);
            
            projViewMatrix = glm::ortho(left / zoom, right / zoom, down / zoom, up / zoom,
                                        0.1f, 100000.0f);
        }
        
        projViewMatrix *= viewport.camera.getViewMatrix();
        
        viewport.inverseProjViewMatrix = glm::inverse(projViewMatrix);
        
        defaultShader->use();
        defaultShader->setMat4("uProjView", projViewMatrix);
        
        //draw the grid
        viewport.gridMesh->draw(GL_LINES);
    }
    
    //draw out borders between viewports
    gl->Disable(GL_DEPTH_TEST);
    gl->Viewport(0, 0, width, height);
    
    noProjShader->use();
    borderMesh->draw(GL_TRIANGLES);
}

void Viewport::changeSize(int width, int height)
{
    this->width = width;
    this->height = height;
}
