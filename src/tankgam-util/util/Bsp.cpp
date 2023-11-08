#include "util/Bsp.h"

#include <fstream>
#include <sstream>

#include <fmt/format.h>

static constexpr uint8_t MAJOR_VERSION = 0;
static constexpr uint16_t MINOR_VERSION = 1;
static constexpr uint8_t PATCH_VERSION = 0;
static constexpr std::string_view FILE_MAGIC_NUMBER = "STMF";

std::pair<glm::vec3, glm::vec3> bsp::getTextureAxisFromNormal(const glm::vec3& normal)
{
    constexpr std::array<glm::vec3, 18> baseAxises
    {
        glm::vec3{0,1,0}, {0,0,1}, {-1,0,0},		// floor
        glm::vec3{0,-1,0}, {0,0,1}, {-1,0,0},		// ceiling
        glm::vec3{0,0,1}, {1,0,0}, {0,1,0},		// west wall
        glm::vec3{0,0,-1}, {-1,0,0}, {0,1,0},		// east wall
        glm::vec3{1,0,0}, {0,0,-1}, {0,1,0},		// south wall
        glm::vec3{-1,0,0}, {0,0,1}, {0,1,0}		// north wall
    };
    
    size_t bestAxis = 0;
    float best = 0.0f;
    
    for (size_t i = 0; i < 6; i++)
    {
        const float dot = glm::dot(normal, baseAxises[i * 3]);
        if (dot > best)
        {
            best = dot;
            bestAxis = i;
        }
    }
    
    return { baseAxises[bestAxis * 3 + 1], baseAxises[bestAxis * 3 + 2] };
}

bsp::File bsp::parseBspFile(std::string_view bspFileName, std::stringstream& file)
{
    const auto readNumber = [&file]<typename T>()
    {
        T number;
        file.read(reinterpret_cast<char*>(&number), sizeof number);
        
        return number;
    };
    
    const auto readNumber_i64 = [readNumber]()
    {
        return readNumber.template operator()<int64_t>();
    };
    
    const auto readNumber_u32 = [readNumber]()
    {
        return readNumber.template operator()<uint32_t>();
    };
    
    const auto readNumber_u16 = [readNumber]()
    {
        return readNumber.template operator()<uint16_t>();
    };
    
    const auto readNumber_u8 = [readNumber]()
    {
        return readNumber.template operator()<uint8_t >();
    };
    
    const auto readNumber_f = [readNumber]()
    {
        return readNumber.template operator()<float>();
    };
    
    //small array length
    const auto readNumber_sAL = readNumber_u16;
    //normal array length
    const auto readNumber_AL = readNumber_u32;
    
    const auto readVertex = [readNumber_f]()
    {
        glm::vec3 vertex;
        vertex.x = readNumber_f();
        vertex.y = readNumber_f();
        vertex.z = readNumber_f();
        
        return vertex;
    };
    
    const auto readString = [&file, readNumber_u8]()
    {
        const uint8_t strLength = readNumber_u8();
        
        std::string string;
        string.resize(strLength);
        file.read(string.data(), strLength);
        
        return string;
    };
    
    bsp::File bspFile;
    
    {
        std::string mapFormat;
        mapFormat.resize(4);
        file.read(mapFormat.data(), mapFormat.size());
        if (mapFormat != FILE_MAGIC_NUMBER)
        {
            throw std::runtime_error(fmt::format("Map {} has incorrect magic number", bspFileName.data()));
        }
        
        const uint8_t fileMajorVersion = readNumber_u8();
        const uint8_t fileMinorVersion = readNumber_u16();
        const uint8_t filePatchVersion = readNumber_u8();
        //todo: convert from old map file version when we reach stable versioning
        if (fileMajorVersion != MAJOR_VERSION || fileMinorVersion != MINOR_VERSION)
        {
            throw std::runtime_error(fmt::format("Map {} has incompatible version of {}.{}, when we want at least {}.{}",
                                                 bspFileName.data(),
                                                 fileMajorVersion, fileMinorVersion,
                                                 MAJOR_VERSION, MINOR_VERSION));
        }
        
        bspFile.header.mapName = readString();
    }
    {
        const bsp::SmallArrayLength numTextures = readNumber_sAL();
        
        for (bsp::SmallArrayLength i = 0; i < numTextures; i++)
        {
            const std::string texture = readString();
            
            if (texture.empty())
            {
                continue;
            }
            
            bspFile.textureNames.push_back(texture);
        }
    }
    {
        const bsp::ArrayLength numTextureInfos = readNumber_AL();
        
        for (bsp::ArrayLength i = 0; i < numTextureInfos; i++)
        {
            bsp::TextureInfo texInfo{};
            
            texInfo.uAxis = readVertex();
            texInfo.uOffset = readNumber_f();
            
            texInfo.vAxis = readVertex();
            texInfo.vOffset = readNumber_f();
            
            texInfo.scale = readNumber_f();
            
            texInfo.textureIndex = readNumber_u16();
            
            bspFile.textureInfos.push_back(texInfo);
        }
    }
    /*
    {
        const bsp::ArrayLength numEntities = readNumber_AL();
        
        for (bsp::ArrayLength i = 0; i < numEntities; i++)
        {
            BspEntity entity{};
            
            entity.type = readString();
            
            if (entity.type.empty())
            {
                continue;
            }
            
            entity.name = readString();
            
            entity.position = readVertex();
            
            const BspSmallArrayLength numProperties = readNumber_sAL();
            
            for (BspSmallArrayLength j = 0; j < numProperties; j++)
            {
                entity.properties.emplace_back(readString());
            }
            
            bspFile.entities.push_back(entity);
        }
    }
    */
    {
        const bsp::ArrayLength numPlanes = readNumber_AL();
        
        for (bsp::ArrayLength i = 0; i < numPlanes; i++)
        {
            Plane plane{};
            plane.normal = readVertex();
            plane.distance = readNumber_f();
            
            bspFile.planes.push_back(plane);
        }
    }
    {
        const bsp::ArrayLength numVertices = readNumber_AL();
        
        for (bsp::ArrayLength i = 0; i < numVertices; i++)
        {
            bspFile.vertices.push_back(readVertex());
        }
    }
    {
        const bsp::ArrayLength numEdges = readNumber_AL();
        
        for (bsp::ArrayLength i = 0; i < numEdges; i++)
        {
            bsp::Edge edge{};
            edge.startVertex = readNumber_u32();
            edge.endVertex = readNumber_u32();
            
            bspFile.edges.push_back(edge);
        }
    }
    {
        const bsp::ArrayLength numFaces = readNumber_AL();
        
        for (bsp::ArrayLength i = 0; i < numFaces; i++)
        {
            bsp::Face face{};
            face.firstEdge = readNumber_u32();
            face.numEdges = readNumber_u32();
            face.plane = readNumber_u32();
            face.textureInfoIndex = readNumber_u32();
            
            bspFile.faces.push_back(face);
        }
    }
    {
        const bsp::ArrayLength numNodes = readNumber_AL();
        
        for (bsp::ArrayLength i = 0; i < numNodes; i++)
        {
            bsp::Node node{};
            
            node.splitPlane = readNumber_u32();
            
            node.frontChild = readNumber_i64();
            node.backChild = readNumber_i64();
            
            node.firstFace = readNumber_u32();
            node.numFaces = readNumber_u32();
            
            bspFile.nodes.push_back(node);
        }
    }
    {
        const bsp::ArrayLength numLeaves = readNumber_AL();
        
        for (bsp::ArrayLength i = 0; i < numLeaves; i++)
        {
            bsp::Leaf leaf{};
            
            leaf.content = readNumber_u8();
            
            leaf.firstFace = readNumber_u32();
            leaf.numFaces = readNumber_u32();
            
            bspFile.leaves.push_back(leaf);
        }
    }
    
    return bspFile;
}

void bsp::writeFile(std::string_view bspFileName, File& bspFile)
{
    std::ofstream file
    {
        bspFileName.data(),
        std::ios::out | std::ios::binary
    };
    
    if (!file.is_open())
    {
        return;
    }
    
    const auto writeNumber = [&file](const auto number)
    {
        file.write(reinterpret_cast<const char*>(&number), sizeof(number));
    };
    
    const auto writeVertex = [writeNumber](const glm::vec3& vertex)
    {
        writeNumber(vertex.x);
        writeNumber(vertex.y);
        writeNumber(vertex.z);
    };
    
    const auto writeString = [&file](const std::string_view string)
    {
        const size_t maxLen = std::max(string.size(), static_cast<size_t>(std::numeric_limits<StringLength>::max()));
        const auto strLength = static_cast<StringLength>(maxLen);
        file.write(reinterpret_cast<const char*>(&strLength), sizeof strLength);
        file.write(string.data(), strLength);
    };
    
    //header versioning info
    file.write(FILE_MAGIC_NUMBER.data(), FILE_MAGIC_NUMBER.size());
    writeNumber(MAJOR_VERSION);
    writeNumber(MINOR_VERSION);
    writeNumber(PATCH_VERSION);
    
    writeString(bspFile.header.mapName);
    
    {
        const auto textureNamesLength = static_cast<bsp::SmallArrayLength>(bspFile.textureNames.size());
        writeNumber(textureNamesLength);
        
        for (std::string_view textureName : bspFile.textureNames)
        {
            writeString(textureName);
        }
    }
    {
        const auto textureInfosLength = static_cast<bsp::ArrayLength>(bspFile.textureInfos.size());
        writeNumber(textureInfosLength);
        
        for (const auto& textureInfo : bspFile.textureInfos)
        {
            writeVertex(textureInfo.uAxis);
            writeNumber(textureInfo.uOffset);
            
            writeVertex(textureInfo.vAxis);
            writeNumber(textureInfo.vOffset);
            
            writeNumber(textureInfo.scale);
            
            writeNumber(textureInfo.textureIndex);
        }
    }
    /*
    {
        const auto entitiesLength = static_cast<BspArrayLength>(bspFile.entities.size());
        writeNumber(entitiesLength);
        
        for (const auto& entity : bspFile.entities)
        {
            writeString(entity.type);
            
            writeString(entity.name);
            
            writeVertex(entity.position);
            
            const auto propertiesLength = static_cast<bsp::SmallArrayLength>(entity.properties.size());
            writeNumber(propertiesLength);
            
            for (const auto& property : entity.properties)
            {
                writeString(property);
            }
        }
    }
    */
    {
        const auto planesLength = static_cast<bsp::ArrayLength>(bspFile.planes.size());
        writeNumber(planesLength);
        
        for (const auto& plane : bspFile.planes)
        {
            writeVertex(plane.normal);
            writeNumber(plane.distance);
        }
    }
    {
        const auto verticiesLength = static_cast<bsp::ArrayLength>(bspFile.vertices.size());
        writeNumber(verticiesLength);
        
        for (const auto& vertex : bspFile.vertices)
        {
            writeVertex(vertex);
        }
    }
    {
        const auto edgesLength = static_cast<bsp::ArrayLength>(bspFile.edges.size());
        writeNumber(edgesLength);
        
        for (const auto& edge : bspFile.edges)
        {
            writeNumber(edge.startVertex);
            writeNumber(edge.endVertex);
        }
    }
    {
        const auto facesLength = static_cast<bsp::ArrayLength>(bspFile.faces.size());
        writeNumber(facesLength);
        
        for (const auto& face : bspFile.faces)
        {
            writeNumber(face.firstEdge);
            writeNumber(face.numEdges);
            writeNumber(face.plane);
            writeNumber(face.textureInfoIndex);
        }
    }
    {
        const auto nodesLength = static_cast<bsp::ArrayLength>(bspFile.nodes.size());
        writeNumber(nodesLength);
        
        for (const auto& node : bspFile.nodes)
        {
            writeNumber(node.splitPlane);
            
            writeNumber(node.frontChild);
            writeNumber(node.backChild);
            
            writeNumber(node.firstFace);
            writeNumber(node.numFaces);
        }
    }
    {
        const auto leavesLength = static_cast<bsp::ArrayLength>(bspFile.leaves.size());
        writeNumber(leavesLength);
        
        for (const auto& leaf : bspFile.leaves)
        {
            writeNumber(leaf.content);
            
            writeNumber(leaf.firstFace);
            writeNumber(leaf.numFaces);
        }
    }
}
