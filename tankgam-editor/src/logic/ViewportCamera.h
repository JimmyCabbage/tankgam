#pragma once

#include <glm/glm.hpp>

class ViewportCamera
{
public:
    ViewportCamera(glm::vec3 position = {}, glm::vec3 front = { 0.0f, 0.0f, -1.0f }, glm::vec3 right = { 1.0f, 0.0f, 0.0f });
    ~ViewportCamera();
    
    enum class Direction
    {
        Forward,
        Backward,
        Left,
        Right,
        Up,
        Down
    };
    
    void move(Direction dir);
    
    void turn(Direction dir);
    
    glm::vec3 getPosition() const;
    
    glm::vec3 getFront() const;
    
    glm::vec3 getRight() const;
    
    glm::vec3 getUp() const;
    
    glm::mat4 getViewMatrix() const;
    
private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
};
