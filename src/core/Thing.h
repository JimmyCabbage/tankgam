#pragma once

#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

#include "Physics.h"

enum class ThingType : uint16_t
{
    Player = 0,
    Enemy = 1,
};

enum ThingFlags : uint16_t
{
    THING_FLAG_SOLID = 1 << 0,
    THING_FLAG_FLY = 1 << 1,
};

inline ThingFlags operator|(ThingFlags a, ThingFlags b)
{
    return static_cast<ThingFlags>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

class Engine;

class Thing
{
public:
    Thing(Engine& engine, ThingType type, ThingFlags flags, phys::Box box, float pitch = 0.0f, float yaw = -90.0f);
    ~Thing();
    
    ThingType getType() const;
    
    ThingFlags getFlags() const;
    
    void setFlags(ThingFlags newFlags);
    
    float getPitch() const;
    
    void setPitch(float newPitch);
    
    float getYaw() const;
    
    void setYaw(float newYaw);
    
    void move(glm::vec2 accelerate);
    
private:
    Engine& engine;
    
    ThingType type;
    ThingFlags flags;
    
    phys::Box box;
    float pitch;
    float yaw;
};
