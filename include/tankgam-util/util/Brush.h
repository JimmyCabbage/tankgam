#pragma once

#include <vector>
#include <string>
#include <optional>

#include <glm/glm.hpp>

#include <util/Plane.h>

class Brush
{
public:
    Brush(std::string textureName, float textureScale, glm::vec3 beginVec, glm::vec3 endVec);
    ~Brush();
    
    Brush(const Brush& o);
    Brush& operator=(const Brush& o);
    
    void setTextureName(std::string newName);
    
    std::string_view getTextureName() const;
    
    float getTextureScale() const;
    
    std::span<const Plane> getPlanes() const;
    
    std::span<const glm::vec3> getVertices() const;
    
    glm::vec3 getColor() const;
    
    std::optional<glm::vec3> getIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection) const;
    
    void translate(glm::vec3 direction);
    
private:
    std::string textureName;
    float textureScale;
    std::vector<Plane> planes;
    
    std::vector<glm::vec3> vertices;
    void regenerateVertices();
    std::vector<glm::vec3> generateVertices(bool checkOutside = false);
    
    glm::vec3 color;
    glm::vec3 randomWireframeColor();
};
