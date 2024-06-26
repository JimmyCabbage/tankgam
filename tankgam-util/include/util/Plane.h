#pragma once

#include <span>
#include <optional>

#include <glm/glm.hpp>

// plane equation is
// normal.x(x) + normal.y(y) + normal.z(z) + distance = 0
struct Plane
{
    glm::vec3 normal;
    float distance;
    
    static bool isValidPlane(const Plane& plane);
    
    static Plane fromVertexAndNormal(glm::vec3 vertex, glm::vec3 normal);
    
    static Plane fromVertices(std::span<const glm::vec3> vertices);
    
    static Plane fromVertices(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
    
    static std::optional<glm::vec3> intersectPlanes(const Plane& plane1, const Plane& plane2, const Plane& plane3);
    
    static std::optional<glm::vec3> intersectRay(const Plane& plane, glm::vec3 rayOrigin, glm::vec3 rayDirection);
    
    static void translatePlane(Plane& plane, glm::vec3 direction);
    
    //rotation is in radians
    static void rotatePlane(Plane& plane, glm::vec3 rotation, glm::vec3 center = {});
    
    enum class Classification
    {
        Coincident,
        Back,
        Front,
        Spanning
    };
    
    static Classification classifyPoint(const Plane& plane, glm::vec3 point);
    
    static Classification classifyPoints(const Plane& plane, std::span<const glm::vec3> polygonVertices);
};
