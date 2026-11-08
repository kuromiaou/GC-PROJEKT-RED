#pragma once
#include <cstdint>

using ComponentId = std::uint32_t;

inline ComponentId nextComponentId() {
    static ComponentId counter = 0;
    return counter++;
}

template <typename T>
ComponentId getComponentId() {
    static const ComponentId id = nextComponentId();
    return id;
}