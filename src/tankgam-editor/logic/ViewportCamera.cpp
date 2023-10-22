#include "ViewportCamera.h"

#include <stdexcept>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

ViewportCamera::ViewportCamera(glm::vec3 position, glm::vec3 front, glm::vec3 right)
    : position{ position }, front{ front }, right{ right }, up{ glm::cross(right, front) }
{
}

void ViewportCamera::move(Direction dir)
{
    constexpr float vel = 128.0f;
    switch (dir)
    {
    case Direction::Forward:
        position += front * vel;
        break;
    case Direction::Backward:
        position -= front * vel;
        break;
    case Direction::Left:
        position -= right * vel;
        break;
    case Direction::Right:
        position += right * vel;
        break;
    case Direction::Up:
        position += up * vel;
        break;
    case Direction::Down:
        position -= up * vel;
        break;
    default:
        throw std::runtime_error("Attempted to move in an impossible direction");
    }
}

void ViewportCamera::turn(Direction dir)
{
    //rotation in rads
    constexpr float rad = glm::radians(30.0f);
    
    glm::quat rot{ 1.0f, 0.0f, 0.0f, 0.0f };
    switch (dir)
    {
    case Direction::Left:
        rot = { std::cos(-rad / 2.0f), 0.0f, std::sin(-rad / 2.0f), 0.0f };
        break;
    case Direction::Right:
        rot = { std::cos(rad / 2.0f), 0.0f, std::sin(rad / 2.0f), 0.0f };
        break;
    default:
        throw std::runtime_error("Attempted to turn in an impossible direction");
    }
    
    front = glm::vec4{ front, 1.0f } * glm::mat4_cast(rot);
    right = glm::vec4{ right, 1.0f } * glm::mat4_cast(rot);
    up = glm::vec4{ up, 1.0f } * glm::mat4_cast(rot);
}

glm::vec3 ViewportCamera::getPosition() const
{
    return position;
}

glm::mat4 ViewportCamera::getViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}
