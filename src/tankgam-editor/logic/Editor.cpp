#include "Editor.h"

#include <limits>

#include "Common.h"

Editor::Editor()
    : viewport{ *this }
{
    defaultState();
}

Editor::~Editor() = default;

void Editor::defaultState()
{
    brushes.clear();
    selectedBrushes.clear();
    
    beginVec = {};
    endVec = {};
    defaultBeginSize = { 0, -GRID_UNIT, 0 };
    defaultEndSize = { GRID_UNIT, 0, GRID_UNIT };
}

Viewport& Editor::getViewport()
{
    return viewport;
}

std::vector<Brush> Editor::getBrushes() const
{
    return brushes;
}

std::vector<Brush> Editor::getSelectedBrushes() const
{
    return selectedBrushes;
}

void Editor::createBrush(glm::vec2 begin, glm::vec2 end, int skipAxis)
{
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
    
    brushes.emplace_back("", beginVec, endVec);
    
    viewport.update();
}

void Editor::selectBrush(glm::vec3 selectOrigin, glm::vec3 selectDirection)
{
    selectedBrushes.clear();
    
    float closestDistance = std::numeric_limits<float>::max();
    const Brush* closestBrush = nullptr;
    
    for (const auto& brush : brushes)
    {
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
        }
    }
    
    if (closestBrush)
    {
        selectedBrushes.push_back(*closestBrush);
    }
    
    viewport.update();
}
