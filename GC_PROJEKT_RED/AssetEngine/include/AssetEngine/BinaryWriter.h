#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

// Le format suppose des float IEEE 754 sur 32 bits : la compilation échoue sinon.
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
              "Le format suppose des float IEEE 754 sur 32 bits");

// Écrit des valeurs dans un tableau d'octets, en little-endian.
// Ne connaît aucun type métier : ce sont les adaptateurs qui l'utilisent.
class BinaryWriter {
  public:
    void WriteU8(std::uint8_t value) { m_buffer.push_back(std::byte{value}); }

    void WriteU32(std::uint32_t value) {
        for (int i = 0; i < 4; ++i) {
            WriteU8(static_cast<std::uint8_t>(value));
            value >>= 8;
        }
    }

    void WriteU64(std::uint64_t value) {
        for (int i = 0; i < 8; ++i) {
            WriteU8(static_cast<std::uint8_t>(value));
            value >>= 8;
        }
    }

    // Entier signé : écrit avec ses bits en complément à deux.
    void WriteI32(std::int32_t value) { WriteU32(static_cast<std::uint32_t>(value)); }

    void WriteBool(bool value) { WriteU8(value ? 1 : 0); }

    void WriteF32(float value) { WriteU32(std::bit_cast<std::uint32_t>(value)); }

    // Longueur en U32, puis les octets UTF-8.
    void WriteString(std::string_view text) {
        if (text.size() > std::numeric_limits<std::uint32_t>::max()) {
            throw std::length_error("String too large");
        }
        WriteU32(static_cast<std::uint32_t>(text.size()));
        for (unsigned char c : text) {
            WriteU8(c);
        }
    }

    // Octets bruts, sans longueur (utilisé par l'archive pour recopier les payloads).
    void WriteBytes(std::span<const std::byte> bytes) { m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end()); }

    std::size_t Size() const { return m_buffer.size(); }

    std::span<const std::byte> Bytes() const { return m_buffer; }

    // Récupère le tableau sans le copier. Le Writer est vide ensuite.
    std::vector<std::byte> TakeBytes() { return std::move(m_buffer); }

  private:
    std::vector<std::byte> m_buffer;
};
