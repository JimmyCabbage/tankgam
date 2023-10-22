#include "Plane.h"

bool Plane::isValidPlane(const Plane& plane)
{
    if (glm::length(plane.normal) <= 0.9f)
    {
        return false;
    }
    
    return true;
}

Plane Plane::fromVertices(std::span<const glm::vec3> vertices)
{
    return Plane::fromVertices(vertices[0], vertices[1], vertices[2]);
}

Plane Plane::fromVertices(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
    const glm::vec3 A = v2 - v1;
    const glm::vec3 B = v3 - v2;
    
    const glm::vec3 normal = glm::normalize(glm::cross(A, B));
    
    const Plane plane
    {
        .normal = normal,
        .distance = -glm::dot(normal, v1)
    };
    
    return plane;
}

std::optional<glm::vec3> Plane::intersectPlanes(const Plane& plane1, const Plane& plane2, const Plane& plane3)
{
    constexpr auto checkParallel = [](const glm::vec3& normal1, const glm::vec3& normal2)
    {
        return std::abs(glm::dot(normal1, normal2)) >= 0.99f;
    };
    
    //make sure all these are valid
    if (!Plane::isValidPlane(plane1) ||
        !Plane::isValidPlane(plane2) ||
        !Plane::isValidPlane(plane3))
    {
        return std::nullopt;
    }
    
    //check if normals are parallel
    if (checkParallel(plane1.normal, plane2.normal) ||
        checkParallel(plane1.normal, plane3.normal) ||
        checkParallel(plane2.normal, plane3.normal))
    {
        return std::nullopt;
    }
    
    //magic from a math webpage i found https://songho.ca/math/plane/plane.html#intersect_3planes
    //i originally did it algebraically since i havent had a math education
    const glm::mat3 normalMatrix
    {
        plane1.normal,
        plane2.normal,
        plane3.normal
    };
    
    const glm::mat3 inverseNormalMatrix = glm::inverse(normalMatrix);
    
    const glm::vec3 distanceVec
    {
        -plane1.distance,
        -plane2.distance,
        -plane3.distance,
    };
    
    const glm::vec3 intersection = inverseNormalMatrix * distanceVec;
    return intersection;
}

std::optional<glm::vec3> Plane::intersectRay(const Plane& plane, glm::vec3 rayOrigin, glm::vec3 rayDirection)
{
    const float denom = glm::dot(plane.normal, rayDirection);
    if (denom == 0.0f)
    {
        return std::nullopt;
    }
    
    const float t = -(glm::dot(plane.normal, rayOrigin) + plane.distance) / denom;
    return rayOrigin + (t * rayDirection);
}

Plane::Classification Plane::classifyPoint(const Plane& plane, glm::vec3 point)
{
    constexpr float planeThickness = 0.01f;

#if 0
    const float distance = glm::dot(plane.normal, point);

    if (distance > plane.distance + planeThickness) return Classification::Front;
    if (distance < plane.distance - planeThickness) return Classification::Back;
    return Classification::Coincident;
#else
    const float distance = glm::dot(plane.normal, point) + plane.distance;
    if (distance >  planeThickness) return Classification::Front;
    if (distance < -planeThickness) return Classification::Back;
    return Classification::Coincident;
#endif
}
