#include "ViewportHelpers.h"

#include <util/Bsp.h>

#include "Common.h"

std::array<Vertex, 12> generateBorders()
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
        Vertex{ .position = { hcTopLeft }, .color = color, .normal = {}, .texCoord = {} },
        Vertex{ .position = { hcTopRight }, .color = color, .normal = {}, .texCoord = {} },
        Vertex{ .position = { hcBottomRight }, .color = color, .normal = {}, .texCoord = {} },
        //tri2 horizontal
        Vertex{ .position = { hcBottomLeft }, .color = color, .normal = {}, .texCoord = {} },
        Vertex{ .position = { hcTopLeft }, .color = color, .normal = {}, .texCoord = {} },
        Vertex{ .position = { hcBottomRight }, .color = color, .normal = {}, .texCoord = {} },
        //tri1 horizontal
        Vertex{ .position = { vcTopLeft }, .color = color, .normal = {}, .texCoord = {} },
        Vertex{ .position = { vcTopRight }, .color = color, .normal = {}, .texCoord = {} },
        Vertex{ .position = { vcBottomRight }, .color = color, .normal = {}, .texCoord = {} },
        //tri2 horizontal
        Vertex{ .position = { vcBottomLeft }, .color = color, .normal = {}, .texCoord = {} },
        Vertex{ .position = { vcTopLeft }, .color = color, .normal = {}, .texCoord = {} },
        Vertex{ .position = { vcBottomRight }, .color = color, .normal = {}, .texCoord = {} },
    };
    
    return verticies;
}

glm::vec3 centerOfVerticies(std::span<const glm::vec3> verticies)
{
    glm::vec3 centroid{};
    for (const auto& point : verticies)
    {
        centroid += point;
    }
    centroid /= float(verticies.size());
    
    return centroid;
}

void sortVerticiesClockwise(const glm::vec3& normal, std::vector<glm::vec3>& verticies)
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

static Vertex makeVertexFromData(glm::vec3 pos, glm::vec3 normal, float textureScale, glm::vec3 color)
{
    const auto [uAxis, vAxis] = bsp::getTextureAxisFromNormal(normal);
    const glm::vec2 texCoord = glm::vec2{ glm::dot(pos, uAxis), glm::dot(pos, vAxis) } / textureScale;
    return Vertex{ pos, color, normal, texCoord };
}

std::pair<std::vector<Vertex>, std::string> makeFaceVertices(const BrushFace& face, glm::vec3 color)
{
    std::vector<glm::vec3> vs = face.vertices;
    
    //sort these all in a clockwise fashion
    sortVerticiesClockwise(face.plane.normal, vs);
    
    std::vector<Vertex> vertexList;
    vertexList.reserve(vs.size());
    
    for (const auto& edgeVertex : vs)
    {
        vertexList.push_back(makeVertexFromData(edgeVertex, face.plane.normal, face.textureScale, color));
    }
    
    return std::make_pair(std::move(vertexList), std::move(face.textureName));
}

std::vector<std::pair<std::vector<Vertex>, std::string>> makeBrushVertices(const Brush& brush, glm::vec3 overrideColor)
{
    auto faces = brush.getFaces();
    const auto color = overrideColor == glm::vec3{} ? brush.getColor() : overrideColor;
    
    std::vector<std::pair<std::vector<Vertex>, std::string>> verticesAndTexturesList;
    for (auto& face : faces)
    {
        if (face.vertices.empty())
        {
            continue;
        }
        
        //sort these all in a clockwise fashion
        sortVerticiesClockwise(face.plane.normal, face.vertices);
        
        std::vector<Vertex> vertexList;
        vertexList.reserve(face.vertices.size());
        
        for (const auto& edgeVertex : face.vertices)
        {
            vertexList.push_back(makeVertexFromData(edgeVertex, face.plane.normal, face.textureScale, color));
        }
        
        auto pair = std::make_pair(std::move(vertexList), std::move(face.textureName));
        verticesAndTexturesList.push_back(std::move(pair));
    }
    
    return verticesAndTexturesList;
}

ViewportCamera setupCamera(Viewport::ViewportType type)
{
    constexpr float initialDistance = 10000.0f;
    
    constexpr float middleGrid = 0.0f;
    
    using Type = Viewport::ViewportType;
    switch (type)
    {
    case Type::Top:
        return ViewportCamera{ { middleGrid, initialDistance, middleGrid }, { 0.0f, -1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } };
    case Type::Front:
        return ViewportCamera{ { middleGrid, middleGrid, -initialDistance }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } };
    case Type::Side:
        return ViewportCamera{ { initialDistance, middleGrid, middleGrid }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
    case Type::Projection:
        return ViewportCamera{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } };
    default:
        throw std::runtime_error("Unknown ViewportType enum");
    }
}

constexpr int getSkipAxis(Viewport::ViewportType type)
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

std::vector<Vertex> generateGrid(Viewport::ViewportType type)
{
    std::vector<Vertex> verticies;
    
    //make grid for 2d views
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
    
    return verticies;
}

std::vector<Vertex> generateCoordinates()
{
    std::vector<Vertex> verticies;
    
    //make center xyz thing always
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
    
    return verticies;
}

glm::vec3 getPositionFromMouse(Viewport::ViewportData& viewport, glm::ivec2 viewportSize, glm::ivec2 mouse)
{
    //normalize mouse to [-1, 1] coordinate system
    const float mouseX = (float(mouse.x) - float(viewport.offset.x)) / (float(viewportSize.x) * 0.5f) - 1.0f;
    const float mouseY = (float(mouse.y) - float(viewport.offset.y)) / (float(viewportSize.y) * 0.5f) - 1.0f;
    
    const glm::vec4 screenPos = glm::vec4{ mouseX, -mouseY, 1.0f, 1.0f };
    const glm::vec4 worldPos = viewport.inverseProjViewMatrix * screenPos;
    
    return glm::vec3{ worldPos };
}

std::pair<glm::vec2, int> getRoundedPositionAndAxis(Viewport::ViewportType type, const glm::vec3& position)
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
