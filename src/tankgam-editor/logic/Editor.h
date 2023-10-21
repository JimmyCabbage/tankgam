#pragma once

#include <glad/gl.h>

#include "Viewport.h"

class Editor
{
public:
    Editor();
    ~Editor();
    
    Viewport& getViewport();
    
private:
    Viewport viewport;
};
