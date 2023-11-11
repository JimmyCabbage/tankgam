#pragma once

#include <vector>
#include <string>
#include <optional>
#include <span>

#include <glm/glm.hpp>

#include <util/Plane.h>

struct BrushFace
{
    std::string textureName;
    float textureScale;
    Plane plane;
    std::vector<glm::vec3> vertices;
};

class Brush
{
public:
    Brush(std::span<const std::string> brushTextureNames, std::span<const float> brushTextureScales, std::span<const Plane> brushPlanes, glm::vec3 color);
    Brush(std::string textureName, float textureScale, std::span<const Plane> brushPlanes, glm::vec3 color);
    Brush(std::string textureName, float textureScale, glm::vec3 beginVec, glm::vec3 endVec);
    ~Brush();
    
    Brush(const Brush& o);
    Brush& operator=(const Brush& o);
    
    size_t getNumFaces() const;
    
    void setTextureName(size_t faceNum, std::string_view newName);
    
    std::string getTextureName(size_t faceNum) const;
    
    std::vector<std::string> getTextureNames() const;
    
    float getTextureScale(size_t faceNum) const;
    
    void setTextureScale(size_t faceNum, float newScale);
    
    std::vector<BrushFace> getFaces() const;
    
    BrushFace getFace(size_t faceNum) const;
    
    glm::vec3 getColor() const;
    
    std::optional<glm::vec3> getIntersection(glm::vec3 rayOrigin, glm::vec3 rayDirection) const;
    
    void translate(glm::vec3 direction);
    
    void translate(size_t faceNum, glm::vec3 direction);
    
    void rotate(glm::vec3 rotation);
    
    void rotate(size_t faceNum, glm::vec3 rotation);
    
private:
    struct BrushFaces
    {
        std::vector<size_t> textureIndices;
        std::vector<float> textureScales;
        std::vector<Plane> planes;
        std::vector<std::vector<glm::vec3>> verticesList;
    } faces;
    
    std::vector<std::string> textureNames;
    
    std::vector<glm::vec3> vertices;
    void regenerateVertices(bool regenerateCenter = true);
    std::vector<glm::vec3> generateVertices(bool checkOutside = false);
    
    glm::vec3 center;
    glm::vec3 calculateCenter(std::span<const glm::vec3> centerVertices);
    
    glm::vec3 color;
    glm::vec3 randomWireframeColor();
};
