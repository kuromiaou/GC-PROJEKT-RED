#pragma once

// ============================================================================
//  ECS minimal : Entity / Component / System
//  - Entity    : un simple numéro
//  - Component : une struct de données (stockée dans un ComponentPool<T>)
//  - System    : une fonction qui appelle Registry::Each<...>
//  Stockage    : sparse set (que des std::vector, composants contigus)
// ============================================================================

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

using Entity = uint32_t;

// ----------------------------------------------------------------------------
// IPool : base commune des pools. Permet au Registry de ranger des pools de
// types différents dans un seul vector, et de dire "oublie l'entité e"
// sans connaître le type du composant.
// ----------------------------------------------------------------------------
class IPool {
public:
    virtual ~IPool() = default;
    virtual void Remove(Entity e) = 0;
};

// ----------------------------------------------------------------------------
// ComponentPool<T> : tous les composants d'UN type, en sparse set.
//
//   sparse[entité]  -> position dans dense (ou NONE si pas de composant)
//   dense[i]        -> le composant (tableau compact, sans trou)
//   owners[i]       -> l'entité à qui appartient dense[i]
// ----------------------------------------------------------------------------
template <typename T>
class ComponentPool : public IPool {
public:
    static constexpr uint32_t NONE = UINT32_MAX;

    // Ajoute (ou remplace) le composant de l'entité e.
    // Attention : la référence renvoyée devient invalide si on ajoute ensuite
    // un autre composant du MÊME type (le vector peut se réallouer).
    T& Add(Entity e, T value) {
        if (Has(e)) {
            dense[sparse[e]] = std::move(value);
            return dense[sparse[e]];
        }
        if (e >= sparse.size()) sparse.resize(static_cast<size_t>(e) + 1, NONE);
        sparse[e] = static_cast<uint32_t>(dense.size());   // il ira à la fin
        dense.push_back(std::move(value));
        owners.push_back(e);
        return dense.back();
    }

    bool Has(Entity e) const {
        return e < sparse.size() && sparse[e] != NONE;
    }

    // Précondition : Has(e). (assert en mode debug)
    T& Get(Entity e) {
        assert(Has(e) && "Get<T> : l'entité n'a pas ce composant");
        return dense[sparse[e]];
    }

    // Swap-and-pop : le dernier composant comble le trou, donc O(1) et sans trou.
    void Remove(Entity e) override {
        if (!Has(e)) return;
        const uint32_t idx = sparse[e];
        const uint32_t last = static_cast<uint32_t>(dense.size()) - 1;
        if (idx != last) {
            const Entity moved = owners[last];
            dense[idx] = std::move(dense[last]);
            owners[idx] = moved;
            sparse[moved] = idx;          // le dernier a changé de place
        }
        sparse[e] = NONE;
        dense.pop_back();
        owners.pop_back();
    }

    // Accès par indice, pour parcourir le tableau compact (utilisé par Each).
    size_t Size() const { return dense.size(); }
    Entity OwnerAt(size_t i) const { return owners[i]; }
    T& At(size_t i) { return dense[i]; }

private:
    std::vector<uint32_t> sparse;   // indexé par l'entité
    std::vector<T>        dense;    // composants contigus
    std::vector<Entity>   owners;   // dense[i] appartient à owners[i]
};

// ----------------------------------------------------------------------------
// TypeId<T>() : un numéro unique par type (0, 1, 2...), attribué au premier
// appel. Sert d'indice dans le vector de pools (pas de map, pas de hachage).
// ----------------------------------------------------------------------------
inline size_t NextTypeId() {
    static size_t id = 0;
    return id++;
}

template <typename T>
size_t TypeId() {
    static const size_t id = NextTypeId();   // initialisé UNE fois par type T
    return id;
}

// ----------------------------------------------------------------------------
// Registry : entités + pools de composants.
// ----------------------------------------------------------------------------
class Registry {
public:
    // --- Entités ---------------------------------------------------------
    Entity Create() {
        Entity e;
        if (!freeIds.empty()) {
            e = freeIds.back();
            freeIds.pop_back();
        }
        else {
            e = next++;
            alive.push_back(false);
        }
        alive[e] = true;
        return e;
    }

    bool IsAlive(Entity e) const { return e < alive.size() && alive[e]; }

    void Destroy(Entity e) {
        if (!IsAlive(e)) return;                  // évite le double Destroy
        for (auto& pool : pools)
            if (pool) pool->Remove(e);
        alive[e] = false;
        freeIds.push_back(e);
    }

    // --- Composants ------------------------------------------------------
    template <typename T>
    T& Add(Entity e, T value) {
        assert(IsAlive(e));
        return GetPool<T>().Add(e, std::move(value));
    }

    template <typename T>
    T& Get(Entity e) {
        return GetPool<T>().Get(e);
    }

    template <typename T>
    bool Has(Entity e) {
        const size_t id = TypeId<T>();
        return id < pools.size() && pools[id] &&
            static_cast<ComponentPool<T>&>(*pools[id]).Has(e);
    }

    template <typename T>
    void Remove(Entity e) {
        const size_t id = TypeId<T>();
        if (id < pools.size() && pools[id]) pools[id]->Remove(e);
    }

    // --- Parcours --------------------------------------------------------
    // Appelle fn(entité, First&, Rest&...) pour chaque entité qui possède TOUS
    // les composants demandés.
    // Règle : dans fn, ne pas ajouter/retirer de composant First, ni détruire
    // d'entité. Stocker ces entités dans un vector et les détruire après.
    template <typename First, typename... Rest, typename Fn>
    void Each(Fn&& fn) {
        auto& pool = GetPool<First>();
        for (size_t i = 0; i < pool.Size(); ++i) {
            const Entity e = pool.OwnerAt(i);
            if ((Has<Rest>(e) && ...))            // fold expression (C++17)
                fn(e, pool.At(i), Get<Rest>(e)...);
        }
    }

private:
    template <typename T>
    ComponentPool<T>& GetPool() {
        const size_t id = TypeId<T>();
        if (id >= pools.size()) pools.resize(id + 1);
        if (!pools[id]) pools[id] = std::make_unique<ComponentPool<T>>();
        return static_cast<ComponentPool<T>&>(*pools[id]);
    }

    Entity next = 0;
    std::vector<Entity> freeIds;
    std::vector<bool>   alive;
    std::vector<std::unique_ptr<IPool>> pools;    // indexé par TypeId<T>()
};