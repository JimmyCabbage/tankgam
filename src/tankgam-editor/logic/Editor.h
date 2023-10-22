#pragma once

#include <vector>

#include <glad/gl.h>

#include "Viewport.h"
#include "Brush.h"
#include "Plane.h"

class Editor
{
public:
    Editor();
    ~Editor();
    
    Viewport& getViewport();
    
private:
    Viewport viewport;
    
    std::vector<Brush> brushes;
};
