#pragma once 

#include < cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <utility>

class BinaryReader {
  public:
    explicit BinaryReader(std::span<const std::byte> bytes) : m_bytes(bytes) {}

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

  private:
    std::span<const std::byte> m_bytes;
    std::size_t m_cursor = 0;
};