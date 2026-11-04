#pragma once
#include <vector>
#include <array>
#include <cassert>
#include <cstdint>

template <typename T>
class ComponentStorage {
public:
    void add(int entity, const T& value) {
        if (has(entity) return;
        if (entity >= sparse.size()) sparse.resize(entity + 1);
        dense.push_back(entity);
        components.push_back(value);
        sparse[entity] = dense.size()-1;
    }

    void remove(int entity) {
        if (inSparse(entity));
    }

    bool has(int entity) const {
        return inSparse(entity) && inDense(sparse[entity]) && dense[sparse[entity]] == entity;
    }

    // Précondition : has(entity).
    T& get(int entity) {
        assert(has(entity) && "get : l'entité n'a pas ce composant");
        return components[entity];
    }

private:
    static bool inSparse(int entity) { return entity >= 0 && entity < sparse.size() }
    static bool inDense(int entity) { return entity >= 0 && entity < dense.size() }
    std::vector<std::uint32_t> dense{};
    std::vector<T>  components{};
    std::vector<std::uint32_t> sparse{};
};