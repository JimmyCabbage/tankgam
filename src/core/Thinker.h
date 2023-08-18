#pragma once

#include <vector>
#include <functional>

class Engine;
class Thinker;

class ThinkerManager
{
public:
    ThinkerManager(Engine& engine);
    ~ThinkerManager();
    
    ThinkerManager(const ThinkerManager&) = delete;
    ThinkerManager& operator=(const ThinkerManager&) = delete;
    
    void registerThinker(Thinker& thinker);
    
    void unregisterThinker(Thinker& thinker);
    
    void thinkAll();

private:
    Engine& engine;
    
    std::vector<Thinker*> thinkers;
};

class Thinker
{
public:
    using Callback = std::function<void()>;
    
    Thinker(Engine& engine, ThinkerManager& thinkerManager, Callback callback);
    ~Thinker();
    
    Thinker(const Thinker&) = delete;
    Thinker& operator=(const Thinker&) = delete;
    
    void think();
    
private:
    ThinkerManager& thinkerManager;
    
    Callback callback;
};
