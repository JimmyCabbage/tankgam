#include "Brush.h"

#include <array>

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
    
    glm::vec3 center{};
    size_t vertexCount = 0;
    //calculate center
    for (size_t i = 0; i < planes.size(); i++)
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
                
                center += vertex;
                vertexCount++;
            }
        }
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
