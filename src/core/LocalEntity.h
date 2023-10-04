#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct LocalEntity
{
    glm::vec3 position;
    glm::quat rotation;
    std::string modelName;
};
