#pragma once

#include <array>

#include "EntityManager/EntityManager.h"

class ClientEntityManager
{
public:
    ClientEntityManager();
    ~ClientEntityManager();
    
    ClientEntityManager(const ClientEntityManager&) = delete;
    ClientEntityManager& operator=(const ClientEntityManager&) = delete;
    
    EntityManager& getCurrent();
    
    void nextManager();
    
private:
    static constexpr size_t NUM_ENTITY_MANAGERS = 128;
    
    std::array<uint32_t, NUM_ENTITY_MANAGERS> entityManagerSequences = {};
    std::array<EntityManager, NUM_ENTITY_MANAGERS> entityManagers = {};
    size_t currentEntityManager = 0;
    size_t prevAckedEntityManager = -1;
};
