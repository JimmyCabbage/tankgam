#pragma once

#include <memory>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <gl/Shader.h>
#include <gl/Mesh.h>

#include "ViewportCamera.h"

class Editor;

class Viewport
{
public:
    explicit Viewport(Editor& editor);
    ~Viewport();
    
    void initGL(GladGLContext& glf, int width, int height);
    
private:
    void createShaders();
    
public:
    void removeGL();
    
    void render();
    
    void changeSize(int width, int height);
    
private:
    Editor& editor;
    
    GladGLContext* gl;
    int width;
    int height;
    
    std::unique_ptr<Shader> defaultShader;
    std::unique_ptr<Shader> brushShader;
    std::unique_ptr<Shader> noProjShader;
    
    std::unique_ptr<Mesh> borderMesh;
    
    enum class ViewportType
    {
        Top,
        Front,
        Side,
        Projection
    };
    
    struct ViewportData
    {
        ViewportType type;
        glm::ivec2 offset;
        float zoom;
        
        glm::mat4 inverseProjViewMatrix;
        ViewportCamera camera;
        
        glm::ivec2 lastClick;
    };
};
