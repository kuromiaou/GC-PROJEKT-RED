#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

// CRC-32 standard (IEEE 802.3, celui de zlib et de PNG) : polynôme réfléchi 0xEDB88320,
// valeur initiale 0xFFFFFFFF, XOR final 0xFFFFFFFF. Valeur de contrôle : CRC("123456789") = 0xCBF43926.
// Implémentation par table : la table de 256 valeurs est calculée à la compilation.

namespace crc32_detail {
constexpr std::array<std::uint32_t, 256> MakeTable() {
    std::array<std::uint32_t, 256> table{};
    for (std::uint32_t i = 0; i < 256; ++i) {
        std::uint32_t c = i;
        for (int k = 0; k < 8; ++k) {
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
        table[i] = c;
    }
    return table;
}
inline constexpr std::array<std::uint32_t, 256> Table = MakeTable();
} // namespace crc32_detail

// Pour calculer un CRC sur plusieurs morceaux : partir de Crc32Begin(), enchaîner
// Crc32Update sur chaque morceau, puis Crc32End.
inline constexpr std::uint32_t Crc32Begin() { return 0xFFFFFFFFu; }

inline std::uint32_t Crc32Update(std::uint32_t state, std::span<const std::byte> bytes) {
    for (std::byte b : bytes) {
        state = crc32_detail::Table[(state ^ std::to_integer<std::uint32_t>(b)) & 0xFFu] ^ (state >> 8);
    }
    return state;
}

inline constexpr std::uint32_t Crc32End(std::uint32_t state) { return state ^ 0xFFFFFFFFu; }

inline std::uint32_t Crc32(std::span<const std::byte> bytes) { return Crc32End(Crc32Update(Crc32Begin(), bytes)); }
