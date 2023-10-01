#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <span>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glad/gl.h>
#include "SDL.h"

#include "Event.h"

enum class ShaderType
{
    Model,
    Text
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct Model
{
private:
    friend class Renderer; //no touchy touchy outside of the renderer
    
    size_t meshIndex;
    size_t textureIndex;
    ShaderType shaderType;
};

class Console;
class FileManager;
class Mesh;
class Texture;
class Shader;
class TextRenderer;

class Renderer
{
public:
    Renderer(Console& console, FileManager& fileManager, std::string_view windowName);
    ~Renderer();
    
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    
    bool consumeEvent(const Event& ev);
    
    int getWidth() const;
    
    int getHeight() const;
    
    void beginDraw();
    
    void endDraw();
    
    void loadMesh(std::string_view meshName, std::span<const Vertex> vertices);
    
    void loadMesh(std::string_view meshName, std::string_view meshFileName);
    
    void loadTexture(std::string_view textureName, std::string_view textureFileName);
    
    std::unique_ptr<Model> createModel(std::string_view meshName, std::string_view textureName, ShaderType shaderType);
    
    std::unique_ptr<Model> createModel(std::string_view modelFileName);
    
    void drawModel(Model& model, glm::vec3 scale, glm::mat4 rotation, glm::vec3 translate);
    
    void drawText(std::string_view text, glm::vec2 position, float size);
    
private:
    Console& console;
    
    FileManager& fileManager;
    
    int width;
    int height;
    
    SDL_Window* window;
    SDL_GLContext context;
    
    GladGLContext gl;
    
    glm::mat4 projMat;
    glm::mat4 orthoProjMat;
    
    std::unique_ptr<Shader> textShader;
    std::unique_ptr<Shader> modelShader;
    
    void setGLSettings() const;
    
    void createGLShaders();
    
    std::vector<Mesh> meshes;
    std::vector<std::string> meshNames;
    
    std::vector<Texture> textures;
    std::vector<std::string> textureNames;
    
    std::unordered_map<std::string, Model> loadedModels;
    
    size_t findMesh(std::string_view meshName);
    
    size_t findTexture(std::string_view textureName);
    
    std::unique_ptr<TextRenderer> textRenderer;
};
