#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class NetBuf;

struct Entity
{
    glm::vec3 position;
    glm::quat rotation;
    std::string modelName;
    
    static void serialize(const Entity& inEntity, NetBuf& outBuf);
    
    static bool deserialize(Entity& outEntity, NetBuf& inBuf);
};
