#include "EntityManager.h"

#include <stdexcept>

EntityManager::EntityManager() = default;

EntityManager::~EntityManager() = default;

EntityManager::EntityManager(const EntityManager& o)
{
    std::copy(o.usedEntities.begin(), o.usedEntities.end(), usedEntities.begin());
    std::copy(o.entities.begin(), o.entities.end(), entities.begin());
}

EntityManager& EntityManager::operator=(const EntityManager& o)
{
    if (&o == this)
    {
        return *this;
    }
    
    std::copy(o.usedEntities.begin(), o.usedEntities.end(), usedEntities.begin());
    std::copy(o.entities.begin(), o.entities.end(), entities.begin());
    
    return *this;
}

EntityManager::EntityManager(EntityManager&& o) noexcept
{
    std::swap(usedEntities, o.usedEntities);
    std::swap(entities, o.entities);
}

EntityManager& EntityManager::operator=(EntityManager&& o) noexcept
{
    if (&o == this)
    {
        return *this;
    }
    
    std::swap(usedEntities, o.usedEntities);
    std::swap(entities, o.entities);
    
    return *this;
}

bool EntityManager::operator==(const EntityManager& o) const
{
    if (&o == this)
    {
        return true;
    }
    
    for (size_t i = 0; i < MAX_LOCAL_ENTITIES; i++)
    {
        if (usedEntities[i] != o.usedEntities[i])
        {
            return false;
        }
    }
    
    for (size_t i = 0; i < MAX_LOCAL_ENTITIES; i++)
    {
        const Entity& entity = entities[i];
        const Entity& oEntity = o.entities[i];
        
        if ((glm::length(entity.position) - glm::length(oEntity.position)) >= 0.001f)
        {
            return false;
        }
        
        if (const float matching = glm::dot(entity.rotation, oEntity.rotation);
            std::abs(matching - 1.0f) >= 0.001f)
        {
            return false;
        }
        
        if (entity.modelName != oEntity.modelName)
        {
            return false;
        }
    }
    
    return true;
}

EntityId EntityManager::allocateLocalEntity()
{
    for (size_t i = MAX_GLOBAL_ENTITIES; i < MAX_LOCAL_ENTITIES; i++)
    {
        if (!usedEntities[i])
        {
            usedEntities[i] = true;
            entities[i] = {};
            
            return static_cast<EntityId>(i);
        }
    }
    
    throw std::runtime_error{ "Ran out of room for local entities!" };
}

void EntityManager::freeLocalEntity(EntityId id)
{
    if (!isLocalId(id))
    {
        throw std::runtime_error{ "Tried to free a not local id id!" };
    }
    
    const size_t realId = static_cast<size_t>(id);
    
    usedEntities[realId] = false;
}

void EntityManager::allocateGlobalEntity(EntityId netId)
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
    entities[realId] = {};
}

EntityId EntityManager::allocateGlobalEntity()
{
    EntityId netId;
    
    bool emptySpot = false;
    for (size_t i = 0; i < MAX_GLOBAL_ENTITIES; i++)
    {
        if (!usedEntities[i])
        {
            netId = static_cast<EntityId>(i);
            emptySpot = true;
            
            break;
        }
    }
    
    if (!emptySpot)
    {
        throw std::runtime_error{ "Ran out of global ids!" };
    }
    
    allocateGlobalEntity(netId);
    
    return netId;
}

void EntityManager::freeGlobalEntity(EntityId netId)
{
    if (!isGlobalId(netId))
    {
        throw std::runtime_error{ "Tried to free a not global id id!" };
    }
    
    const size_t realId = static_cast<size_t>(netId);
    
    usedEntities[realId] = false;
}

bool EntityManager::doesEntityExist(EntityId entityId) const
{
    return usedEntities[entityId];
}

bool EntityManager::isGlobalId(EntityId entityId) const
{
    if (entityId > MAX_GLOBAL_ENTITIES)
    {
        return false;
    }

#if 0
    if (!usedEntities[entityId])
    {
        return false;
    }
#endif
    
    return true;
}

bool EntityManager::isLocalId(EntityId entityId) const
{
    if (entityId < MAX_GLOBAL_ENTITIES || entityId > MAX_LOCAL_ENTITIES)
    {
        return false;
    }

#if 0
    if (!usedEntities[entityId])
    {
        return false;
    }
#endif
    
    return true;
}

std::vector<EntityId> EntityManager::getGlobalEntities() const
{
    std::vector<EntityId> entities;
    for (size_t i = 0; i < MAX_GLOBAL_ENTITIES; i++)
    {
        if (!usedEntities[i])
        {
            continue;
        }
        
        entities.push_back(static_cast<EntityId>(i));
    }
    
    return entities;
}

std::vector<EntityId> EntityManager::getLocalEntities() const
{
    std::vector<EntityId> entities;
    for (size_t i = MAX_GLOBAL_ENTITIES; i < MAX_LOCAL_ENTITIES; i++)
    {
        if (!usedEntities[i])
        {
            continue;
        }
        
        entities.push_back(static_cast<EntityId>(i));
    }
    
    return entities;
}

Entity* EntityManager::getGlobalEntity(EntityId id)
{
    if (!isGlobalId(id))
    {
        return nullptr;
    }
    
    const size_t realId = static_cast<size_t>(id);
    if (!usedEntities[realId])
    {
        return nullptr;
    }
    
    return &entities[realId];
}

Entity* EntityManager::getLocalEntity(EntityId id)
{
    if (!isLocalId(id))
    {
        return nullptr;
    }
    
    const size_t realId = static_cast<size_t>(id);
    if (!usedEntities[realId])
    {
        return nullptr;
    }
    
    return &entities[realId];
}
