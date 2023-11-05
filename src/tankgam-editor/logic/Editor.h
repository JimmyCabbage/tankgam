#pragma once

#include <vector>
#include <span>

#include <glad/gl.h>

#include "Viewport.h"
#include "Brush.h"
#include "Plane.h"

class Editor
{
public:
    Editor();
    ~Editor();
    
private:
    void defaultState();
    
public:
    Viewport& getViewport();
    
    std::vector<Brush> getBrushes() const;
    
    std::vector<Brush> getSelectedBrushes() const;
    
    void createBrush(glm::vec2 begin, glm::vec2 end, int skipAxis);
    
    void selectBrush(glm::vec3 selectOrigin, glm::vec3 selectDirection);
    
    void deleteSelectedBrushes();
    
    void moveSelectedBrushes(glm::vec3 moveDir);
    
private:
    Viewport viewport;
    
    std::vector<Brush> brushes;
    std::vector<Brush> selectedBrushes;
    std::vector<size_t> selectedBrushesIndices;
    
    glm::vec3 beginVec;
    glm::vec3 endVec;
    glm::vec3 defaultBeginSize;
    glm::vec3 defaultEndSize;
};
