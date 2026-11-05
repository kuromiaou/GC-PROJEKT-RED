#pragma once
#include <vector>
#include <cassert>
#include "Entity.h"

template <typename T>
class ComponentStorage {
public:
    void add(EntityId entity, const T& value) {
        if (has(entity)) return;
        if (entity >= sparse.size()) sparse.resize(entity + 1);
        dense.push_back(entity);
        components.push_back(value);
        sparse[entity] = dense.size() - 1;
    }

    void remove(EntityId entity) {
        if (!has(entity)) return;
        std::uint32_t max = dense.size() - 1;
        dense[sparse[entity]] = dense[max];
        components[sparse[entity]] = components[max];
        sparse[dense[max]] = sparse[entity];
        dense.pop_back();
        components.pop_back();

    }

    bool has(EntityId entity) const {
        return inSparse(entity) && inDense(sparse[entity]) && dense[sparse[entity]] == entity;
    };
    
    // Précondition : has(entity).
    T& get(EntityId entity) {
        assert(has(entity) && "get : l'entité n'a pas ce composant");
        return components[entity];
    }

private:
    bool inSparse(EntityId entity) const { return entity < sparse.size(); };
    bool inDense(std::uint32_t index) const { return index < dense.size(); };
    std::vector<EntityId> dense{};
    std::vector<T>  components{};
    std::vector<std::uint32_t> sparse{};
};