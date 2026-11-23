#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "AssetEngine/BinaryReader.h"
#include "AssetEngine/BinaryWriter.h"
#include "AssetEngine/Sequence.h"

// Deux types généraux, sans rapport avec la physique, et leurs adaptateurs.
// Un adaptateur = une paire Write / Read. Le cœur (Writer, Reader, Archive) n'est pas modifié
// pour les ajouter : c'est ce qui rend le module réutilisable.

// Identifiants de type utilisés dans le répertoire de l'archive.
namespace asset_types {
inline constexpr std::uint32_t Player = 1;
inline constexpr std::uint32_t Inventory = 2;
} // namespace asset_types

// ---------------------------------------------------------------- Player
// Deux versions du schéma :
//   V1 : health, speed, name
//   V2 : health, speed, name, level   (champ ajouté)
// Le payload commence par le numéro de version du schéma (U32).
// Un payload V1 est relu en V2 avec level = 1 (valeur par défaut documentée).

inline constexpr std::uint32_t PlayerSchemaV1 = 1;
inline constexpr std::uint32_t PlayerSchemaV2 = 2;
inline constexpr std::uint32_t PlayerDefaultLevel = 1;

struct PlayerV1 {
    std::uint32_t health = 0;
    float speed = 0.0f;
    std::string name;
};

struct Player { // version actuelle (V2)
    std::uint32_t health = 0;
    float speed = 0.0f;
    std::string name;
    std::uint32_t level = PlayerDefaultLevel;

    bool operator==(const Player &) const = default;
};

// Écrit l'ancienne version : sert à produire des archives V1 pour tester la migration.
inline void Write(BinaryWriter &writer, const PlayerV1 &player) {
    writer.WriteU32(PlayerSchemaV1);
    writer.WriteU32(player.health);
    writer.WriteF32(player.speed);
    writer.WriteString(player.name);
}

inline void Write(BinaryWriter &writer, const Player &player) {
    writer.WriteU32(PlayerSchemaV2);
    writer.WriteU32(player.health);
    writer.WriteF32(player.speed);
    writer.WriteString(player.name);
    writer.WriteU32(player.level);
}

// Lit V1 ou V2. Toute autre version est refusée explicitement.
inline bool Read(BinaryReader &reader, Player &player) {
    const std::size_t start = reader.Position();
    std::uint32_t schema = 0;
    Player temp;
    bool ok = reader.ReadU32(schema);
    if (ok && (schema == PlayerSchemaV1 || schema == PlayerSchemaV2)) {
        ok = reader.ReadU32(temp.health) && reader.ReadF32(temp.speed) && reader.ReadString(temp.name);
        if (ok && schema == PlayerSchemaV2) {
            ok = reader.ReadU32(temp.level);
        } else if (ok) {
            temp.level = PlayerDefaultLevel; // migration V1 -> V2
        }
    } else {
        ok = false; // version inconnue
    }
    if (!ok) {
        reader.Rewind(start);
        return false;
    }
    player = std::move(temp);
    return true;
}

// ---------------------------------------------------------------- Inventory
// Un type avec une chaîne ET une collection.

struct Item {
    std::uint32_t id = 0;
    std::uint32_t count = 0;

    bool operator==(const Item &) const = default;
};

struct Inventory {
    std::string owner;
    std::vector<Item> items;

    bool operator==(const Inventory &) const = default;
};

inline constexpr std::uint32_t InventoryMaxItems = 100'000;
inline constexpr std::size_t ItemSize = 8; // id U32 + count U32

inline void Write(BinaryWriter &writer, const Item &item) {
    writer.WriteU32(item.id);
    writer.WriteU32(item.count);
}

inline bool Read(BinaryReader &reader, Item &item) {
    const std::size_t start = reader.Position();
    Item temp;
    if (!reader.ReadU32(temp.id) || !reader.ReadU32(temp.count)) {
        reader.Rewind(start);
        return false;
    }
    item = temp;
    return true;
}

inline void Write(BinaryWriter &writer, const Inventory &inventory) {
    writer.WriteString(inventory.owner);
    WriteSequence(writer, inventory.items, [](BinaryWriter &w, const Item &item) { Write(w, item); });
}

inline bool Read(BinaryReader &reader, Inventory &inventory) {
    const std::size_t start = reader.Position();
    Inventory temp;
    const bool ok = reader.ReadString(temp.owner) &&
                    ReadSequence(reader, temp.items, InventoryMaxItems, ItemSize,
                                 [](BinaryReader &r, Item &item) { return Read(r, item); });
    if (!ok) {
        reader.Rewind(start);
        return false;
    }
    inventory = std::move(temp);
    return true;
}
