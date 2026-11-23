#pragma once

#include <cstdint>

// Une entrée du répertoire de l'archive. Sur le disque : 8 + 4 + 8 + 8 = 28 octets,
// écrits champ par champ (jamais avec sizeof, qui vaut 32 à cause du remplissage).
struct AssetEntry {
    std::uint64_t id = 0;     // identifiant unique et stable de l'asset
    std::uint32_t type = 0;   // type de l'asset (0 est interdit), défini par les adaptateurs
    std::uint64_t offset = 0; // position du payload dans le fichier
    std::uint64_t size = 0;   // taille du payload en octets
};
