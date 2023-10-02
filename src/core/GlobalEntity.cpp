#include "GlobalEntity.h"

#include "NetBuf.h"

void GlobalEntity::serialize(const GlobalEntity& inEntity, NetBuf& outBuf)
{
    outBuf.writeVec3(inEntity.local.position);
    outBuf.writeQuat(inEntity.local.rotation);
    outBuf.writeString(inEntity.local.modelName);
}

bool GlobalEntity::deserialize(GlobalEntity& outEntity, NetBuf& inBuf)
{
    if (!inBuf.readVec3(outEntity.local.position))
    {
        return false;
    }
    
    if (!inBuf.readQuat(outEntity.local.rotation))
    {
        return false;
    }
    
    if (!inBuf.writeString(outEntity.local.modelName))
    {
        return false;
    }
    
    return true;
}
