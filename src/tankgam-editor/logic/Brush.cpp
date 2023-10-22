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
