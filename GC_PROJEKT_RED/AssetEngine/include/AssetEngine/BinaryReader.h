#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <utility>

// Lit des valeurs dans un tableau d'octets, en little-endian.
// Règle commune à toutes les fonctions Read : en cas d'échec, elles renvoient false,
// ne modifient pas la valeur de sortie et ne consomment aucun octet.
class BinaryReader {
  public:
    explicit BinaryReader(std::span<const std::byte> bytes) : m_bytes(bytes) {}

    bool ReadU8(std::uint8_t &value) {
        if (m_cursor >= m_bytes.size()) {
            return false;
        }
        value = std::to_integer<std::uint8_t>(m_bytes[m_cursor]);
        m_cursor += 1;
        return true;
    }

    bool ReadU32(std::uint32_t &value) {
        if (m_cursor > m_bytes.size() || m_bytes.size() - m_cursor < 4) {
            return false;
        }
        std::uint32_t result = 0;
        for (int i = 0; i < 4; ++i) {
            result |= std::uint32_t(std::to_integer<std::uint8_t>(m_bytes[m_cursor + i])) << (8 * i);
        }
        m_cursor += 4;
        value = result;
        return true;
    }

    bool ReadU64(std::uint64_t &value) {
        if (m_cursor > m_bytes.size() || m_bytes.size() - m_cursor < 8) {
            return false;
        }
        std::uint64_t result = 0;
        for (int i = 0; i < 8; ++i) {
            result |= std::uint64_t(std::to_integer<std::uint8_t>(m_bytes[m_cursor + i])) << (8 * i);
        }
        m_cursor += 8;
        value = result;
        return true;
    }

    bool ReadI32(std::int32_t &value) {
        std::uint32_t bits = 0;
        if (!ReadU32(bits)) {
            return false;
        }
        value = static_cast<std::int32_t>(bits);
        return true;
    }

    bool ReadBool(bool &value) {
        const std::size_t start = m_cursor;
        std::uint8_t byte = 0;
        if (!ReadU8(byte)) {
            return false;
        }
        if (byte > 1) {
            m_cursor = start;
            return false;
        }
        value = (byte == 1);
        return true;
    }

    bool ReadF32(float &value) {
        std::uint32_t bits = 0;
        if (!ReadU32(bits)) {
            return false;
        }
        value = std::bit_cast<float>(bits);
        return true;
    }

    bool ReadString(std::string &value) {
        const std::size_t start = m_cursor;
        std::uint32_t length = 0;
        if (!ReadU32(length)) {
            return false;
        }
        constexpr std::uint32_t MaxLength = 1024 * 1024;
        if (length > MaxLength || m_cursor > m_bytes.size() || length > m_bytes.size() - m_cursor) {
            m_cursor = start;
            return false;
        }
        std::string result;
        result.reserve(length);
        for (std::uint32_t i = 0; i < length; ++i) {
            result.push_back(static_cast<char>(std::to_integer<unsigned char>(m_bytes[m_cursor + i])));
        }
        m_cursor += length;
        value = std::move(result);
        return true;
    }

    std::size_t Position() const { return m_cursor; }

    std::size_t Remaining() const { return m_cursor < m_bytes.size() ? m_bytes.size() - m_cursor : 0; }

    // Revient à une position déjà lue (utilisé pour annuler une lecture composée qui échoue).
    void Rewind(std::size_t position) {
        if (position <= m_cursor) {
            m_cursor = position;
        }
    }

  private:
    std::span<const std::byte> m_bytes;
    std::size_t m_cursor = 0;
};
