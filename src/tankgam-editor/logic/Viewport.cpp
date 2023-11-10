#include "Viewport.h"

#include <string_view>

#include <gl/Vertex.h>
#include <util/Brush.h>

#include "Common.h"
#include "Editor.h"
#include "ViewportHelpers.h"

Viewport::Viewport(FileManager& fileManager, Editor& editor)
    : fileManager{ fileManager }, editor{ editor },
      gl{ nullptr },
      width{ 0 }, height{ 0 }, viewportWidth{ 0 }, viewportHeight{ 0 },
      toolType{ ViewportToolType::Select },
      currentViewport{ nullptr }
{
}

Viewport::~Viewport() = default;

void Viewport::update()
{
    if (gl)
    {
        brushMeshes.clear();
        brushTextureNames.clear();
        const auto brushes = editor.getBrushes();
        for (const auto& brush : brushes)
        {
            const auto textureNames = brush.getTextureNames();
            for (const auto& texture : textureNames)
            {
                if (textures.find(texture) == textures.end())
                {
                    const std::vector<char> textureBuffer = fileManager.readFileRaw(texture);
                    
                    auto textureData = reinterpret_cast<const uint8_t*>(textureBuffer.data());
                    textures.insert(std::pair{ texture, Texture{ *gl, std::span<const uint8_t>{ textureData, textureBuffer.size() } } });
                }
            }
            
            auto verticesAndTexturesList = makeBrushVertices(brush);
            for (auto& [vertices, textureName] : verticesAndTexturesList)
            {
                brushMeshes.emplace_back(*gl, vertices);
                brushTextureNames.push_back(std::move(textureName));
            }
        }
        
        selectedBrushMeshes.clear();
        const auto selectedBrushes = editor.getSelectedBrushes();
        for (const auto& brush : selectedBrushes)
        {
            const auto verticesAndTexturesList = makeBrushVertices(brush, glm::vec3{ 1.0f, 0.0f, 0.0f });
            for (const auto& [vertices, textureName] : verticesAndTexturesList)
            {
                selectedBrushMeshes.emplace_back(*gl, vertices);
            }
        }
        
        const auto selectedFaces = editor.getSelectedFaces();
        for (const auto& face : selectedFaces)
        {
            const auto verticesAndTextures = makeFaceVertices(face, glm::vec3{ 1.0f, 0.0f, 0.0f });
            selectedBrushMeshes.emplace_back(*gl, verticesAndTextures.first);
        }
    }
}

void Viewport::initGL(GladGLContext& glf, int width, int height)
{
    gl = &glf;
    {
        GLfloat lineWidths[2];
        gl->GetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, lineWidths);
        maxLineWidth = std::min(lineWidths[1], 2.0f);
    }
    changeSize(width, height);
    
    gl->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    currentTextureName = editor.getAvailableTextures()[0];
    
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
    selectedBrushMeshes.clear();
    brushTextureNames.clear();
    brushMeshes.clear();
    
    currentViewport = nullptr;
    viewportDatas.clear();
    
    borderMesh.reset();
    
    noProjShader.reset();
    brushColorShader.reset();
    brushShader.reset();
    defaultShader.reset();
    
    textures.clear();
    currentTextureName = "";
    
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

void Viewport::setTextureName(std::string newTextureName)
{
    currentTextureName = std::move(newTextureName);
}

std::string Viewport::getTextureName() const
{
    return currentTextureName;
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
                    float d1 = dot(vNormal, vec3(1.0f / -sqrt(14.0f), 3.0f / sqrt(14.0f), -sqrt(2.0f / 7.0f)));
                    d1 = (d1 / 2.0f) + 0.65f;
                    //outFragColor = vec4(vColor * d1, 1.0f);
                    outFragColor = vec4(texture(diffuseTexture, vTexCoord).rgb * d1, 1.0f);
                })";
    
    brushShader = std::make_unique<Shader>(*gl, DEFAULT_PROJ_VERT, BRUSH_PROJ_FRAG);
    
    constexpr std::string_view BRUSH_COLOR_PROJ_FRAG = R"(#version 420 core
                in vec3 vPos;
                in vec3 vColor;
                in vec3 vNormal;
                in vec2 vTexCoord;

                layout (location = 0) out vec4 outFragColor;

                layout (binding = 0) uniform sampler2D diffuseTexture;

                void main()
                {
                    float d1 = dot(vNormal, vec3(1.0f / sqrt(14.0f), 3.0f / sqrt(14.0f), sqrt(2.0f / 7.0f)));
                    d1 = (d1 / 2.0f) + 0.65f;
                    outFragColor = vec4(vColor * d1, 1.0f);
                    //outFragColor = vec4(texture(diffuseTexture, vTexCoord).rgb * d1, 1.0f);
                })";
    
    brushColorShader = std::make_unique<Shader>(*gl, DEFAULT_PROJ_VERT, BRUSH_COLOR_PROJ_FRAG);
    
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

void Viewport::deleteKey()
{
    if (toolType == ViewportToolType::Select)
    {
        editor.deleteSelectedBrushes();
    }
}

void Viewport::moveSelected(MoveDir moveDir)
{
    if (toolType == ViewportToolType::Select || toolType == ViewportToolType::SelectFace)
    {
        glm::vec3 verticalAxis = currentViewport->camera.getUp();
        glm::vec3 horizontalAxis = currentViewport->camera.getRight();
        if (currentViewport->type == ViewportType::Projection)
        {
            verticalAxis = glm::vec3{ 0.0f, 1.0f, 0.0f };
            horizontalAxis = glm::vec3{ 0.0f, 0.0f, 0.0f };
        }
        
        using Move = MoveDir;
        switch (moveDir)
        {
        case Move::Up:
            editor.moveSelected(verticalAxis * GRID_UNIT);
            break;
        case Move::Down:
            editor.moveSelected(verticalAxis * -GRID_UNIT);
            break;
        case Move::Left:
            editor.moveSelected(horizontalAxis * -GRID_UNIT);
            break;
        case Move::Right:
            editor.moveSelected(horizontalAxis * GRID_UNIT);
            break;
        default:
            throw std::runtime_error{ "Invalid selected brush move" };
        }
    }
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

void Viewport::turnSelected(TurnDir turnDir)
{
    if (toolType == ViewportToolType::Select)
    {
        glm::vec3 rotAxis = currentViewport->camera.getFront();
        if (currentViewport->type == ViewportType::Projection)
        {
            rotAxis = glm::vec3{ 0.0f, 1.0f, 0.0f };
        }
        
        using Turn = TurnDir;
        switch (turnDir)
        {
        case Turn::Left:
            editor.rotateSelected(rotAxis * glm::radians(-5.0f));
            break;
        case Turn::Right:
            editor.rotateSelected(rotAxis * glm::radians(-5.0f));
            break;
        default:
            throw std::runtime_error{ "Invalid selected brush rotate" };
        }
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

void Viewport::clickLeftEnd(int x, int y, bool ctrlHeld)
{
    if (!currentViewport)
    {
        return;
    }
    
    currentViewport = &chooseViewportMouse({ x, y });
    if (toolType == ViewportToolType::Brush)
    {
        //make a new brush
        if (currentViewport->type != ViewportType::Projection &&
            glm::distance(glm::vec2{ currentViewport->lastClick }, glm::vec2{ x, y }) > 0.1f)
        {
            const glm::vec3 beginMousePosition = getPositionFromMouse(*currentViewport, { viewportWidth, viewportHeight }, currentViewport->lastClick);
            
            //round off to the grid, and figure out which axis we don't know about
            const auto [beginMouseRounded, beginMouseAxis] = getRoundedPositionAndAxis(currentViewport->type, beginMousePosition);
            
            const glm::vec3 endMousePosition = getPositionFromMouse(*currentViewport, { viewportWidth, viewportHeight }, { x, y });
            
            //ditto
            const auto [endMouseRounded, endMouseAxis] = getRoundedPositionAndAxis(currentViewport->type, endMousePosition);
            
            editor.createBrush(currentTextureName, beginMouseRounded, endMouseRounded, beginMouseAxis);
        }
    }
    else if (toolType == ViewportToolType::Select)
    {
        //select a brush
        if (currentViewport->type == ViewportType::Projection)
        {
            const glm::vec3 rayDirection = glm::normalize(getPositionFromMouse(*currentViewport, { viewportWidth, viewportHeight }, { x, y }));
            
            const glm::vec3 rayOrigin = currentViewport->camera.getPosition();
            
            editor.selectBrush(rayOrigin, rayDirection, !ctrlHeld);
        }
    }
    else if (toolType == ViewportToolType::SelectFace)
    {
        //select a brush
        if (currentViewport->type == ViewportType::Projection)
        {
            const glm::vec3 rayDirection = glm::normalize(getPositionFromMouse(*currentViewport, { viewportWidth, viewportHeight }, { x, y }));
            
            const glm::vec3 rayOrigin = currentViewport->camera.getPosition();
            
            editor.selectFace(rayOrigin, rayDirection, !ctrlHeld);
        }
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
    
    //for alpha blending
    gl->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //draw all viewports
    for (auto& viewport : viewportDatas)
    {
        gl->Viewport(viewport.offset.x, -viewport.offset.y + viewportHeight, viewportWidth, viewportHeight);
        
        const glm::mat4 projViewMatrix = calculateViewportMatrix(viewport);
        
        renderGrid(viewport, projViewMatrix);
        
        renderBrushes(viewport, projViewMatrix);
        
        defaultShader->use();
        defaultShader->setMat4("uProjView", projViewMatrix);
        
        gl->DepthFunc(GL_LEQUAL);
        gl->Enable(GL_LINE_SMOOTH);
        gl->LineWidth(maxLineWidth);
        
        viewport.coordinateMesh->draw(GL_LINES);
        
        gl->LineWidth(1.0f);
        gl->Disable(GL_LINE_SMOOTH);
        gl->DepthFunc(GL_LESS);
    }
    
    //draw out borders between viewports
    gl->Disable(GL_DEPTH_TEST);
    gl->Viewport(0, 0, width, height);
    
    noProjShader->use();
    borderMesh->draw(GL_TRIANGLES);
}

glm::mat4 Viewport::calculateViewportMatrix(ViewportData& viewport) const
{
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
    
    return projViewMatrix;
}

void Viewport::renderGrid(ViewportData& viewport, const glm::mat4& projViewMatrix)
{
    viewport.inverseProjViewMatrix = glm::inverse(projViewMatrix);
    
    if (viewport.gridMesh)
    {
        defaultShader->use();
        defaultShader->setMat4("uProjView", projViewMatrix);
        
        //draw the grid
        viewport.gridMesh->draw(GL_LINES);
    }
}

void Viewport::renderBrushes(ViewportData& viewport, const glm::mat4& projViewMatrix)
{
    renderVisibleBrushes(viewport, projViewMatrix);
    
    renderSelectedBrushes(viewport, projViewMatrix);
}

void Viewport::renderVisibleBrushes(ViewportData& viewport, const glm::mat4& projViewMatrix)
{
    //select appropriate brush shader for the viewport
    if (viewport.type == ViewportType::Projection)
    {
        brushShader->use();
        brushShader->setMat4("uProjView", projViewMatrix);
    }
    else
    {
        defaultShader->use();
        defaultShader->setMat4("uProjView", projViewMatrix);
        
        //we're lines, we always want to show
        gl->DepthFunc(GL_ALWAYS);
    }
    
    //draw all brush meshes
    for (size_t i = 0; i < brushMeshes.size(); i++)
    {
        auto& brushMesh = brushMeshes[i];
        auto& brushTextureName = brushTextureNames[i];
        if (viewport.type == ViewportType::Projection)
        {
            textures.at(brushTextureName).bind();
            brushMesh.draw(GL_TRIANGLE_FAN);
        }
        else
        {
            brushMesh.draw(GL_LINES);
        }
    }
    
    if (viewport.type != ViewportType::Projection)
    {
        //reset depth func
        gl->DepthFunc(GL_LESS);
    }
}

void Viewport::renderSelectedBrushes(ViewportData& viewport, const glm::mat4& projViewMatrix)
{
    if (toolType != ViewportToolType::Select && toolType != ViewportToolType::SelectFace)
    {
        return;
    }
    
    gl->DepthMask(GL_FALSE);
    gl->DepthFunc(GL_LEQUAL);
    
    //draw out highlighted brushes
    for (auto& brushMesh : selectedBrushMeshes)
    {
        if (viewport.type == ViewportType::Projection)
        {
            brushColorShader->use();
            brushColorShader->setMat4("uProjView", projViewMatrix);
            
            brushMesh.draw(GL_TRIANGLE_FAN);
        }
        else
        {
            defaultShader->use();
            defaultShader->setMat4("uProjView", projViewMatrix);
            
            brushMesh.draw(GL_LINES);
        }
    }
    
    gl->DepthFunc(GL_LESS);
    gl->DepthMask(GL_TRUE);
}

void Viewport::changeSize(int width, int height)
{
    this->width = width;
    this->height = height;
    viewportWidth = width / 2;
    viewportHeight = height / 2;
}
