#include "Thing.h"

#include <stdexcept>

#include "sys/Renderer.h"
#include "Engine.h"
#include "World.h"
#include "Physics.h"

Thing::Thing(Engine& engine, ThingType type, ThingFlags flags, phys::Box box, float pitch, float yaw)
    : engine{ engine },
      type{ type }, flags{ flags },
      box{ box },
      pitch{ pitch }, yaw{ yaw }
{
}

Thing::~Thing() = default;

ThingType Thing::getType() const
{
    return type;
}

ThingFlags Thing::getFlags() const
{
    return flags;
}

void Thing::setFlags(ThingFlags newFlags)
{
    flags = newFlags;
}

float Thing::getPitch() const
{
    return pitch;
}

void Thing::setPitch(float newPitch)
{
    pitch = newPitch;
}

float Thing::getYaw() const
{
    return yaw;
}

void Thing::setYaw(float newYaw)
{
    yaw = newYaw;
}

void Thing::move(glm::vec2 accelerate)
{
    //apply friction
    {
        const float speed = std::sqrt(glm::dot(box.velocity, box.velocity));
        
        /*if (speed < SOME_LIMIT)
        {
            data.box.velocity = {};
            break;
        }*/
        
        constexpr float friction = 2.0f;
        
        float newSpeed = speed - friction;
        if (newSpeed < 0.0f)
        {
            newSpeed = 0.0f;
        }
        newSpeed /= speed;
        
        box.velocity *= newSpeed;
    }
    
    //accelerate the box!!!
    const float accelSpeed = std::sqrt(glm::dot(accelerate, accelerate));
    
    if (accelSpeed > 1.0f)
    {
        const glm::vec2 accelUnit = glm::normalize(accelerate);
        
        const float currSpeed = glm::dot(box.velocity, accelUnit);
        const float addSpeed = accelSpeed - currSpeed;
        if (addSpeed <= 0.01f)
        {
            return;
        }
        
        const float speed = std::min(accelSpeed, addSpeed);
        box.velocity += accelUnit * speed;
    }
    
    if (flags & THING_FLAG_SOLID)
    {
        size_t counter = 0; //no infinite loop
        while (counter++ < 500)
        {
            bool hit = false;
            for (const auto& segment : engine.getWorld().getSegments())
            {
                glm::vec2 normal{};
                const float time = phys::collision(box, segment, normal);
                
                constexpr float moveEpsilon = 0.9f;
                if (time > moveEpsilon)
                {
                    continue;
                }
                
                //sliiiide
                const float dot = (box.velocity.x * normal.y + box.velocity.y * normal.x) * time;
                box.velocity = glm::vec2{dot * normal.y, dot * normal.x};
                
                hit = true;
                break;
            }
            
            if (!hit)
            {
                break;
            }
        }
    }
    
    box.center += box.velocity;
}
