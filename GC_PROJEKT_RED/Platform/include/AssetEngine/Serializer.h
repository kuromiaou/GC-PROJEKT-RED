#pragma once

#include <cstdint>

struct Serializer {
    std::uint64_t id;
    std::uint32_t type;
    std::uint64_t offset;
    std::uint64_t size;
};