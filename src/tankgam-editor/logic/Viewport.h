#pragma once

#include <glad/gl.h>

class Viewport
{
public:
    Viewport();
    ~Viewport();
    
    void initGL(GladGLContext& glf, int iwidth, int iheight);
    void removeGL();
    
    void render();
    
    void changeSize(int iwidth, int iheight);
    
private:
    GladGLContext* gl;
    int width;
    int height;
};
