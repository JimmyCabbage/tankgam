#pragma once

#include <vector>

#include "Physics.h"

class Engine;

class World
{
public:
	explicit World(Engine& /*engine*/);
	~World();

	World(const World&) = delete;
	World& operator=(const World&) = delete;

    std::vector<phys::Segment> getSegments() const;
    
private:
	//Engine& engine;
};
