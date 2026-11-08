#pragma once
#include <vector>
#include <memory>
#include <cassert>
#include "Entity.h"
#include "IComponentStorage.h"
#include "ComponentStorage.h"
#include "ComponentId.h"

struct Alive {};

class EntityManager {
public:
    EntityId createEntity() {
        EntityId entity;
        if (!freeEntityIds.empty()) {
            entity = freeEntityIds.back();
            freeEntityIds.pop_back();
        }
        else {
            entity = nextEntityId;
            nextEntityId++;
        }
        alive.add(entity, Alive{});
        return entity;
    }

    void destroyEntity(EntityId entity) {
        assert(isAlive(entity) && "destroyEntity : l'entité n'est pas vivante");
        for (auto& storage : storages) {
            if (storage && storage->has(entity)) storage->remove(entity);
        }
        alive.remove(entity);
        freeEntityIds.push_back(entity);
    }

    bool isAlive(EntityId entity) const { return alive.has(entity); }

    template <typename T>
    void addComponent(EntityId entity, const T& value) {
        assert(isAlive(entity) && "addComponent : l'entité n'est pas vivante");
        getStorage<T>().add(entity, value);
    }

    template <typename T>
    void removeComponent(EntityId entity) {
        assert(isAlive(entity) && "removeComponent : l'entité n'est pas vivante");
        getStorage<T>().remove(entity);
    }

    template <typename T>
    bool hasComponent(EntityId entity) {
        return isAlive(entity) && getStorage<T>().has(entity);
    }

    // Précondition : hasComponent<T>(entity).
    template <typename T>
    T& getComponent(EntityId entity) {
        assert(isAlive(entity) && "getComponent : l'entité n'est pas vivante");
        return getStorage<T>().get(entity);
    }

    template <typename T>
    const std::vector<EntityId>& entitiesWith() {
        return getStorage<T>().entities();
    }

private:
    template <typename T>
    ComponentStorage<T>& getStorage() {
        ComponentId id = getComponentId<T>();
        if (id >= storages.size()) storages.resize(id + 1);
        if (!storages[id]) storages[id] = std::make_unique<ComponentStorage<T>>();
        return *static_cast<ComponentStorage<T>*>(storages[id].get());
    }

    EntityId nextEntityId = 0;
    std::vector<EntityId> freeEntityIds{};
    ComponentStorage<Alive> alive{};
    std::vector<std::unique_ptr<IComponentStorage>> storages{};
};