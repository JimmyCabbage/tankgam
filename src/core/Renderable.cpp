#include "Renderable.h"

#include <stdexcept>
#include <algorithm>

#include "Engine.h"
#include "sys/Renderer.h"

RenderableManager::RenderableManager(Engine& engine)
    : renderer{ engine.getRenderer() }
{
}

RenderableManager::~RenderableManager() = default;

void RenderableManager::registerRenderable(Renderable& renderable)
{
    if (std::find(renderables.begin(), renderables.end(), &renderable) != renderables.end())
    {
        throw std::runtime_error{ "Tried to register same renderable twice!" };
    }
    
    renderables.push_back(&renderable);
}

void RenderableManager::unregisterRenderable(Renderable& renderable)
{
    if (std::find(renderables.begin(), renderables.end(), &renderable) == renderables.end())
    {
        throw std::runtime_error{ "Tried to unregister registered renderable twice!" };
    }
    
    std::erase(renderables, &renderable);
}

void RenderableManager::drawAll()
{
    for (Renderable* renderable : renderables)
    {
        renderable->draw();
    }
}

Renderable::Renderable(Engine& engine, std::string_view modelFileName)
    : renderer{ engine.getRenderer() }, renderableManager{ engine.getRenderableManager() }
{
    renderableManager.registerRenderable(*this);
    
    model = renderer.createModel(modelFileName);
}

Renderable::~Renderable()
{
    renderableManager.unregisterRenderable(*this);
}

void Renderable::setPosition(glm::vec3 newPosition)
{
    position = newPosition;
}

void Renderable::draw()
{
    renderer.drawModel(*model, glm::vec3{ 1.0f }, glm::identity<glm::quat>(), position);
}
