#pragma once

#include "LocalEntity.h"

class NetBuf;

struct GlobalEntity
{
    LocalEntity* local;
    
    static void serialize(const GlobalEntity& inEntity, NetBuf& outBuf);
    
    static bool deserialize(GlobalEntity& outEntity, NetBuf& inBuf);
};
