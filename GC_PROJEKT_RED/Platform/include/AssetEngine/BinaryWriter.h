#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

class BinaryWriter {
  public:
    void WriteU8(std::uint8_t value) { m_buffer.push_back(std::byte{value}); }

    void WriteU32(std::uint32_t value) {
        for (int i = 0; i < 4; ++i) {
            WriteU8(static_cast<std::uint8_t>(value));
            value >>= 8;
        }
    }

    void WriteString(std::string_view text) {
        if (text.size() > std::numeric_limits<std::uint32_t>::max()) {
            throw std::length_error("String too large");
        }

        WriteU32(static_cast<std::uint32_t>(text.size()));

        for (unsigned char c : text) {
            WriteU8(c);
        }
    }

    std::span<const std::byte> Bytes() const { return m_buffer; }

  private:
    std::vector<std::byte> m_buffer;
};