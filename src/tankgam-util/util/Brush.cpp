#include "util/Brush.h"

#include <array>
#include <algorithm>
#include <random>
#include <stdexcept>

#include <fmt/format.h>

Brush::Brush(std::span<const std::string> brushTextureNames, std::span<const float> brushTextureScales, std::span<const Plane> brushPlanes, glm::vec3 color)
    : color{ color }
{
    //just some error checking
    {
        const size_t s1 = brushTextureNames.size();
        const size_t s2 = brushTextureScales.size();
        const size_t s3 = brushPlanes.size();
        
        if (s1 != s2 || s2 != s3)
        {
            throw std::runtime_error{ fmt::format("Brush constructor arrays out of sync {} {} {}", s1, s2, s3) };
        }
    }
    
    faces.planes.resize(brushPlanes.size());
    std::copy(brushPlanes.begin(), brushPlanes.end(), faces.planes.begin());
    
    faces.textureIndices.resize(faces.planes.size());
    faces.textureScales.resize(faces.planes.size());
    for (size_t i = 0; i < brushTextureNames.size(); i++)
    {
        setTextureName(i, brushTextureNames[i]);
        faces.textureScales[i] = brushTextureScales[i];
    }
    
    regenerateVertices();
}

Brush::Brush(std::string textureName, float textureScale, std::span<const Plane> brushPlanes, glm::vec3 color)
    : color{ color }
{
    faces.planes.resize(brushPlanes.size());
    std::copy(brushPlanes.begin(), brushPlanes.end(), faces.planes.begin());
    
    textureNames.push_back(std::move(textureName));
    
    faces.textureIndices.resize(faces.planes.size());
    faces.textureScales.resize(faces.planes.size());
    std::fill(faces.textureIndices.begin(), faces.textureIndices.end(), 0);
    std::fill(faces.textureScales.begin(), faces.textureScales.end(), textureScale);
    
    regenerateVertices();
}

Brush::Brush(std::string textureName, float textureScale, glm::vec3 beginVec, glm::vec3 endVec)
{
    const glm::vec3 up{ 0.0f, 1.0f, 0.0f };
    const glm::vec3 right{ 1.0f, 0.0f, 0.0f };
    const glm::vec3 back{ 0.0f, 0.0f, 1.0f };
    
    faces.planes.push_back(Plane::fromVertexAndNormal(beginVec, up));
    faces.planes.push_back(Plane::fromVertexAndNormal(beginVec, right));
    faces.planes.push_back(Plane::fromVertexAndNormal(beginVec, back));

    faces.planes.push_back(Plane::fromVertexAndNormal(endVec, -up));
    faces.planes.push_back(Plane::fromVertexAndNormal(endVec, -right));
    faces.planes.push_back(Plane::fromVertexAndNormal(endVec, -back));
    
    textureNames.push_back(std::move(textureName));
    
    faces.textureIndices.resize(faces.planes.size());
    faces.textureScales.resize(faces.planes.size());
    std::fill(faces.textureIndices.begin(), faces.textureIndices.end(), 0);
    std::fill(faces.textureScales.begin(), faces.textureScales.end(), textureScale);
    
    regenerateVertices();
    
    color = randomWireframeColor();
}

Brush::~Brush() = default;

Brush::Brush(const Brush& o) = default;

Brush& Brush::operator=(const Brush& o)
{
    if (this == &o)
    {
        return *this;
    }
    
    faces = o.faces;
    
    return *this;
}

size_t Brush::getNumFaces() const
{
    return faces.planes.size();
}

void Brush::setTextureName(size_t faceNum, std::string_view newName)
{
    if (std::find(textureNames.begin(), textureNames.end(), newName) == textureNames.end())
    {
        textureNames.emplace_back(newName.data());
    }
    const auto it = std::find(textureNames.begin(), textureNames.end(), newName);
    
    const size_t loc = std::distance(textureNames.begin(), it);
    faces.textureIndices[faceNum] = loc;
}

std::string Brush::getTextureName(size_t faceNum) const
{
    return textureNames[faces.textureIndices[faceNum]];
}

std::vector<std::string> Brush::getTextureNames() const
{
    return textureNames;
}

float Brush::getTextureScale(size_t faceNum) const
{
    return faces.textureScales[faceNum];
}

void Brush::setTextureScale(size_t faceNum, float newScale)
{
    faces.textureScales[faceNum] = newScale;
}

std::vector<BrushFace> Brush::getFaces() const
{
    std::vector<BrushFace> brushFaces;
    for (size_t i = 0; i < faces.planes.size(); i++)
    {
        brushFaces.emplace_back(textureNames[faces.textureIndices[i]], faces.textureScales[i],
                                faces.planes[i], faces.verticesList[i]);
    }
    
    return brushFaces;
}

BrushFace Brush::getFace(size_t faceNum) const
{
    return BrushFace
    {
        textureNames[faces.textureIndices[faceNum]], faces.textureScales[faceNum],
        faces.planes[faceNum], faces.verticesList[faceNum]
    };
}

glm::vec3 Brush::getColor() const
{
    return color;
}

std::optional<glm::vec3> Brush::getIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection) const
{
    //test if point is on backside of all faces
    std::optional<glm::vec3> finalIntersection;
    float closestFaceDistance = std::numeric_limits<float>::max();
    for (size_t i = 0; i < faces.planes.size(); i++)
    {
        std::optional<glm::vec3> intersection = Plane::intersectRay(faces.planes[i], rayOrigin, rayDirection);
        if (intersection.has_value())
        {
            bool inside = true;
            
            for (const auto& otherPlane : faces.planes)
            {
                if (Plane::classifyPoint(otherPlane, intersection.value()) == Plane::Classification::Front)
                {
                    inside = false;
                    break;
                }
            }
            
            if (inside)
            {
                const float faceDistance = glm::distance(intersection.value(), rayOrigin);
                if (faceDistance < closestFaceDistance)
                {
                    finalIntersection = intersection.value();
                    closestFaceDistance = faceDistance;
                }
            }
        }
    }
    
    return finalIntersection;
}

void Brush::translate(glm::vec3 direction)
{
    for (auto& plane : faces.planes)
    {
        Plane::translatePlane(plane, direction);
    }
    
    regenerateVertices();
}

void Brush::translate(size_t faceNum, glm::vec3 direction)
{
    Plane::translatePlane(faces.planes[faceNum], direction);
    
    regenerateVertices();
}

void Brush::rotate(glm::vec3 rotation)
{
    for (auto& plane : faces.planes)
    {
        Plane::rotatePlane(plane, rotation, center);
    }
    
    regenerateVertices();
}

void Brush::rotate(size_t faceNum, glm::vec3 rotation)
{
    const glm::vec3 newCenter = calculateCenter(faces.verticesList[faceNum]);
    Plane::rotatePlane(faces.planes[faceNum], rotation, newCenter);
    
    regenerateVertices();
}

void Brush::regenerateVertices()
{
    vertices = generateVertices();
    
    const glm::vec3 newCenter = calculateCenter(vertices);
    
    //flip around planes to face this point
    for (auto& plane : faces.planes)
    {
        if (Plane::classifyPoint(plane, newCenter) == Plane::Classification::Front)
        {
            plane.normal = -plane.normal;
            plane.distance = -plane.distance;
        }
    }
    
    vertices = generateVertices(true);
    
    center = calculateCenter(vertices);
    
    faces.verticesList.resize(faces.planes.size());
    for (auto& faceVertices : faces.verticesList)
    {
        faceVertices.clear();
    }
    
    for (const auto& vertex : vertices)
    {
        for (size_t i = 0; i < faces.planes.size(); i++)
        {
            const auto& plane = faces.planes[i];
            if (Plane::classifyPoint(plane, vertex) == Plane::Classification::Coincident)
            {
                faces.verticesList[i].push_back(vertex);
            }
        }
    }
}

std::vector<glm::vec3> Brush::generateVertices(bool checkOutside)
{
    std::vector<glm::vec3> newVertices;
    for (size_t i = 0; i < faces.planes.size() ; i++)
    {
        for (size_t j = 0; j < faces.planes.size(); j++)
        {
            if (j == i)
            {
                continue;
            }
            
            for (size_t k = 0; k < faces.planes.size(); k++)
            {
                if (k == i || k == j)
                {
                    continue;
                }
                
                const auto possibleVertex = Plane::intersectPlanes(faces.planes[i], faces.planes[j], faces.planes[k]);
                
                //check if this was a valid singular intersection
                if (!possibleVertex.has_value())
                {
                    continue;
                }
                
                const auto vertex = possibleVertex.value();
                
                //check we don't already have this
                bool similar = false;
                for (const auto& otherVertex : newVertices)
                {
                    if (glm::distance(vertex, otherVertex) <= 0.01f)
                    {
                        similar = true;
                        break;
                    }
                }
                
                if (similar)
                {
                    break;
                }
                
                if (checkOutside)
                {
                    //make sure this isn't outside of any of the planes
                    bool outSide = false;
                    for (const auto& checkPlane : faces.planes)
                    {
                        if (Plane::classifyPoint(checkPlane, vertex) == Plane::Classification::Front)
                        {
                            outSide = true;
                            break;
                        }
                    }
                    
                    if (outSide)
                    {
                        continue;
                    }
                }
                
                newVertices.push_back(vertex);
            }
        }
    }
    
    return newVertices;
}

glm::vec3 Brush::calculateCenter(std::span<const glm::vec3> centerVertices)
{
    glm::vec3 newCenter{};
    size_t vertexCount = 0;
    //calculate center
    for (const auto& vertex : centerVertices)
    {
        newCenter += vertex;
        vertexCount++;
    }
    
    newCenter /= static_cast<float>(vertexCount);
    
    return newCenter;
}

glm::vec3 Brush::randomWireframeColor()
{
    //for a nice wireframe color
    //TODO: make this stored or something
    std::random_device dev{};
    std::mt19937 rng{ dev() };
    std::uniform_int_distribution colorGenerator{ 125, 225 };
    
    return glm::vec3
    {
        0.0f,
        static_cast<float>(colorGenerator(rng)) / 255.0f,
        static_cast<float>(colorGenerator(rng)) / 255.0f
    };
}
