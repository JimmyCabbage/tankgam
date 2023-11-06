#pragma once

#include <vector>
#include <span>

#include <glad/gl.h>

#include <util/FileManager.h>

#include "StdLog.h"
#include "Viewport.h"
#include "Brush.h"

#include <util/Plane.h>

class Editor
{
public:
    Editor();
    ~Editor();
    
private:
    void defaultState();
    
public:
    FileManager& getFileManager();
    
    Viewport& getViewport();
    
    std::span<const std::string> getAvailableTextures() const;
    
    std::vector<Brush> getBrushes() const;
    
    std::vector<Brush> getSelectedBrushes() const;
    
    void createBrush(std::string_view textureName, glm::vec2 begin, glm::vec2 end, int skipAxis);
    
    void selectBrush(glm::vec3 selectOrigin, glm::vec3 selectDirection);
    
    void deleteSelectedBrushes();
    
    void moveSelectedBrushes(glm::vec3 moveDir);
    
private:
    StdLog stdLog;
    FileManager fileManager;
    Viewport viewport;
    
    std::vector<std::string> availableTextures;
    
    std::vector<Brush> brushes;
    std::vector<Brush> selectedBrushes;
    std::vector<size_t> selectedBrushesIndices;
    
    glm::vec3 beginVec;
    glm::vec3 endVec;
    glm::vec3 defaultBeginSize;
    glm::vec3 defaultEndSize;
};
