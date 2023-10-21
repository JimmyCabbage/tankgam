#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/hash.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 texCoord;
    
    bool operator==(const Vertex& o) const
    {
        return position == o.position &&
               color == o.color &&
               normal == o.normal &&
               texCoord == o.texCoord;
    }
};

namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(const Vertex& v) const noexcept
        {
            const size_t ph = hash<glm::vec3>{}(v.position);
            const size_t ch = hash<glm::vec3>{}(v.color);
            const size_t nh = hash<glm::vec3>{}(v.normal);
            const size_t th = hash<glm::vec2>{}(v.texCoord);
            
            return ph ^ ((ch << 1) ^ ((th << 2) ^ ((nh << 3) ^ (th << 4))));
        }
    };
}
