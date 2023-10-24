#include "Viewport.h"

#include <string_view>

#include <gl/Vertex.h>

#include "Common.h"
#include "Editor.h"
#include "Brush.h"

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

static glm::vec3 centerOfVerticies(std::span<const glm::vec3> verticies)
{
    glm::vec3 centroid{};
    for (const auto& point : verticies)
    {
        centroid += point;
    }
    centroid /= float(verticies.size());
    
    return centroid;
}

static void sortVerticiesClockwise(const glm::vec3& normal, std::vector<glm::vec3>& verticies)
{
    const glm::vec3 centroid = centerOfVerticies(verticies);
    
    using IndexedAngle = std::pair<int, float>;
    
    std::vector<IndexedAngle> angles;
    for (size_t i = 0; i < verticies.size(); i++)
    {
        angles.emplace_back(0, 0.0f);
    }
    
    const glm::vec3 frontOfCenter = centroid + normal;
    
    const glm::vec3& referencePoint = verticies[0];
    const glm::vec3 referenceVector = referencePoint - centroid;
    
    const auto liesOnRightSide = [&frontOfCenter, &referencePoint, &centroid](const glm::vec3& point)
    {
        return glm::dot((frontOfCenter - point), glm::cross(centroid - point, referencePoint - point)) < 0.0f;
    };
    
    for (size_t i = 1; i < verticies.size(); i++)
    {
        const float sinSign = liesOnRightSide(verticies[i]) ? 1.0f : -1.0f;
        const glm::vec3 vector = verticies[i] - centroid;
        const float sinAlpha = glm::length(glm::cross(referenceVector, vector));
        const float cosAlpha = glm::dot(referenceVector, vector);
        const float alpha = std::atan2(sinSign * sinAlpha, cosAlpha);
        angles[i].second = alpha;
        angles[i].first = int(i);
    }
    
    std::sort(angles.begin(), angles.end(), [](const IndexedAngle& a, const IndexedAngle& b)
    {
        return a.second > b.second;
    });
    
    std::vector<glm::vec3> newVerticies;
    newVerticies.reserve(verticies.size());
    
    for (const auto& [vertex, order] : angles)
    {
        newVerticies.push_back(verticies[vertex]);
    }
    
    verticies = std::move(newVerticies);
}

static std::vector<std::vector<Vertex>> makeBrushVertices(const Brush& brush)
{
    const auto planes = brush.getPlanes();
    const auto vertices = brush.getVertices();
    const auto color = brush.getColor();
    
    std::vector<std::vector<Vertex>> verticesList;
    for (const auto& plane : planes)
    {
        std::vector<glm::vec3> planeVertices;
        for (const auto& vertex : vertices)
        {
            if (Plane::classifyPoint(plane, vertex) == Plane::Classification::Coincident)
            {
                planeVertices.push_back(vertex);
            }
        }
        
        if (planeVertices.empty())
        {
            continue;
        }
        
        //sort these all in a clockwise fashion
        sortVerticiesClockwise(plane.normal, planeVertices);
        
        std::vector<Vertex> vertexList;
        vertexList.reserve(planeVertices.size());
        
        for (const auto & edgeVertex : planeVertices)
        {
            vertexList.emplace_back(edgeVertex, color, plane.normal, glm::vec2{});
        }
        
        verticesList.push_back(std::move(vertexList));
    }
    
    return verticesList;
}

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

static glm::vec3 getPositionFromMouse(Viewport::ViewportData& viewport, glm::ivec2 viewportSize, glm::ivec2 mouse)
{
    //normalize mouse to [-1, 1] coordinate system
    const float mouseX = (float(mouse.x) - float(viewport.offset.x)) / (float(viewportSize.x) * 0.5f) - 1.0f;
    const float mouseY = (float(mouse.y) - float(viewport.offset.y)) / (float(viewportSize.y) * 0.5f) - 1.0f;
    
    const glm::vec4 screenPos = glm::vec4{ mouseX, -mouseY, 1.0f, 1.0f };
    const glm::vec4 worldPos = viewport.inverseProjViewMatrix * screenPos;
    
    return glm::vec3{ worldPos };
}

static std::pair<glm::vec2, int> getRoundedPositionAndAxis(Viewport::ViewportType type, const glm::vec3& position)
{
    int skipAxis = getSkipAxis(type);
    
    const float axisX = position[(skipAxis + 1) % 3];
    const float axisY = position[(skipAxis + 2) % 3];
    
    constexpr auto roundNumber = [](float number) -> float
    {
        return std::round(number / GRID_UNIT) * GRID_UNIT;
    };
    
    const float roundedX = roundNumber(std::round(axisX));
    const float roundedY = roundNumber(std::round(axisY));
    
    return { { roundedX, roundedY }, skipAxis };
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
        
        //select appropriate brush shader for the viewport
        if (viewport.type == ViewportType::Projection)
        {
            brushShader->use();
            
            brushShader->setMat4("uProjView", projViewMatrix);
            
            gl->Enable(GL_CULL_FACE);
        }
        else
        {
            defaultShader->use();
            
            defaultShader->setMat4("uProjView", projViewMatrix);
            
            //we're lines, we always want to show
            gl->DepthFunc(GL_ALWAYS);
        }
        
        //draw all brush meshes
        for (auto& brushMesh : brushMeshes)
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
        if (viewport.type == ViewportType::Projection)
        {
            gl->Disable(GL_CULL_FACE);
        }
        else
        {
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
