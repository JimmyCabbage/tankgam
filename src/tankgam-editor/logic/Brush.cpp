#include "Brush.h"

#include <array>
#include <random>

Brush::Brush(std::string textureName, glm::vec3 beginVec, glm::vec3 endVec)
    : textureName{ std::move(textureName) }, planes{}
{
    const glm::vec3 up{ 0.0f, 1.0f, 0.0f };
    const glm::vec3 right{ 1.0f, 0.0f, 0.0f };
    const glm::vec3 back{ 0.0f, 0.0f, 1.0f };
    
    planes.push_back(Plane::fromVertexAndNormal(beginVec, up));
    planes.push_back(Plane::fromVertexAndNormal(beginVec, right));
    planes.push_back(Plane::fromVertexAndNormal(beginVec, back));
    
    planes.push_back(Plane::fromVertexAndNormal(endVec, -up));
    planes.push_back(Plane::fromVertexAndNormal(endVec, -right));
    planes.push_back(Plane::fromVertexAndNormal(endVec, -back));
    
    vertices = generateVertices();
    
    glm::vec3 center{};
    size_t vertexCount = 0;
    //calculate center
    for (const auto& vertex : vertices)
    {
        center += vertex;
        vertexCount++;
    }
    
    center /= static_cast<float>(vertexCount);
    
    //flip around planes to face this point
    for (auto& plane : planes)
    {
        if (Plane::classifyPoint(plane, center) == Plane::Classification::Front)
        {
            plane.normal = -plane.normal;
            plane.distance = -plane.distance;
        }
    }
    
    vertices = generateVertices(true);
    
    color = randomWireframeColor();
}

Brush::~Brush() = default;

void Brush::setTextureName(std::string newName)
{
    textureName = std::move(newName);
}

std::string_view Brush::getTextureName() const
{
    return textureName;
}

std::span<const Plane> Brush::getPlanes() const
{
    return planes;
}

std::span<const glm::vec3> Brush::getVertices() const
{
    return vertices;
}

glm::vec3 Brush::getColor() const
{
    return color;
}

std::vector<glm::vec3> Brush::generateVertices(bool checkOutside)
{
    std::vector<glm::vec3> newVertices;
    for (size_t i = 0; i < planes.size() ; i++)
    {
        for (size_t j = 0; j < planes.size(); j++)
        {
            if (j == i)
            {
                continue;
            }
            
            for (size_t k = 0; k < planes.size(); k++)
            {
                if (k == i || k == j)
                {
                    continue;
                }
                
                const auto possibleVertex = Plane::intersectPlanes(planes[i], planes[j], planes[k]);
                
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
                    for (const auto& checkPlane: planes)
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
