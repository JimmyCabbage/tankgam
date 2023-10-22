#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

class Editor;

class Viewport
{
public:
    explicit Viewport(Editor& editor);
    ~Viewport();
    
    void initGL(GladGLContext& glf, int width, int height);
    void removeGL();
    
    void render();
    
    void changeSize(int width, int height);
    
private:
    Editor& editor;
    
    GladGLContext* gl;
    int width;
    int height;
    
    glm::vec3 begin;
};
