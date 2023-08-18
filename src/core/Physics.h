#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace phys
{
    struct Segment
    {
        glm::vec2 begin;
        glm::vec2 end;
        glm::vec2 normal;
    };
    
    struct Box
    {
        glm::vec2 center;
        float width;
        float height;
        glm::vec2 velocity;
    };
    
    //return is where the collision happened
    //1.0 is made it to the destination
    //0.0 is it didn't
    float collision(Box movingBox, Box staticBox, glm::vec2& outNormal);
    
    //return is where the collision happened
    //1.0 is made it to the destination
    //0.0 is it didn't
    float collision(Box movingBox, Segment staticSegment, glm::vec2& outNormal);
}
