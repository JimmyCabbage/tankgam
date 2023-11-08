#include "Editor.h"

#include <limits>
#include <stdexcept>

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
    availableTextures = fileManager.fileNamesInDir("textures/");
    if (availableTextures.empty())
    {
        throw std::runtime_error{ "No available textures!" };
    }
    
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

void Editor::buildMap()
{
    BspBuilder bspBuilder;
    bspBuilder.addBrushes(brushes);
    
    bsp::File file = bspBuilder.build();
    bsp::writeFile("test.tgmap", file);
}
