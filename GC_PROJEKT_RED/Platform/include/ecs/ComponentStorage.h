#pragma once
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

#include "Entity.h"
#include "IComponentStorage.h"

template <typename T>
class ComponentStorage : public IComponentStorage {
public:
    void add(EntityId entity, const T& value) {
        if (has(entity)) return;
        if (entity >= sparse.size()) sparse.resize(entity + 1);
        dense.push_back(entity);
        components.push_back(value);
        sparse[entity] = static_cast<std::uint32_t>(dense.size() - 1);
    }

    void remove(EntityId entity) override {
        if (!has(entity)) return;
        const std::uint32_t removedIndex = sparse[entity];
        const std::uint32_t lastIndex = static_cast<std::uint32_t>(dense.size() - 1);
        const EntityId movedEntity = dense[lastIndex];

        // Le dernier élément prend la place de celui qu'on retire
        dense[removedIndex] = movedEntity;
        components[removedIndex] = std::move(components[lastIndex]);
        sparse[movedEntity] = removedIndex;

        dense.pop_back();
        components.pop_back();
    }

    bool has(EntityId entity) const override {
        return inSparse(entity) && inDense(sparse[entity]) && dense[sparse[entity]] == entity;
    }

    // Précondition : has(entity).
    T& get(EntityId entity) {
        assert(has(entity) && "get : l'entité n'a pas ce composant");
        return components[sparse[entity]];
    }

    const std::vector<EntityId>& entities() const { return dense; }

private:
    bool inSparse(EntityId entity) const { return entity < sparse.size(); }
    bool inDense(std::uint32_t index) const { return index < dense.size(); }

    std::vector<EntityId> dense{};
    std::vector<T> components{};
    std::vector<std::uint32_t> sparse{};
};