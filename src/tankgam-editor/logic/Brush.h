#pragma once

#include <vector>
#include <string>

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
    
private:
    std::string textureName;
    std::vector<Plane> planes;
};
