#pragma once

#include <array>
#include <optional>
#include <cstring>

#include "GlobalEntity.h"
#include "LocalEntity.h"

using EntityId = uint16_t;
using NetEntityId = uint16_t;

class EntityManager
{
public:
    EntityManager();
    ~EntityManager();
    
    EntityId allocateLocalEntity();
    
    void freeLocalEntity(EntityId id);
    
    NetEntityId nextAvailableGlobalId();
    
    EntityId allocateGlobalEntity(NetEntityId netId);
    
    void freeGlobalEntity(NetEntityId netId);
    
    NetEntityId convertIdNetToLocal(EntityId id) const;
    
    EntityId convertIdLocalToNet(NetEntityId netId) const;
    
private:
    //global entities mapped from range of [0, MAX_GLOBAL_ENTITIES)
    static constexpr size_t MAX_GLOBAL_ENTITIES = 256;
    
    //local entities mapped from a range of [MAX_GLOBAL_ENTITIES, MAX_LOCAL_ENTITIES)
    static constexpr size_t MAX_LOCAL_ENTITIES = MAX_GLOBAL_ENTITIES * 2;
    
    std::array<bool, MAX_LOCAL_ENTITIES> usedEntities = {};
    std::array<LocalEntity, MAX_LOCAL_ENTITIES> localEntities = {};
    std::array<GlobalEntity, MAX_GLOBAL_ENTITIES> globalEntities = {};
};
