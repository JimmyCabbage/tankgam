#include "Editor.h"

#include <limits>
#include <stdexcept>
#include <fstream>
#include <algorithm>

#include <fmt/format.h>

#include <util/Bsp.h>
#include <util/BspBuilder.h>

#include "Common.h"

Editor::Editor()
    : stdLog{}, fileManager{ stdLog }, viewport{ fileManager, *this }
{
    fileManager.loadAssetsFile("base_textures.assets");
    
    defaultState();
}

Editor::~Editor() = default;

void Editor::defaultState()
{
    mapName = "default";
    
    availableTextures = fileManager.fileNamesInDir("textures/");
    if (availableTextures.empty())
    {
        throw std::runtime_error{ "No available textures!" };
    }
    usedTextures.clear();
    
    brushes.clear();
    selectedBrushes.clear();
    selectedBrushesIndices.clear();
    
    beginVec = {};
    endVec = {};
    defaultBeginSize = { -GRID_UNIT, -GRID_UNIT, -GRID_UNIT };
    defaultEndSize = { GRID_UNIT, 0, GRID_UNIT };
    
    viewport.update();
}

FileManager& Editor::getFileManager()
{
    return fileManager;
}

Viewport& Editor::getViewport()
{
    return viewport;
}

void Editor::setMapName(std::string newMapName)
{
    mapName = std::move(newMapName);
}

std::string Editor::getMapName() const
{
    return mapName;
}

std::span<const std::string> Editor::getAvailableTextures() const
{
    return availableTextures;
}

std::vector<Brush> Editor::getBrushes() const
{
    return brushes;
}

std::vector<Brush> Editor::getSelectedBrushes() const
{
    return selectedBrushes;
}

void Editor::createBrush(std::string_view textureName, glm::vec2 begin, glm::vec2 end, int skipAxis)
{
    //make sure this brush actually has some volume
    if (std::abs(begin.x - end.x) < 0.1f ||
        std::abs(begin.y - end.y) < 0.1f)
    {
        return;
    }
    
    const int knownAxis1 = (skipAxis + 1) % 3;
    const int knownAxis2 = (skipAxis + 2) % 3;
    
    //fill out the missing axis with data that we had before
    beginVec[skipAxis] = defaultBeginSize[skipAxis];
    beginVec[knownAxis1] = begin[0];
    beginVec[knownAxis2] = begin[1];
    //update the default with new known data
    defaultBeginSize[knownAxis1] = begin[0];
    defaultBeginSize[knownAxis2] = begin[1];
    
    //fill out the missing axis with data that we had before
    endVec[skipAxis] = defaultEndSize[skipAxis];
    endVec[knownAxis1] = end[0];
    endVec[knownAxis2] = end[1];
    //update the default with new known data
    defaultEndSize[knownAxis1] = end[0];
    defaultEndSize[knownAxis2] = end[1];
    
    brushes.emplace_back(textureName.data(), GRID_UNIT, beginVec, endVec);
    if (std::find(usedTextures.begin(), usedTextures.end(), std::string{ textureName.data() }) == usedTextures.end())
    {
        usedTextures.emplace_back(textureName.data());
    }
    
    viewport.update();
}

void Editor::selectBrush(glm::vec3 selectOrigin, glm::vec3 selectDirection)
{
    selectedBrushes.clear();
    selectedBrushesIndices.clear();
    
    float closestDistance = std::numeric_limits<float>::max();
    const Brush* closestBrush = nullptr;
    size_t brushNum;
    
    for (size_t i = 0; i < brushes.size(); i++)
    {
        const auto& brush = brushes[i];
        
        const auto intersection = brush.getIntersection(selectOrigin, selectDirection);
        if (!intersection.has_value())
        {
            continue;
        }
        
        const float intersectionDistance = glm::distance(intersection.value(), selectOrigin);
        if (intersectionDistance < closestDistance)
        {
            closestDistance = intersectionDistance;
            closestBrush = &brush;
            brushNum = i;
        }
    }
    
    if (closestBrush)
    {
        selectedBrushes.push_back(*closestBrush);
        selectedBrushesIndices.push_back(brushNum);
    }
    
    viewport.update();
}

void Editor::deleteSelectedBrushes()
{
    for (size_t index : selectedBrushesIndices)
    {
        brushes.erase(brushes.begin() + index);
    }
    
    selectedBrushes.clear();
    selectedBrushesIndices.clear();
    
    viewport.update();
}

void Editor::moveSelectedBrushes(glm::vec3 moveDir)
{
    for (size_t i = 0; i < selectedBrushesIndices.size(); i++)
    {
        const size_t index = selectedBrushesIndices[i];
        brushes[index].translate(moveDir);
        selectedBrushes[i] = brushes[index];
    }
    
    viewport.update();
}

void Editor::saveMap()
{
    if (mapName.empty())
    {
        return;
    }
    
    std::ofstream file{ mapName + ".map" };
    
    if (!file.is_open())
    {
        return;
    }
    
    for (std::string_view texture : usedTextures)
    {
        file << "t " << texture.data() << '\n';
    }
    
    size_t numNormals = 0;
    size_t numPlanes = 0;
    for (const auto& brush : brushes)
    {
        const size_t startPlane = numPlanes;
        const auto planes = brush.getPlanes();
        for (const auto& plane : planes)
        {
            file << "n "
                 << plane.normal.x << ' '
                 << plane.normal.y << ' '
                 << plane.normal.z << '\n';
            const size_t normLoc = numNormals++;
            
            file << "p "
                 << normLoc << ' '
                 << plane.distance << '\n';
            numPlanes++;
        }
        
        const glm::vec3 c = brush.getColor();
        const size_t texLoc = std::distance(usedTextures.begin(), std::find(usedTextures.begin(), usedTextures.end(), brush.getTextureName()));
        file << "b "
             << brush.getTextureScale() << ' '
             << c.x << ' '
             << c.y << ' '
             << c.z << ' '
             << texLoc << ' '
             << numPlanes - startPlane << ' ';
        for (size_t i = startPlane; i < numPlanes; i++)
        {
            file << i << ' ';
        }
        file << '\n';
    }
}

void Editor::loadMap(std::string fileName)
{
    defaultState();
    
    //remove extension if present
    mapName = fileName.substr(0, fileName.find_last_of('.'));
    
    std::ifstream file{ fileName };
    
    if (!file.is_open())
    {
        throw std::runtime_error{ fmt::format("Failed to open file {}", fileName) };
    }
    
    std::vector<glm::vec3> normals;
    std::vector<Plane> planes;
    
    int lineNum = 0;
    std::string line;
    while (std::getline(file, line))
    {
        lineNum++;
        
        if (line.empty())
        {
            continue;
        }
        
        std::stringstream lineStream{ std::move(line) };
        
        char type;
        lineStream >> type;
        switch (type)
        {
        case 't':
        {
            std::string texture;
            lineStream >> texture;
            
            usedTextures.push_back(std::move(texture));
        }
            break;
        case 'n':
        {
            glm::vec3 n;
            lineStream >> n.x >> n.y >> n.z;
            
            normals.push_back(n);
        }
            break;
        case 'p':
        {
            Plane plane{};
            
            size_t normLoc;
            lineStream >> normLoc >> plane.distance;
            
            plane.normal = normals[normLoc];
            
            planes.push_back(plane);
        }
            break;
        case 'b':
        {
            float textureScale;
            glm::vec3 c;
            size_t texLoc;
            size_t numPlanes;
            
            lineStream >> textureScale
                >> c.x >> c.y >> c.z
                >> texLoc
                >> numPlanes;
            
            const std::string textureName = usedTextures[texLoc];
            std::vector<Plane> brushPlanes;
            brushPlanes.reserve(numPlanes);
            
            for (size_t i = 0; i < numPlanes; i++)
            {
                size_t planeLoc;
                lineStream >> planeLoc;
                
                brushPlanes.push_back(planes[planeLoc]);
            }
            
            brushes.emplace_back(textureName, textureScale, brushPlanes, c);
        }
            break;
        default:
            stdLog.logf("Unknown option %c in map file (line: %d), skipping..", type, lineNum);
        }
    }
    
    viewport.update();
}

void Editor::buildMap()
{
    BspBuilder bspBuilder;
    bspBuilder.addBrushes(brushes);
    
    bsp::File file = bspBuilder.build();
    bsp::writeFile(fmt::format("{}.tgmap", mapName), file);
}
