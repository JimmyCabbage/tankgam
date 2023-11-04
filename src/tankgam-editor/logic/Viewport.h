#pragma once

#include <memory>
#include <vector>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <gl/Shader.h>
#include <gl/Mesh.h>

#include "ViewportCamera.h"
#include "ViewportToolType.h"

class Editor;

class Viewport
{
public:
    explicit Viewport(Editor& editor);
    ~Viewport();
    
    //when map state is changed
    void update();
    
    void initGL(GladGLContext& glf, int width, int height);
    
    void quitGL();

public:
    enum class ViewportType
    {
        Top,
        Front,
        Side,
        Projection
    };
    
    void setToolType(ViewportToolType viewportToolType);
    
    ViewportToolType getToolType() const;
    
private:
    void createShaders();
    
    void createViewport(ViewportType type, glm::ivec2 offset);
    
public:
    struct ViewportData
    {
        ViewportType type;
        glm::ivec2 offset;
        float zoom;
        
        glm::mat4 inverseProjViewMatrix;
        ViewportCamera camera;
        
        glm::ivec2 lastClick;
        
        std::unique_ptr<Mesh> gridMesh;
        std::unique_ptr<Mesh> coordinateMesh;
    };
    
    enum class MoveDir
    {
        Forward,
        Back,
        Left,
        Right,
        Up,
        Down
    };
    
    void moveCamera(MoveDir moveDir);
    
    enum class TurnDir
    {
        Left,
        Right
    };
    
    void turnCamera(TurnDir turnDir);
    
    void zoomInCamera();
    
    void zoomOutCamera();
    
private:
    void zoomCamera(float amount);
    
public:
    void clickLeftStart(int x, int y);
    
    void clickLeftEnd(int x, int y);
    
private:
    ViewportData& chooseViewportMouse(glm::ivec2 omouse);
    
public:
    void render();
    
    void changeSize(int width, int height);
    
private:
    Editor& editor;
    
    GladGLContext* gl;
    float maxLineWidth;
    int width;
    int height;
    int viewportWidth;
    int viewportHeight;
    
    ViewportToolType toolType;
    
    std::unique_ptr<Shader> defaultShader;
    std::unique_ptr<Shader> brushShader;
    std::unique_ptr<Shader> noProjShader;
    
    std::unique_ptr<Mesh> borderMesh;
    
    std::vector<ViewportData> viewportDatas;
    ViewportData* currentViewport;
    
    std::vector<Mesh> brushMeshes;
};
