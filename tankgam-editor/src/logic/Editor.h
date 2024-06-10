#pragma once

#include <vector>
#include <span>
#include <filesystem>

#include <glad/gl.h>

#include <util/FileManager.h>
#include <util/Plane.h>
#include <util/Brush.h>

#include "StdLog.h"
#include "Viewport.h"

class Editor
{
public:
    Editor();
    ~Editor();
    
public:
    FileManager& getFileManager();
    
    Viewport& getViewport();
    
    void setMapName(std::filesystem::path newMapPath);
    
    std::filesystem::path getMapPath() const;
    
    std::span<const std::string> getAvailableTextures() const;
    
    std::vector<Brush> getBrushes() const;
    
    std::vector<Brush> getSelectedBrushes() const;
    
    std::vector<BrushFace> getSelectedFaces() const;
    
    void createBrush(std::string_view textureName, glm::vec2 begin, glm::vec2 end, int skipAxis);
    
    void selectBrush(glm::vec3 selectOrigin, glm::vec3 selectDirection, bool overridePrevSel = true);
    
    void selectFace(glm::vec3 selectOrigin, glm::vec3 selectDirection, bool overridePrevSel = true);
    
    void deleteSelectedBrushes();
    
    void moveSelected(glm::vec3 moveDir);
    
    void rotateSelected(glm::vec3 rotDir);
    
    void newMap();

    bool saveMap();
    
    void openMap(std::filesystem::path fileName);
    
    void buildMap();
    
private:
    StdLog stdLog;
    FileManager fileManager;
    Viewport viewport;
    
    std::filesystem::path mapPath;
    
    std::vector<std::string> availableTextures;
    std::vector<std::string> usedTextures;
    
    std::vector<Brush> brushes;
    std::vector<Brush> selectedBrushes;
    std::vector<size_t> selectedBrushesIndices;
    std::vector<std::pair<size_t, size_t>> selectedFaces;
    
    glm::vec3 beginVec;
    glm::vec3 endVec;
    glm::vec3 defaultBeginSize;
    glm::vec3 defaultEndSize;
};
