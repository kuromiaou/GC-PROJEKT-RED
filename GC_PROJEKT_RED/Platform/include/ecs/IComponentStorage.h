#pragma once
#include "Entity.h"

class IComponentStorage {
public:
    virtual ~IComponentStorage() = default;

    virtual void remove(EntityId entity) = 0;
    virtual bool has(EntityId entity) const = 0;
};