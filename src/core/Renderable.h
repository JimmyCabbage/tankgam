#pragma once

#include <memory>
#include <vector>
#include <string_view>

#include <glm/glm.hpp>

class Engine;
class Renderer;
class Model;
class Renderable;

class RenderableManager
{
public:
    RenderableManager(Engine& engine);
    ~RenderableManager();
    
    RenderableManager(const RenderableManager&) = delete;
    RenderableManager& operator=(const RenderableManager&) = delete;
    
    void registerRenderable(Renderable& renderable);
    
    void unregisterRenderable(Renderable& renderable);
    
    void drawAll();

private:
    Renderer& renderer;
    
    std::vector<Renderable*> renderables;
};

class Renderable
{
public:
    Renderable(Engine& engine, std::string_view modelFileName);
    ~Renderable();
    
    Renderable(const Renderable&) = delete;
    Renderable& operator=(const Renderable&) = delete;
    
    void setPosition(glm::vec3 newPosition);
    
    void draw();
    
private:
    Renderer& renderer;
    
    RenderableManager& renderableManager;
    
    glm::vec3 position;
    
    std::unique_ptr<Model> model;
};
