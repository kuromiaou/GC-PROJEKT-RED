#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#include "AssetEngine/BinaryReader.h"
#include "AssetEngine/BinaryWriter.h"

// Séquences de taille variable : le nombre d'éléments en U32, puis chaque élément.
// writeOne / readOne disent comment écrire et lire UN élément : ce sont les adaptateurs.

template <typename T, typename WriteOne>
void WriteSequence(BinaryWriter &writer, const std::vector<T> &items, WriteOne writeOne) {
    if (items.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw std::length_error("Sequence too large");
    }
    writer.WriteU32(static_cast<std::uint32_t>(items.size()));
    for (const T &item : items) {
        writeOne(writer, item);
    }
}

// maxCount : nombre maximal accepté (borne fixée par le format).
// minElementSize : taille minimale d'un élément sur le disque, pour refuser un nombre
// impossible AVANT d'allouer : count éléments demandent au moins count * minElementSize octets.
template <typename T, typename ReadOne>
bool ReadSequence(BinaryReader &reader, std::vector<T> &items, std::uint32_t maxCount, std::size_t minElementSize,
                  ReadOne readOne) {
    const std::size_t start = reader.Position();
    std::uint32_t count = 0;
    if (!reader.ReadU32(count)) {
        return false;
    }
    if (count > maxCount || (minElementSize > 0 && count > reader.Remaining() / minElementSize)) {
        reader.Rewind(start);
        return false;
    }
    std::vector<T> result;
    result.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        T item{};
        if (!readOne(reader, item)) {
            reader.Rewind(start);
            return false;
        }
        result.push_back(std::move(item));
    }
    items = std::move(result);
    return true;
}
