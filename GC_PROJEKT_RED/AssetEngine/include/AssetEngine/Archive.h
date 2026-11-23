#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AssetEngine/AssetEntry.h"
#include "AssetEngine/BinaryReader.h"
#include "AssetEngine/BinaryWriter.h"
#include "AssetEngine/Crc32.h"

// Format d'archive (version 1), tous les entiers en little-endian :
//
//   offset 0   magic            4 octets  "GAST"
//   offset 4   formatVersion    U32       1
//   offset 8   assetCount       U32
//   offset 12  directoryOffset  U64       position du répertoire
//   offset 20  crc32            U32       CRC-32 de tout le fichier SAUF ces 4 octets
//   offset 24  payloads         les données des assets, les unes après les autres
//   ...        répertoire       assetCount entrées de 28 octets, jusqu'à la fin du fichier
//
// Le répertoire est écrit à la fin : on connaît alors tous les offsets sans calcul préalable.
// Le CRC couvre le header (sans son propre champ), les payloads et le répertoire.

namespace archive {
inline constexpr std::uint8_t Magic[4] = {'G', 'A', 'S', 'T'};
inline constexpr std::uint32_t FormatVersion = 1;
inline constexpr std::size_t HeaderSize = 24;
inline constexpr std::size_t CrcOffset = 20;
inline constexpr std::size_t EntrySize = 28;
inline constexpr std::uint32_t MaxAssets = 1'000'000;
inline constexpr std::uint64_t MaxFileSize = 1ull << 30; // 1 Gio : borne avant allocation

// Vrai si [offset, offset + size) tient dans [0, total), sans débordement possible.
inline bool IsValidRange(std::uint64_t offset, std::uint64_t size, std::uint64_t total) {
    return offset <= total && size <= total - offset;
}

inline std::uint32_t ComputeCrc(std::span<const std::byte> file) {
    std::uint32_t state = Crc32Begin();
    state = Crc32Update(state, file.subspan(0, CrcOffset));
    state = Crc32Update(state, file.subspan(HeaderSize));
    return Crc32End(state);
}
} // namespace archive

// Lit un fichier entier en mémoire. Refuse un fichier plus grand que MaxFileSize.
inline bool LoadFileBytes(const std::filesystem::path &path, std::vector<std::byte> &out, std::string &error) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        error = "impossible d'ouvrir " + path.string();
        return false;
    }
    const std::streamoff size = file.tellg();
    if (size < 0 || static_cast<std::uint64_t>(size) > archive::MaxFileSize) {
        error = "taille de fichier invalide ou trop grande";
        return false;
    }
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    file.seekg(0);
    if (size > 0 && !file.read(reinterpret_cast<char *>(bytes.data()), size)) {
        error = "erreur de lecture de " + path.string();
        return false;
    }
    out = std::move(bytes);
    return true;
}

// Écrit d'abord dans un fichier temporaire, puis le renomme : en cas d'échec,
// l'ancien fichier reste intact et aucun fichier à moitié écrit ne reste.
inline bool SaveFileBytes(const std::filesystem::path &path, std::span<const std::byte> bytes, std::string &error) {
    std::filesystem::path temp = path;
    temp += ".tmp";
    {
        std::ofstream file(temp, std::ios::binary | std::ios::trunc);
        if (!file) {
            error = "impossible de creer " + temp.string();
            return false;
        }
        file.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        file.close();
        if (!file) {
            error = "erreur d'ecriture dans " + temp.string();
            std::error_code ignored;
            std::filesystem::remove(temp, ignored);
            return false;
        }
    }
    std::error_code ec;
    std::filesystem::rename(temp, path, ec);
    if (ec) {
        error = "impossible de renommer vers " + path.string() + " : " + ec.message();
        std::error_code ignored;
        std::filesystem::remove(temp, ignored);
        return false;
    }
    return true;
}

// Construit une archive. Chaque asset est sérialisé par son adaptateur au moment de Add.
class ArchiveWriter {
  public:
    // Ajoute un asset déjà sérialisé. Refuse un ID déjà présent ou un type 0.
    bool AddRaw(std::uint64_t id, std::uint32_t type, std::vector<std::byte> payload) {
        if (type == 0 || m_index.contains(id)) {
            return false;
        }
        m_index.emplace(id, m_assets.size());
        m_assets.push_back({id, type, std::move(payload)});
        return true;
    }

    // Sérialise value avec son adaptateur Write(BinaryWriter&, const T&).
    template <typename T> bool Add(std::uint64_t id, std::uint32_t type, const T &value) {
        BinaryWriter payload;
        Write(payload, value);
        return AddRaw(id, type, payload.TakeBytes());
    }

    std::size_t Count() const { return m_assets.size(); }

    std::vector<std::byte> Build() const {
        BinaryWriter out;
        std::uint64_t payloadBytes = 0;
        for (const Pending &asset : m_assets) {
            payloadBytes += asset.payload.size();
        }
        const std::uint64_t directoryOffset = archive::HeaderSize + payloadBytes;

        // Header (le CRC est complété à la fin)
        for (std::uint8_t c : archive::Magic) {
            out.WriteU8(c);
        }
        out.WriteU32(archive::FormatVersion);
        out.WriteU32(static_cast<std::uint32_t>(m_assets.size()));
        out.WriteU64(directoryOffset);
        out.WriteU32(0);

        // Payloads
        std::vector<AssetEntry> entries;
        entries.reserve(m_assets.size());
        for (const Pending &asset : m_assets) {
            entries.push_back({asset.id, asset.type, out.Size(), asset.payload.size()});
            out.WriteBytes(asset.payload);
        }

        // Répertoire
        for (const AssetEntry &entry : entries) {
            out.WriteU64(entry.id);
            out.WriteU32(entry.type);
            out.WriteU64(entry.offset);
            out.WriteU64(entry.size);
        }

        std::vector<std::byte> bytes = out.TakeBytes();
        std::uint32_t crc = archive::ComputeCrc(bytes);
        for (std::size_t i = 0; i < 4; ++i) {
            bytes[archive::CrcOffset + i] = std::byte{static_cast<std::uint8_t>(crc)};
            crc >>= 8;
        }
        return bytes;
    }

    bool SaveToFile(const std::filesystem::path &path, std::string &error) const {
        const std::vector<std::byte> bytes = Build();
        return SaveFileBytes(path, bytes, error);
    }

  private:
    struct Pending {
        std::uint64_t id;
        std::uint32_t type;
        std::vector<std::byte> payload;
    };
    std::vector<Pending> m_assets;
    std::unordered_map<std::uint64_t, std::size_t> m_index;
};

// Ouvre une archive : tout est validé à l'ouverture, puis on charge les assets par ID
// sans désérialiser les autres.
class ArchiveReader {
  public:
    // Valide et adopte bytes. En cas d'échec, l'ArchiveReader reste vide et error explique pourquoi.
    bool Open(std::vector<std::byte> bytes, std::string &error) {
        Clear();
        const std::uint64_t total = bytes.size();
        if (total < archive::HeaderSize) {
            error = "fichier tronque : header incomplet";
            return false;
        }
        BinaryReader reader(bytes);
        std::uint8_t magic[4] = {};
        for (std::uint8_t &c : magic) {
            reader.ReadU8(c);
        }
        for (int i = 0; i < 4; ++i) {
            if (magic[i] != archive::Magic[i]) {
                error = "magic number invalide : ce n'est pas une archive GAST";
                return false;
            }
        }
        std::uint32_t version = 0, count = 0, storedCrc = 0;
        std::uint64_t directoryOffset = 0;
        reader.ReadU32(version);
        reader.ReadU32(count);
        reader.ReadU64(directoryOffset);
        reader.ReadU32(storedCrc);

        if (version != archive::FormatVersion) {
            error = "version de format non supportee : " + std::to_string(version);
            return false;
        }
        if (archive::ComputeCrc(bytes) != storedCrc) {
            error = "CRC-32 invalide : archive corrompue";
            return false;
        }
        if (count > archive::MaxAssets) {
            error = "nombre d'assets excessif : " + std::to_string(count);
            return false;
        }
        const std::uint64_t directorySize = std::uint64_t(count) * archive::EntrySize;
        if (directoryOffset < archive::HeaderSize || !archive::IsValidRange(directoryOffset, directorySize, total) ||
            directoryOffset + directorySize != total) {
            error = "position ou taille du repertoire invalide";
            return false;
        }

        std::vector<AssetEntry> entries;
        std::unordered_map<std::uint64_t, std::size_t> index;
        entries.reserve(count);
        BinaryReader dir(std::span<const std::byte>(bytes).subspan(static_cast<std::size_t>(directoryOffset)));
        for (std::uint32_t i = 0; i < count; ++i) {
            AssetEntry entry;
            dir.ReadU64(entry.id);
            dir.ReadU32(entry.type);
            dir.ReadU64(entry.offset);
            dir.ReadU64(entry.size);
            if (entry.type == 0) {
                error = "type d'asset invalide (0) pour l'id " + std::to_string(entry.id);
                return false;
            }
            // Le payload doit être entre la fin du header et le début du répertoire.
            if (entry.offset < archive::HeaderSize || !archive::IsValidRange(entry.offset, entry.size, directoryOffset)) {
                error = "offset ou taille hors limites pour l'id " + std::to_string(entry.id);
                return false;
            }
            if (!index.emplace(entry.id, entries.size()).second) {
                error = "id en double : " + std::to_string(entry.id);
                return false;
            }
            entries.push_back(entry);
        }

        m_bytes = std::move(bytes);
        m_entries = std::move(entries);
        m_index = std::move(index);
        return true;
    }

    bool OpenFile(const std::filesystem::path &path, std::string &error) {
        std::vector<std::byte> bytes;
        if (!LoadFileBytes(path, bytes, error)) {
            return false;
        }
        return Open(std::move(bytes), error);
    }

    const std::vector<AssetEntry> &Entries() const { return m_entries; }

    const AssetEntry *Find(std::uint64_t id) const {
        auto it = m_index.find(id);
        return it == m_index.end() ? nullptr : &m_entries[it->second];
    }

    // Octets bruts d'un asset (vue sur le buffer interne, valable tant que l'ArchiveReader vit).
    std::span<const std::byte> Payload(const AssetEntry &entry) const {
        return std::span<const std::byte>(m_bytes).subspan(static_cast<std::size_t>(entry.offset),
                                                           static_cast<std::size_t>(entry.size));
    }

    // Charge l'asset id avec son adaptateur Read(BinaryReader&, T&).
    // Tout ou rien : value n'est modifié que si le décodage complet réussit.
    template <typename T> bool Load(std::uint64_t id, std::uint32_t expectedType, T &value, std::string &error) const {
        const AssetEntry *entry = Find(id);
        if (!entry) {
            error = "id inconnu : " + std::to_string(id);
            return false;
        }
        if (entry->type != expectedType) {
            error = "type inattendu pour l'id " + std::to_string(id);
            return false;
        }
        BinaryReader reader(Payload(*entry));
        T temp{};
        if (!Read(reader, temp)) {
            error = "payload invalide ou version de schema non supportee pour l'id " + std::to_string(id);
            return false;
        }
        if (reader.Remaining() != 0) {
            error = "octets en trop dans le payload de l'id " + std::to_string(id);
            return false;
        }
        value = std::move(temp);
        return true;
    }

  private:
    void Clear() {
        m_bytes.clear();
        m_entries.clear();
        m_index.clear();
    }

    std::vector<std::byte> m_bytes;
    std::vector<AssetEntry> m_entries;
    std::unordered_map<std::uint64_t, std::size_t> m_index;
};
