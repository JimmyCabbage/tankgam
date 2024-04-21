#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#include "Entity.h"

using EntityId = uint16_t;

class EntityManager
{
public:
    EntityManager();
    ~EntityManager();

#ifdef ENTITY_MANAGER_COPY_CONSTRUCTOR
    EntityManager(const EntityManager& o);
    EntityManager& operator=(const EntityManager& o);
#endif
    
    EntityManager(EntityManager&& o) noexcept;
    EntityManager& operator=(EntityManager&& o) noexcept;
    
    bool operator==(const EntityManager& o) const;
    
    EntityId allocateLocalEntity();
    
    void freeLocalEntity(EntityId id);
    
    void allocateGlobalEntity(EntityId netId);
    
    EntityId allocateGlobalEntity();
    
    void freeGlobalEntity(EntityId netId);
    
    bool doesEntityExist(EntityId entityId) const;
    
    bool isGlobalId(EntityId entityId) const;
    
    bool isLocalId(EntityId entityId) const;
    
    std::vector<EntityId> getGlobalEntities() const;
    
    std::vector<EntityId> getLocalEntities() const;
    
    Entity* getGlobalEntity(EntityId id);
    
    Entity* getLocalEntity(EntityId id);
    
    static constexpr size_t NUM_ENTITY_MANAGERS = 128;
    
private:
    //global entities mapped from range of [0, MAX_GLOBAL_ENTITIES)
    static constexpr size_t MAX_GLOBAL_ENTITIES = 256;
    
    //local entities mapped from a range of [MAX_GLOBAL_ENTITIES, MAX_LOCAL_ENTITIES)
    static constexpr size_t MAX_LOCAL_ENTITIES = MAX_GLOBAL_ENTITIES * 2;
    
    std::array<bool, MAX_LOCAL_ENTITIES> usedEntities = {};
    std::array<Entity, MAX_LOCAL_ENTITIES> entities = {};
};
