#include "EntityManager.h"

#include <stdexcept>

EntityManager::EntityManager() = default;

EntityManager::~EntityManager() = default;

EntityId EntityManager::allocateLocalEntity()
{
    for (size_t i = MAX_GLOBAL_ENTITIES; i < MAX_LOCAL_ENTITIES; i++)
    {
        if (!usedEntities[i])
        {
            usedEntities[i] = true;
            localEntities[i] = {};
            
            return static_cast<EntityId>(i);
        }
    }
    
    throw std::runtime_error{ "Ran out of room for local entities!" };
}

void EntityManager::freeLocalEntity(EntityId id)
{
    const size_t realId = static_cast<size_t>(id);
    if (realId < MAX_GLOBAL_ENTITIES)
    {
        throw std::runtime_error{ "Tried to free a global entity as a local entity" };
    }
    
    if (!usedEntities[realId])
    {
        throw std::runtime_error{ "Tried to free unallocated local entity!" };
    }
    
    usedEntities[realId] = false;
}

NetEntityId EntityManager::nextAvailableGlobalId()
{
    for (size_t i = 0; i < MAX_GLOBAL_ENTITIES; i++)
    {
        if (!usedEntities[i])
        {
            usedEntities[i] = true;
            localEntities[i] = {};
            
            return static_cast<NetEntityId>(i);
        }
    }
    
    throw std::runtime_error{ "Ran out of room for local entities!" };
}

EntityId EntityManager::allocateGlobalEntity(NetEntityId netId)
{
    const size_t realId = static_cast<size_t>(netId);
    if (realId >= MAX_GLOBAL_ENTITIES)
    {
        throw std::runtime_error{ "Tried to allocate global entity outside of available id range!" };
    }
    
    if (usedEntities[realId])
    {
        throw std::runtime_error{ "Tried to allocate global entity in occupied slot!" };
    }
    
    usedEntities[realId] = true;
    localEntities[realId] = {};
    globalEntities[realId].local = &localEntities[realId];
    
    return static_cast<EntityId>(realId);
}

void EntityManager::freeGlobalEntity(NetEntityId netId)
{
    const size_t realId = static_cast<size_t>(netId);
    if (realId >= MAX_GLOBAL_ENTITIES)
    {
        throw std::runtime_error{ "Tried to free a local entity as a global entity" };
    }
    
    if (!usedEntities[realId])
    {
        throw std::runtime_error{ "Tried to free unallocated global entity!" };
    }
    
    usedEntities[realId] = false;
    globalEntities[realId].local = nullptr;
}

NetEntityId EntityManager::convertIdNetToLocal(EntityId id) const
{
    //TODO: add bounds checks
    return static_cast<NetEntityId>(id);
}

EntityId EntityManager::convertIdLocalToNet(NetEntityId netId) const
{
    //TODO: add bounds checks
    return static_cast<EntityId>(netId);
}
