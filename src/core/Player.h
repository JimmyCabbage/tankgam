#pragma once

#include "Thing.h"
#include "Renderable.h"

class Engine;

class Player
{
public:
    Player(Engine& engine);
    ~Player();

private:
    Engine& engine;
    
    Thing thing;
    
    Renderable tankBody;
    Renderable tankTurret;
};
