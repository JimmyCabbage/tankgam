#include "Thinker.h"

#include <stdexcept>

ThinkerManager::ThinkerManager(Engine& engine)
    : engine{ engine }
{
}

ThinkerManager::~ThinkerManager() = default;

void ThinkerManager::registerThinker(Thinker& thinker)
{
    if (std::find(thinkers.begin(), thinkers.end(), &thinker) != thinkers.end())
    {
        throw std::runtime_error{ "Tried to register same thinker twice!" };
    }
    
    thinkers.push_back(&thinker);
}

void ThinkerManager::unregisterThinker(Thinker& thinker)
{
    if (std::find(thinkers.begin(), thinkers.end(), &thinker) == thinkers.end())
    {
        throw std::runtime_error{ "Tried to unregister registered thinker twice!" };
    }
    
    std::erase(thinkers, &thinker);
}

void ThinkerManager::thinkAll()
{
    for (Thinker* thinker : thinkers)
    {
        thinker->think();
    }
}

Thinker::Thinker(Engine& engine, ThinkerManager& thinkerManager, Callback callback)
    : thinkerManager{ thinkerManager }, callback{ std::move(callback) }
{
    thinkerManager.registerThinker(*this);
}

Thinker::~Thinker()
{
    thinkerManager.unregisterThinker(*this);
}

void Thinker::think()
{
    callback();
}
