#include "util/Plane.h"

#include <glm/gtc/matrix_transform.hpp>

bool Plane::isValidPlane(const Plane& plane)
{
    if (glm::length(plane.normal) <= 0.9f)
    {
        return false;
    }
    
    return true;
}

Plane Plane::fromVertexAndNormal(glm::vec3 vertex, glm::vec3 normal)
{
    const Plane plane
    {
        .normal = normal,
        .distance = -glm::dot(normal, vertex)
    };
    
    return plane;
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
    const glm::vec3 v1 = -plane1.distance * glm::cross(plane2.normal, plane3.normal);
    const glm::vec3 v2 = -plane2.distance * glm::cross(plane3.normal, plane1.normal);
    const glm::vec3 v3 = -plane3.distance * glm::cross(plane1.normal, plane2.normal);
    const float denominator = glm::dot(plane1.normal, glm::cross(plane2.normal, plane3.normal));
    
    const glm::vec3 intersection = (v1 + v2 + v3) / denominator;
    
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

void Plane::translatePlane(Plane& plane, glm::vec3 direction)
{
    const glm::vec3 point = (plane.normal * -plane.distance) + direction;
    plane = fromVertexAndNormal(point, plane.normal);
}

void Plane::rotatePlane(Plane& plane, glm::vec3 rotation, glm::vec3 center)
{
    const glm::mat4 transMat = glm::translate(glm::mat4{ 1.0f }, center);
    const glm::mat4 revTransMat = glm::translate(glm::mat4{ 1.0f }, -center);
    
    glm::mat4 rotMat = glm::rotate(glm::mat4{ 1.0f }, rotation.x, glm::vec3{ 1.0f, 0.0f, 0.0f });
    rotMat = glm::rotate(rotMat, rotation.y, glm::vec3{ 0.0f, 1.0f, 0.0f });
    rotMat = glm::rotate(rotMat, rotation.z, glm::vec3{ 0.0f, 0.0f, 1.0f });
    
    const glm::mat4 mat = transMat * rotMat * revTransMat;
    
    const glm::vec3 rotatedPoint = mat * glm::vec4{ plane.normal * -plane.distance, 1.0f };
    const glm::vec3 rotatedNormal = glm::normalize(glm::vec3{ rotMat * glm::vec4{ plane.normal, 1.0f } });
    plane = fromVertexAndNormal(rotatedPoint, rotatedNormal);
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

Plane::Classification Plane::classifyPoints(const Plane& plane, std::span<const glm::vec3> polygonVertices)
{
    size_t backCount = 0;
    size_t frontCount = 0;
    
    for (const auto& vertex : polygonVertices)
    {
        switch (Plane::classifyPoint(plane, vertex))
        {
        case Classification::Front:
            frontCount++;
            break;
        case Classification::Back:
            backCount++;
            break;
        case Classification::Coincident:
            //do nothing
            break;
        }
    }
    
    if (backCount && frontCount) return Classification::Spanning;
    if (backCount) return Classification::Back;
    if (frontCount) return Classification::Front;
    return Classification::Coincident;
}
