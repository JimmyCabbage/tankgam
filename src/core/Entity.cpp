#include "Entity.h"

#include "NetBuf.h"

void Entity::serialize(const Entity& inEntity, NetBuf& outBuf)
{
    outBuf.writeVec3(inEntity.position);
    outBuf.writeQuat(inEntity.rotation);
    outBuf.writeString(inEntity.modelName);
}

bool Entity::deserialize(Entity& outEntity, NetBuf& inBuf)
{
    if (!inBuf.readVec3(outEntity.position))
    {
        return false;
    }
    
    if (!inBuf.readQuat(outEntity.rotation))
    {
        return false;
    }
    
    if (!inBuf.readString(outEntity.modelName))
    {
        return false;
    }
    
    return true;
}
