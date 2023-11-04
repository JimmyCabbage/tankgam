#include "Viewport.h"

#include <string_view>

#include <gl/Vertex.h>

#include "Common.h"
#include "Editor.h"
#include "Brush.h"
#include "ViewportHelpers.h"

Viewport::Viewport(Editor& editor)
    : editor{ editor }, gl{ nullptr },
      width{ 0 }, height{ 0 }, viewportWidth{ 0 }, viewportHeight{ 0 },
      toolType{ ViewportToolType::Select },
      currentViewport{ nullptr }
{
}

Viewport::~Viewport() = default;

void Viewport::update()
{
    const auto brushes = editor.getBrushes();
    for (const auto& brush : brushes)
    {
        const auto verticesList = makeBrushVertices(brush);
        for (const auto& vertices : verticesList)
        {
            brushMeshes.emplace_back(*gl, vertices);
        }
    }
}

void Viewport::initGL(GladGLContext& glf, int width, int height)
{
    gl = &glf;
    {
        GLfloat lineWidths[2];
        gl->GetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, lineWidths);
        maxLineWidth = lineWidths[1];
    }
    changeSize(width, height);
    
    gl->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    createShaders();
    
    borderMesh = std::make_unique<Mesh>(*gl, generateBorders());
    
    createViewport(ViewportType::Top, { 0, 0 });
    createViewport(ViewportType::Front, { width, 0 });
    createViewport(ViewportType::Side, { 0, height });
    createViewport(ViewportType::Projection, { width, height });
    currentViewport = viewportDatas.data();
}

void Viewport::quitGL()
{
    brushMeshes.clear();
    
    currentViewport = nullptr;
    viewportDatas.clear();
    
    borderMesh.reset();
    
    noProjShader.reset();
    brushShader.reset();
    defaultShader.reset();
    
    gl = nullptr;
}

void Viewport::setToolType(ViewportToolType viewportToolType)
{
    toolType = viewportToolType;
}

ViewportToolType Viewport::getToolType() const
{
    return toolType;
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

                //layout (binding = 0) uniform sampler2D diffuseTexture;

                void main()
                {
                    float d1 = dot(vNormal, vec3(1.0f / sqrt(14.0f), 3.0f / sqrt(14.0f), sqrt(2.0f / 7.0f)));
                    d1 = (d1 / 2.0f) + 0.5f;
                    outFragColor = vec4(vColor * d1, 1.0f);
                    //outFragColor = vec4(texture(diffuseTexture, vTexCoord).rgb, 1.0f);
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

void Viewport::createViewport(ViewportType type, glm::ivec2 offset)
{
    ViewportData viewport{ type, offset, 100.0f, glm::mat4{ 1.0f } };
    viewport.camera = setupCamera(type);
    if (type != ViewportType::Projection)
    {
        viewport.gridMesh = std::make_unique<Mesh>(*gl, generateGrid(type));
    }
    viewport.coordinateMesh = std::make_unique<Mesh>(*gl, generateCoordinates());
    
    viewportDatas.push_back(std::move(viewport));
}

void Viewport::moveCamera(Viewport::MoveDir moveDir)
{
    using Move = MoveDir;
    switch (moveDir)
    {
    case Move::Forward:
        if (currentViewport->type == ViewportType::Projection) currentViewport->camera.move(ViewportCamera::Direction::Forward);
        else currentViewport->camera.move(ViewportCamera::Direction::Up);
        break;
    case Move::Back:
        if (currentViewport->type == ViewportType::Projection) currentViewport->camera.move(ViewportCamera::Direction::Backward);
        else currentViewport->camera.move(ViewportCamera::Direction::Down);
        break;
    case Move::Left:
        currentViewport->camera.move(ViewportCamera::Direction::Left);
        break;
    case Move::Right:
        currentViewport->camera.move(ViewportCamera::Direction::Right);
        break;
    case Move::Up:
        currentViewport->camera.move(ViewportCamera::Direction::Up);
        break;
    case Move::Down:
        currentViewport->camera.move(ViewportCamera::Direction::Down);
        break;
    }
}

void Viewport::turnCamera(TurnDir turnDir)
{
    if (!currentViewport)
    {
        return;
    }
    
    using Turn = TurnDir;
    
    switch (turnDir)
    {
    case Turn::Left:
        if (currentViewport->type == ViewportType::Projection) currentViewport->camera.turn(ViewportCamera::Direction::Left);
        break;
    case Turn::Right:
        if (currentViewport->type == ViewportType::Projection) currentViewport->camera.turn(ViewportCamera::Direction::Right);
        break;
    }
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

void Viewport::clickLeftStart(int x, int y)
{
    if (!currentViewport)
    {
        return;
    }
    
    currentViewport = &chooseViewportMouse({ x, y });
    if (currentViewport->type != ViewportType::Projection)
    {
        currentViewport->lastClick = { x, y };
    }
}

void Viewport::clickLeftEnd(int x, int y)
{
    if (!currentViewport)
    {
        return;
    }
    
    currentViewport = &chooseViewportMouse({ x, y });
    //make a new brush
    if (currentViewport->type != ViewportType::Projection &&
        toolType == ViewportToolType::Brush &&
        glm::distance(glm::vec2{ currentViewport->lastClick }, glm::vec2{ x, y }) > 0.1f)
    {
        const glm::vec3 beginMousePosition = getPositionFromMouse(*currentViewport, { viewportWidth, viewportHeight }, currentViewport->lastClick);
        
        //round off to the grid, and figure out which axis we don't know about
        const auto [beginMouseRounded, beginMouseAxis] = getRoundedPositionAndAxis(currentViewport->type, beginMousePosition);
        
        const glm::vec3 endMousePosition = getPositionFromMouse(*currentViewport, { viewportWidth, viewportHeight }, { x, y });
        
        //ditto
        const auto [endMouseRounded, endMouseAxis] = getRoundedPositionAndAxis(currentViewport->type, endMousePosition);
        
        editor.createBrush(beginMouseRounded, endMouseRounded, beginMouseAxis);
    }
}

Viewport::ViewportData& Viewport::chooseViewportMouse(glm::ivec2 omouse)
{
    const glm::ivec2 clampedMouse = glm::clamp(omouse, { 0, 0 }, { viewportWidth * 2, viewportHeight * 2 });
    
    for (auto& viewport : viewportDatas)
    {
        const bool inX = clampedMouse.x >= viewport.offset.x &&
                         clampedMouse.x <= viewport.offset.x + viewportWidth;
        const bool inY = clampedMouse.y >= viewport.offset.y &&
                         clampedMouse.y <= viewport.offset.y + viewportHeight;
        
        if (inX && inY)
        {
            return viewport;
        }
    }
    
    throw std::runtime_error("Clicked outside of viewport");
}

void Viewport::render()
{
    if (!gl)
    {
        return;
    }
    
    if (viewportHeight <= 0 || viewportWidth <= 0)
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
        
        glm::mat4 projViewMatrix {1.0f};
        if (viewport.type == ViewportType::Projection)
        {
            projViewMatrix = glm::perspective(glm::radians(90.0f), float(viewportWidth) / float(viewportHeight), 0.1f,
                                              10000.0f);
        }
        else
        {
            const float zoom = viewport.zoom / 100.0f;
            
            const float left = -float(viewportWidth);
            const float right = float(viewportWidth);
            const float down = -float(viewportHeight);
            const float up = float(viewportHeight);
            
            projViewMatrix = glm::ortho(left / zoom, right / zoom, down / zoom, up / zoom,
                                        1.0f, 100000.0f);
        }
        
        projViewMatrix *= viewport.camera.getViewMatrix();
        
        viewport.inverseProjViewMatrix = glm::inverse(projViewMatrix);
        
        if (viewport.gridMesh)
        {
            defaultShader->use();
            defaultShader->setMat4("uProjView", projViewMatrix);
            
            //draw the grid
            viewport.gridMesh->draw(GL_LINES);
        }
        
        //select appropriate brush shader for the viewport
        if (viewport.type == ViewportType::Projection)
        {
            brushShader->use();
            
            brushShader->setMat4("uProjView", projViewMatrix);
            
            gl->Enable(GL_CULL_FACE);
        }
        else
        {
            //we're lines, we always want to show
            gl->DepthFunc(GL_ALWAYS);
        }
        
        //draw all brush meshes
        for (auto& brushMesh: brushMeshes)
        {
            if (viewport.type == ViewportType::Projection)
            {
                brushMesh.draw(GL_TRIANGLE_FAN);
            }
            else
            {
                brushMesh.draw(GL_LINES);
            }
        }
        
        //if we're not projection make sure to reset the depth stuff
        //also get ready to draw the coordinate thing
        if (viewport.type == ViewportType::Projection)
        {
            gl->Disable(GL_CULL_FACE);
            
            defaultShader->use();
            defaultShader->setMat4("uProjView", projViewMatrix);
        }
        
        gl->Enable(GL_LINE_SMOOTH);
        gl->LineWidth(maxLineWidth);
        
        viewport.coordinateMesh->draw(GL_LINES);
        
        gl->LineWidth(1.0f);
        gl->Disable(GL_LINE_SMOOTH);
        
        if (viewport.type != ViewportType::Projection)
        {
            //reset depth func
            gl->DepthFunc(GL_LESS);
        }
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
    viewportWidth = width / 2;
    viewportHeight = height / 2;
}
