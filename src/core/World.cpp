#include "World.h"

World::World(Engine& /*engine*/)
/*: engine{ engine }*/
{
}

World::~World() = default;

std::vector<phys::Segment> World::getSegments() const
{
    return {};
}
