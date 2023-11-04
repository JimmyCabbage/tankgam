#pragma once

#include <vector>
#include <string>
#include <optional>

#include <glm/glm.hpp>

#include "Plane.h"

class Brush
{
public:
    Brush(std::string textureName, glm::vec3 beginVec, glm::vec3 endVec);
    ~Brush();
    
    void setTextureName(std::string newName);
    std::string_view getTextureName() const;
    
    std::span<const Plane> getPlanes() const;
    
    std::span<const glm::vec3> getVertices() const;
    
    glm::vec3 getColor() const;
    
    //raycast to find the closest plane
    std::optional<size_t> getPlaneNum(glm::vec3 rayOrigin, glm::vec3 rayDirection) const;
    
private:
    std::string textureName;
    std::vector<Plane> planes;
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> generateVertices(bool checkOutside = false);
    
    glm::vec3 color;
    glm::vec3 randomWireframeColor();
};
