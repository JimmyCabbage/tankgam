#include "Viewport.h"

Viewport::Viewport(Editor& editor)
    : editor{ editor }, gl{ nullptr }
{
}

Viewport::~Viewport() = default;

void Viewport::initGL(GladGLContext& glf, int width, int height)
{
    gl = &glf;
    this->width = width;
    this->height = height;
    
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

void Viewport::changeSize(int width, int height)
{
    this->width = width;
    this->height = height;
}
