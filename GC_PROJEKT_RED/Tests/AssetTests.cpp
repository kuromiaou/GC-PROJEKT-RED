// Tests automatisés du coeur AssetEngine : couche octets, séquences, CRC-32 et archive.
// Ce programme ne lie QUE la bibliothèque AssetEngine : aucun type métier, aucune physique.
// Chaque test vérifie un aller-retour (écrire puis relire la même valeur),
// les octets exacts produits (little-endian), ou le rejet d'une donnée invalide.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <vector>

#include <filesystem>

#include "AssetEngine/Archive.h"
#include "AssetEngine/BinaryReader.h"
#include "AssetEngine/BinaryWriter.h"
#include "AssetEngine/Crc32.h"
#include "AssetEngine/Sequence.h"

static int failures = 0;

static void check(bool condition, const char *description) {
    if (condition) {
        std::printf("[OK]    %s\n", description);
    } else {
        std::printf("[ECHEC] %s\n", description);
        failures++;
    }
}

// Compare le contenu du Writer à une liste d'octets attendue.
static bool bytesEqual(std::span<const std::byte> actual, const std::vector<std::uint8_t> &expected) {
    if (actual.size() != expected.size()) {
        return false;
    }
    for (std::size_t i = 0; i < expected.size(); ++i) {
        if (std::to_integer<std::uint8_t>(actual[i]) != expected[i]) {
            return false;
        }
    }
    return true;
}

// Construit un buffer à partir d'octets écrits à la main (pour simuler un fichier abîmé).
static std::vector<std::byte> makeBuffer(const std::vector<std::uint8_t> &values) {
    std::vector<std::byte> buffer;
    for (std::uint8_t v : values) {
        buffer.push_back(std::byte{v});
    }
    return buffer;
}

static void testU8() {
    std::printf("\n--- U8 ---\n");
    BinaryWriter writer;
    writer.WriteU8(0);
    writer.WriteU8(255);
    check(bytesEqual(writer.Bytes(), {0x00, 0xFF}), "WriteU8 ecrit un octet par valeur");

    BinaryReader reader(writer.Bytes());
    std::uint8_t a = 1, b = 0, c = 0;
    check(reader.ReadU8(a) && a == 0, "ReadU8 relit 0");
    check(reader.ReadU8(b) && b == 255, "ReadU8 relit 255");
    check(!reader.ReadU8(c), "ReadU8 echoue a la fin du buffer");
}

static void testU32() {
    std::printf("\n--- U32 ---\n");
    BinaryWriter writer;
    writer.WriteU32(0x12345678);
    check(bytesEqual(writer.Bytes(), {0x78, 0x56, 0x34, 0x12}), "WriteU32 ecrit en little-endian (78 56 34 12)");

    BinaryReader reader(writer.Bytes());
    std::uint32_t value = 0;
    check(reader.ReadU32(value) && value == 0x12345678, "ReadU32 relit 0x12345678");

    auto truncated = makeBuffer({0x01, 0x02, 0x03});
    BinaryReader shortReader(truncated);
    std::uint32_t untouched = 42;
    check(!shortReader.ReadU32(untouched), "ReadU32 refuse un buffer de 3 octets");
    check(untouched == 42, "ReadU32 ne modifie pas la valeur en cas d'echec");
}

static void testU64() {
    std::printf("\n--- U64 ---\n");
    BinaryWriter writer;
    writer.WriteU64(0x0102030405060708ULL);
    check(bytesEqual(writer.Bytes(), {0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}),
          "WriteU64 ecrit 8 octets en little-endian");

    BinaryReader reader(writer.Bytes());
    std::uint64_t value = 0;
    check(reader.ReadU64(value) && value == 0x0102030405060708ULL, "ReadU64 relit 0x0102030405060708");

    // Une valeur qui ne tient pas sur 32 bits : verifie les 4 octets du haut.
    BinaryWriter big;
    big.WriteU64(5000000000ULL);
    BinaryReader bigReader(big.Bytes());
    std::uint64_t bigValue = 0;
    check(bigReader.ReadU64(bigValue) && bigValue == 5000000000ULL, "ReadU64 relit 5 000 000 000 (plus de 32 bits)");

    // Deux U64 a la suite : verifie que le curseur avance bien de 8.
    BinaryWriter two;
    two.WriteU64(1);
    two.WriteU64(2);
    BinaryReader twoReader(two.Bytes());
    std::uint64_t first = 0, second = 0;
    check(twoReader.ReadU64(first) && twoReader.ReadU64(second) && first == 1 && second == 2,
          "Deux ReadU64 a la suite relisent 1 puis 2");
}

static void testBool() {
    std::printf("\n--- Bool ---\n");
    BinaryWriter writer;
    writer.WriteBool(true);
    writer.WriteBool(false);
    check(bytesEqual(writer.Bytes(), {0x01, 0x00}), "WriteBool ecrit 01 pour true et 00 pour false");

    BinaryReader reader(writer.Bytes());
    bool a = false, b = true;
    check(reader.ReadBool(a) && a == true, "ReadBool relit true");
    check(reader.ReadBool(b) && b == false, "ReadBool relit false");

    // Un octet 07 n'existe pas dans le format : fichier corrompu.
    auto corrupted = makeBuffer({0x07, 0x01});
    BinaryReader badReader(corrupted);
    bool value = false;
    check(!badReader.ReadBool(value), "ReadBool refuse l'octet 07");
    check(value == false, "ReadBool ne modifie pas la valeur en cas d'echec");
    std::uint8_t next = 0;
    check(badReader.ReadU8(next) && next == 0x07, "ReadBool remet le curseur en place apres un echec");
}

static void testF32() {
    std::printf("\n--- F32 ---\n");
    BinaryWriter writer;
    writer.WriteF32(1.0f);
    check(bytesEqual(writer.Bytes(), {0x00, 0x00, 0x80, 0x3F}), "WriteF32(1.0f) ecrit 00 00 80 3F (IEEE 754)");

    BinaryWriter values;
    values.WriteF32(3.14f);
    values.WriteF32(-0.5f);
    values.WriteF32(0.0f);
    BinaryReader reader(values.Bytes());
    float a = 0, b = 0, c = 1;
    check(reader.ReadF32(a) && a == 3.14f, "ReadF32 relit 3.14f exactement");
    check(reader.ReadF32(b) && b == -0.5f, "ReadF32 relit -0.5f");
    check(reader.ReadF32(c) && c == 0.0f, "ReadF32 relit 0.0f");
}

static void testString() {
    std::printf("\n--- String ---\n");
    BinaryWriter writer;
    writer.WriteU32(1);
    writer.WriteString("Hi");
    check(bytesEqual(writer.Bytes(), {0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x48, 0x69}),
          "U32(1) + String(\"Hi\") donne les 10 octets attendus");

    BinaryWriter texts;
    texts.WriteString("Player");
    texts.WriteString("");
    texts.WriteString("Zo\xC3\xA9"); // "Zoé" en UTF-8 : 4 octets pour 3 lettres
    BinaryReader reader(texts.Bytes());
    std::string a, b = "x", c;
    check(reader.ReadString(a) && a == "Player", "ReadString relit \"Player\"");
    check(reader.ReadString(b) && b.empty(), "ReadString relit une chaine vide");
    check(reader.ReadString(c) && c == "Zo\xC3\xA9" && c.size() == 4, "ReadString relit \"Zoe\" accentue (4 octets UTF-8)");

    // La longueur annonce 5 caracteres, mais il n'en reste que 2 : fichier tronque.
    auto truncated = makeBuffer({0x05, 0x00, 0x00, 0x00, 0x48, 0x69});
    BinaryReader shortReader(truncated);
    std::string untouched = "avant";
    check(!shortReader.ReadString(untouched), "ReadString refuse une chaine tronquee");
    check(untouched == "avant", "ReadString ne modifie pas la valeur en cas d'echec");
    std::uint32_t length = 0;
    check(shortReader.ReadU32(length) && length == 5, "ReadString remet le curseur en place apres un echec");

    // Une longueur absurde (environ 4 milliards) : refusee avant toute allocation.
    auto huge = makeBuffer({0xFF, 0xFF, 0xFF, 0xFF, 0x41});
    BinaryReader hugeReader(huge);
    std::string ignored;
    check(!hugeReader.ReadString(ignored), "ReadString refuse une longueur de 4 milliards");
}

static void testSequence() {
    std::printf("\n--- Enchainement de types ---\n");
    BinaryWriter writer;
    writer.WriteU32(42);
    writer.WriteString("Bob");
    writer.WriteBool(true);
    writer.WriteF32(75.5f);
    writer.WriteU64(123456789012ULL);

    BinaryReader reader(writer.Bytes());
    std::uint32_t id = 0;
    std::string name;
    bool alive = false;
    float mass = 0;
    std::uint64_t score = 0;
    const bool ok = reader.ReadU32(id) && reader.ReadString(name) && reader.ReadBool(alive) && reader.ReadF32(mass) &&
                    reader.ReadU64(score);
    check(ok && id == 42 && name == "Bob" && alive && mass == 75.5f && score == 123456789012ULL,
          "Plusieurs types a la suite sont relus dans le meme ordre");

    std::uint8_t extra = 0;
    check(!reader.ReadU8(extra), "Plus rien a lire apres le dernier champ");
}

static void testI32() {
    std::printf("\n--- I32 ---\n");
    BinaryWriter writer;
    writer.WriteI32(-1);
    writer.WriteI32(-123456);
    check(bytesEqual(std::span<const std::byte>(writer.Bytes()).subspan(0, 4), {0xFF, 0xFF, 0xFF, 0xFF}),
          "WriteI32(-1) ecrit FF FF FF FF (complement a deux)");
    BinaryReader reader(writer.Bytes());
    std::int32_t a = 0, b = 0;
    check(reader.ReadI32(a) && a == -1 && reader.ReadI32(b) && b == -123456, "ReadI32 relit -1 et -123456");
}

static void testVariableSequence() {
    std::printf("\n--- Sequences ---\n");
    auto writeOne = [](BinaryWriter &w, std::uint32_t v) { w.WriteU32(v); };
    auto readOne = [](BinaryReader &r, std::uint32_t &v) { return r.ReadU32(v); };

    BinaryWriter writer;
    WriteSequence(writer, std::vector<std::uint32_t>{10, 20, 30}, writeOne);
    WriteSequence(writer, std::vector<std::uint32_t>{}, writeOne);
    BinaryReader reader(writer.Bytes());
    std::vector<std::uint32_t> a, b{99};
    check(ReadSequence(reader, a, 100, 4, readOne) && a == std::vector<std::uint32_t>{10, 20, 30},
          "Sequence de 3 elements relue a l'identique");
    check(ReadSequence(reader, b, 100, 4, readOne) && b.empty(), "Sequence vide relue a l'identique");

    // Le nombre annonce (1 million) est impossible : il resterait seulement 4 octets.
    auto lying = makeBuffer({0x40, 0x42, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00});
    BinaryReader lyingReader(lying);
    std::vector<std::uint32_t> untouched{7};
    check(!ReadSequence(lyingReader, untouched, 2'000'000, 4, readOne), "Sequence refusee : nombre impossible");
    check(untouched == std::vector<std::uint32_t>{7}, "Sequence : la valeur n'est pas modifiee en cas d'echec");
    check(lyingReader.Position() == 0, "Sequence : le curseur revient en place");

    BinaryWriter many;
    WriteSequence(many, std::vector<std::uint32_t>{1, 2, 3}, writeOne);
    BinaryReader manyReader(many.Bytes());
    std::vector<std::uint32_t> limited;
    check(!ReadSequence(manyReader, limited, 2, 4, readOne), "Sequence refusee : plus que le maximum autorise");
}

static void testCrc() {
    std::printf("\n--- CRC-32 ---\n");
    const char *text = "123456789";
    std::vector<std::byte> data;
    for (const char *p = text; *p; ++p) {
        data.push_back(std::byte{static_cast<std::uint8_t>(*p)});
    }
    check(Crc32(data) == 0xCBF43926u, "CRC-32(\"123456789\") = 0xCBF43926 (valeur de reference)");
    check(Crc32({}) == 0u, "CRC-32 d'un buffer vide = 0");
    std::uint32_t state = Crc32Begin();
    state = Crc32Update(state, std::span<const std::byte>(data).subspan(0, 4));
    state = Crc32Update(state, std::span<const std::byte>(data).subspan(4));
    check(Crc32End(state) == 0xCBF43926u, "CRC-32 calcule en deux morceaux = meme resultat");
}

// --- outils pour fabriquer des archives abimees ---
static std::vector<std::byte> payloadOf(std::initializer_list<std::uint8_t> values) {
    std::vector<std::byte> out;
    for (std::uint8_t v : values) {
        out.push_back(std::byte{v});
    }
    return out;
}

static void patchU32(std::vector<std::byte> &bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t i = 0; i < 4; ++i) {
        bytes[offset + i] = std::byte{static_cast<std::uint8_t>(value >> (8 * i))};
    }
}

static void patchU64(std::vector<std::byte> &bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t i = 0; i < 8; ++i) {
        bytes[offset + i] = std::byte{static_cast<std::uint8_t>(value >> (8 * i))};
    }
}

// Recalcule un CRC valide : simule un fichier fabrique pour tromper le CRC,
// afin de verifier que les controles de structure le rejettent quand meme.
static void fixCrc(std::vector<std::byte> &bytes) { patchU32(bytes, archive::CrcOffset, archive::ComputeCrc(bytes)); }

static bool opens(std::vector<std::byte> bytes, std::string &error) {
    ArchiveReader reader;
    return reader.Open(std::move(bytes), error);
}

// Verifie qu'une archive est refusee, et affiche le diagnostic obtenu.
static void expectRejected(std::vector<std::byte> bytes, const char *label) {
    std::string error;
    const bool rejected = !opens(std::move(bytes), error);
    check(rejected, (std::string(label) + " -> \"" + error + "\"").c_str());
}

static std::vector<std::byte> twoAssetArchive() {
    ArchiveWriter writer;
    writer.AddRaw(42, 7, payloadOf({1, 2, 3}));
    writer.AddRaw(7, 9, payloadOf({4, 5}));
    return writer.Build();
}

static void testArchive() {
    std::printf("\n--- Archive ---\n");
    std::string error;

    // Archive vide
    ArchiveWriter empty;
    std::vector<std::byte> emptyBytes = empty.Build();
    check(emptyBytes.size() == archive::HeaderSize, "Archive vide = header seul (24 octets)");
    ArchiveReader emptyReader;
    check(emptyReader.Open(emptyBytes, error) && emptyReader.Entries().empty(), "Archive vide relue : 0 entree");

    // Plusieurs assets, liste et acces par ID
    std::vector<std::byte> bytes = twoAssetArchive();
    check(bytes.size() == 24 + 3 + 2 + 2 * 28, "Taille = header + payloads + 2 entrees de 28 octets");
    check(std::to_integer<char>(bytes[0]) == 'G' && std::to_integer<char>(bytes[3]) == 'T', "Le fichier commence par GAST");
    ArchiveReader reader;
    check(reader.Open(bytes, error), "Archive de 2 assets ouverte");
    check(reader.Entries().size() == 2 && reader.Entries()[0].id == 42 && reader.Entries()[1].id == 7,
          "Liste des entrees : ids 42 et 7");
    const AssetEntry *e42 = reader.Find(42);
    check(e42 && e42->type == 7 && e42->offset == 24 && e42->size == 3, "Entree 42 : type 7, offset 24, taille 3");
    check(e42 && bytesEqual(reader.Payload(*e42), {1, 2, 3}), "Payload de l'id 42 lu sans toucher aux autres");
    check(reader.Find(1234) == nullptr, "Un id absent n'est pas trouve");

    ArchiveWriter dup;
    check(dup.AddRaw(1, 1, {}) && !dup.AddRaw(1, 1, {}), "ArchiveWriter refuse un id en double");
    check(!dup.AddRaw(2, 0, {}), "ArchiveWriter refuse le type 0");

    // --- Rejets ---
    std::vector<std::byte> truncated(bytes.begin(), bytes.end() - 5);
    expectRejected(truncated, "Archive tronquee refusee");
    expectRejected(std::vector<std::byte>(bytes.begin(), bytes.begin() + 10), "Header incomplet refuse");

    std::vector<std::byte> altered = bytes;
    altered[25] ^= std::byte{0x01}; // un bit change dans un payload
    expectRejected(altered, "Octet altere refuse");

    std::vector<std::byte> badMagic = bytes;
    badMagic[0] = std::byte{'X'};
    expectRejected(badMagic, "Mauvais magic refuse");

    std::vector<std::byte> badVersion = bytes;
    patchU32(badVersion, 4, 99);
    fixCrc(badVersion);
    expectRejected(badVersion, "Version de format inconnue refusee");

    std::vector<std::byte> hugeCount = bytes;
    patchU32(hugeCount, 8, 0xFFFFFFFFu);
    fixCrc(hugeCount);
    expectRejected(hugeCount, "Nombre d'assets excessif refuse");

    std::vector<std::byte> badDirOffset = bytes;
    patchU64(badDirOffset, 12, 0xFFFFFFFFFFFFFFF0ull); // ferait deborder offset + taille
    fixCrc(badDirOffset);
    expectRejected(badDirOffset, "Offset du repertoire hors fichier refuse");

    const std::size_t dir = 24 + 3 + 2; // debut du repertoire
    std::vector<std::byte> badSize = bytes;
    patchU64(badSize, dir + 20, 1'000'000); // taille du premier payload
    fixCrc(badSize);
    expectRejected(badSize, "Taille de payload excessive refusee");

    std::vector<std::byte> duplicate = bytes;
    patchU64(duplicate, dir + 28, 42); // le 2e asset prend aussi l'id 42
    fixCrc(duplicate);
    expectRejected(duplicate, "Id en double refuse");

    std::vector<std::byte> zeroType = bytes;
    patchU32(zeroType, dir + 8, 0);
    fixCrc(zeroType);
    expectRejected(zeroType, "Type 0 refuse");

    ArchiveReader kept;
    kept.Open(bytes, error);
    check(!kept.Open(altered, error) && kept.Entries().empty(), "Un Open rate laisse un lecteur vide, pas a moitie charge");
}

static void testFiles() {
    std::printf("\n--- Fichiers ---\n");
    std::string error;
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "asset_tests_archive.gast";
    ArchiveWriter writer;
    writer.AddRaw(5, 1, payloadOf({9, 8, 7}));
    check(writer.SaveToFile(path, error), "Archive ecrite sur le disque");
    ArchiveReader reader;
    check(reader.OpenFile(path, error) && reader.Find(5) != nullptr, "Archive relue depuis le disque");
    check(!std::filesystem::exists(path.string() + ".tmp"), "Aucun fichier temporaire ne reste");
    std::filesystem::remove(path);

    const std::filesystem::path impossible =
        std::filesystem::temp_directory_path() / "dossier_qui_n_existe_pas_123" / "archive.gast";
    const bool writeFailed = !writer.SaveToFile(impossible, error);
    check(writeFailed, ("Echec d'ecriture detecte -> \"" + error + "\"").c_str());
    const bool openFailed = !reader.OpenFile(impossible, error);
    check(openFailed, ("Fichier absent detecte -> \"" + error + "\"").c_str());
}

int main() {
    testU8();
    testU32();
    testU64();
    testI32();
    testBool();
    testF32();
    testString();
    testSequence();
    testVariableSequence();
    testCrc();
    testArchive();
    testFiles();

    std::printf("\n%s (%d echec(s))\n", failures == 0 ? "TOUT EST BON" : "IL Y A DES ERREURS", failures);
    return failures == 0 ? 0 : 1;
}
