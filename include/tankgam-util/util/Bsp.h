#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <cstdint>

#include <glm/glm.hpp>

#include <util/Plane.h>

namespace bsp
{
    std::pair<glm::vec3, glm::vec3> getTextureAxisFromNormal(const glm::vec3& normal);
    
    using StringLength = uint8_t;
    using SmallArrayLength = uint16_t;
    using ArrayLength = uint32_t;
    
    struct Header
    {
        std::string mapName;
    };
    
    struct TextureInfo
    {
        glm::vec3 uAxis;
        float uOffset;
        
        glm::vec3 vAxis;
        float vOffset;
        
        float scale;
        
        SmallArrayLength textureIndex;
        
        bool operator<=>(const TextureInfo& o) const = default;
    };
    
    struct Edge
    {
        ArrayLength startVertex;
        ArrayLength endVertex;
    };
    
    struct Face
    {
        ArrayLength firstEdge;
        ArrayLength numEdges;
        
        ArrayLength plane;
        
        ArrayLength textureInfoIndex;
    };
    
    struct Node
    {
        ArrayLength splitPlane;
        
        int64_t frontChild; //an index
        int64_t backChild;  //positive if node, negative if leaf
        
        ArrayLength firstFace;
        ArrayLength numFaces;
    };
    
    struct Leaf
    {
        uint8_t content;
        
        ArrayLength firstFace;
        ArrayLength numFaces;
    };
    
    struct File
    {
        Header header;
        
        std::vector<std::string> textureNames;
        
        std::vector<TextureInfo> textureInfos;
        
        std::vector<Plane> planes;
        
        std::vector<glm::vec3> vertices;
        std::vector<Edge> edges;
        std::vector<Face> faces;
        
        std::vector<Node> nodes;
        std::vector<Leaf> leaves;
    };
    
    File parseBspFile(std::string_view bspFileName, std::stringstream& bspFileStream);
    
    void writeFile(std::string_view bspFileName, File& bspFile);
}
