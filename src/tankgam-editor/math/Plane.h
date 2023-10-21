#pragma once

#include <span>
#include <optional>

#include <glm/glm.hpp>

//plane equation is
//normal.x(x) + normal.y(y) + normal.z(z) + distance = 0
struct Plane
{
    glm::vec3 normal;
    float distance;
    
    static bool isValidPlane(const Plane& plane);
    
    static Plane fromVertices(std::span<const glm::vec3> vertices);
    
    static std::optional<glm::vec3> intersectPlanes(const Plane& plane1, const Plane& plane2, const Plane& plane3);
    
    static std::optional<glm::vec3> intersectRay(const Plane& plane, glm::vec3 rayOrigin, glm::vec3 rayDirection);
};
