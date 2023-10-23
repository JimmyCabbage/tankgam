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
    
    std::span<const Brush> getBrushes() const;
    
    void createBrush(glm::vec2 begin, glm::vec2 end, int skipAxis);
    
private:
    Viewport viewport;
    
    std::vector<Brush> brushes;
    
    glm::vec3 beginVec;
    glm::vec3 endVec;
    glm::vec3 defaultBeginSize;
    glm::vec3 defaultEndSize;
};
