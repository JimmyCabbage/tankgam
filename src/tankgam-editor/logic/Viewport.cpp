#include "Viewport.h"

Viewport::Viewport()
    : gl{ nullptr }
{
}

Viewport::~Viewport() = default;

void Viewport::initGL(GladGLContext& glf, int iwidth, int iheight)
{
    gl = &glf;
    width = iwidth;
    height = iheight;
    
    gl->Enable(GL_DEPTH_TEST);
    gl->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Viewport::removeGL()
{
    gl = nullptr;
}

void Viewport::render()
{
    if (!gl)
    {
        return;
    }
    
    gl->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Viewport::changeSize(int iwidth, int iheight)
{
    width = iwidth;
    height = iheight;
}
