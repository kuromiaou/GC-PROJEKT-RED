// Démo de l'Asset Engine.
//   AssetDemo [dossier]   (par défaut : data)
// 1. Écrit une archive de types généraux, la relit, liste ses entrées, charge un asset par ID.
// 2. Écrit une archive contenant un Player en ancienne version (V1) et le relit en V2.
// 3. Sauvegarde une scène physique et un snapshot, puis les restaure.
// 4. Mesure taille, temps et mémoire temporaire pour deux volumes de données.

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "Adapters/GameTypes.h"
#include "Adapters/Physics.h"
#include "AssetEngine/Archive.h"

namespace fs = std::filesystem;

static const char *TypeName(std::uint32_t type) {
    switch (type) {
    case asset_types::Player: return "Player";
    case asset_types::Inventory: return "Inventory";
    case asset_types::PhysicsScene: return "PhysicsScene";
    case asset_types::PhysicsSnapshot: return "PhysicsSnapshot";
    default: return "?";
    }
}

static void ListEntries(const ArchiveReader &reader) {
    std::printf("  %-6s %-16s %10s %10s\n", "id", "type", "offset", "taille");
    for (const AssetEntry &e : reader.Entries()) {
        std::printf("  %-6llu %-16s %10llu %10llu\n", static_cast<unsigned long long>(e.id), TypeName(e.type),
                    static_cast<unsigned long long>(e.offset), static_cast<unsigned long long>(e.size));
    }
}

static bool Fail(const std::string &what, const std::string &error) {
    std::printf("ERREUR %s : %s\n", what.c_str(), error.c_str());
    return false;
}

static bool DemoGeneral(const fs::path &dir) {
    std::printf("\n=== 1. Types generaux ===\n");
    std::string error;
    ArchiveWriter writer;
    writer.Add(1, asset_types::Player, Player{100, 5.5f, "Bob", 7});
    writer.Add(2, asset_types::Player, Player{80, 4.0f, "Zo\xC3\xA9", 2});
    writer.Add(10, asset_types::Inventory, Inventory{"Bob", {{1, 3}, {42, 1}, {7, 99}}});
    writer.Add(11, asset_types::Inventory, Inventory{"Zo\xC3\xA9", {}});
    const fs::path path = dir / "general_v2.gast";
    if (!writer.SaveToFile(path, error)) return Fail("ecriture", error);
    std::printf("Ecrit : %s (%llu octets)\n", path.string().c_str(), static_cast<unsigned long long>(fs::file_size(path)));

    ArchiveReader reader;
    if (!reader.OpenFile(path, error)) return Fail("ouverture", error);
    ListEntries(reader);

    Inventory inv;
    if (!reader.Load(10, asset_types::Inventory, inv, error)) return Fail("chargement id 10", error);
    std::printf("Charge l'id 10 seul : inventaire de %s, %zu objets\n", inv.owner.c_str(), inv.items.size());
    return true;
}

static bool DemoMigration(const fs::path &dir) {
    std::printf("\n=== 2. Versions de schema ===\n");
    std::string error;
    ArchiveWriter writer;
    writer.Add(1, asset_types::Player, PlayerV1{60, 3.0f, "Ancien"});
    const fs::path path = dir / "general_v1.gast";
    if (!writer.SaveToFile(path, error)) return Fail("ecriture", error);

    ArchiveReader reader;
    Player player;
    if (!reader.OpenFile(path, error) || !reader.Load(1, asset_types::Player, player, error))
        return Fail("migration", error);
    std::printf("Ecrit : %s (Player en schema V1)\n", path.string().c_str());
    std::printf("Relu en V2 : %s, health %u, level %u (valeur par defaut)\n", player.name.c_str(), player.health,
                player.level);

    BinaryWriter future;
    future.WriteU32(99);
    ArchiveWriter futureArchive;
    futureArchive.AddRaw(1, asset_types::Player, future.TakeBytes());
    ArchiveReader futureReader;
    futureReader.Open(futureArchive.Build(), error);
    if (!futureReader.Load(1, asset_types::Player, player, error)) {
        std::printf("Schema 99 refuse : %s\n", error.c_str());
    }
    return true;
}

static PhysicsWorld MakeScene() {
    PhysicsWorld world;
    RigidBody ground;
    ground.id = 1;
    ground.isStatic = true;
    ground.mass = 0.0f;
    ground.collider = {ColliderType::Box, 0.0f, {50.0f, 0.5f, 50.0f}};
    RigidBody ball;
    ball.id = 2;
    ball.position = {0.0f, 10.0f, 0.0f};
    ball.mass = 2.0f;
    ball.inertia = {0.8f, 0.8f, 0.8f};
    ball.linearVelocity = {1.0f, 0.0f, 0.0f};
    ball.collider = {ColliderType::Sphere, 0.5f, {}};
    RigidBody crate;
    crate.id = 3;
    crate.position = {2.0f, 5.0f, 0.0f};
    crate.orientation = {0.0f, 0.38268343f, 0.0f, 0.9238795f};
    crate.mass = 10.0f;
    crate.inertia = {1.6f, 1.6f, 1.6f};
    crate.collider = {ColliderType::Box, 0.0f, {0.5f, 0.5f, 0.5f}};
    world.bodies = {ground, ball, crate};
    world.joints = {{2, 3, 2.5f}};
    world.RebuildDerived();
    return world;
}

static bool DemoPhysics(const fs::path &dir) {
    std::printf("\n=== 3. Scene physique ===\n");
    std::string error;
    PhysicsWorld world = MakeScene();
    ArchiveWriter writer;
    writer.Add(100, asset_types::PhysicsScene, world);
    for (int i = 0; i < 30; ++i) world.Step();
    writer.Add(101, asset_types::PhysicsSnapshot, world);
    const fs::path path = dir / "physics_scene.gast";
    if (!writer.SaveToFile(path, error)) return Fail("ecriture", error);
    std::printf("Ecrit : %s (scene initiale + snapshot au pas 30)\n", path.string().c_str());

    ArchiveReader reader;
    PhysicsWorld restored;
    if (!reader.OpenFile(path, error) || !reader.Load(101, asset_types::PhysicsSnapshot, restored, error))
        return Fail("restauration", error);
    const RigidBody *ball = restored.FindBody(2);
    std::printf("Restaure dans un monde vide : pas %llu, %zu corps, balle en y = %.3f\n",
                static_cast<unsigned long long>(restored.stepIndex), restored.bodies.size(), ball ? ball->position.y : 0.0f);
    std::printf("Identique a l'etat sauvegarde : %s\n", restored == world ? "oui" : "NON");
    return true;
}

// ------------------------------------------------------------- mesures
struct Measure {
    std::size_t assets = 0;
    std::uint64_t fileBytes = 0;
    double saveMs = 0, loadMs = 0;
    std::uint64_t peakTempSave = 0, peakTempLoad = 0;
};

static Measure MeasureVolume(const fs::path &dir, std::size_t inventoryCount, int runs) {
    std::vector<Inventory> data(inventoryCount);
    for (std::size_t i = 0; i < inventoryCount; ++i) {
        data[i].owner = "Owner" + std::to_string(i);
        for (std::uint32_t k = 0; k < 10; ++k) data[i].items.push_back({k, static_cast<std::uint32_t>(i + k)});
    }
    const fs::path path = dir / ("bench_" + std::to_string(inventoryCount) + ".gast");
    Measure m;
    m.assets = inventoryCount;
    std::string error;
    using clock = std::chrono::steady_clock;

    for (int run = 0; run < runs; ++run) {
        auto t0 = clock::now();
        ArchiveWriter writer;
        std::uint64_t payloadBytes = 0;
        for (std::size_t i = 0; i < data.size(); ++i) writer.Add(i + 1, asset_types::Inventory, data[i]);
        std::vector<std::byte> bytes = writer.Build();
        for (const auto &d : data) payloadBytes += 4 + d.owner.size() + 4 + d.items.size() * ItemSize;
        SaveFileBytes(path, bytes, error);
        auto t1 = clock::now();

        ArchiveReader reader;
        reader.OpenFile(path, error);
        std::vector<Inventory> loaded(reader.Entries().size());
        for (std::size_t i = 0; i < loaded.size(); ++i) reader.Load(i + 1, asset_types::Inventory, loaded[i], error);
        auto t2 = clock::now();

        m.saveMs += std::chrono::duration<double, std::milli>(t1 - t0).count();
        m.loadMs += std::chrono::duration<double, std::milli>(t2 - t1).count();
        m.fileBytes = bytes.size();
        // Mémoire temporaire estimée : payloads gardés par l'ArchiveWriter + buffer final.
        m.peakTempSave = payloadBytes + bytes.size();
        // Au chargement : fichier entier en mémoire + répertoire (vector + table de hachage, estimée).
        m.peakTempLoad = bytes.size() + reader.Entries().size() * (sizeof(AssetEntry) + 32);
    }
    m.saveMs /= runs;
    m.loadMs /= runs;
    fs::remove(path);
    return m;
}

static void DemoMeasures(const fs::path &dir) {
    std::printf("\n=== 4. Mesures (moyenne de 5 executions) ===\n");
    std::printf("%10s %14s %12s %12s %16s %16s\n", "assets", "taille (o)", "save (ms)", "load (ms)", "temp save (o)",
                "temp load (o)");
    for (std::size_t count : {1'000u, 100'000u}) {
        const Measure m = MeasureVolume(dir, count, 5);
        std::printf("%10zu %14llu %12.2f %12.2f %16llu %16llu\n", m.assets, static_cast<unsigned long long>(m.fileBytes),
                    m.saveMs, m.loadMs, static_cast<unsigned long long>(m.peakTempSave),
                    static_cast<unsigned long long>(m.peakTempLoad));
    }
    std::printf("Chaque asset : un Inventory de 10 objets. save = serialiser + construire + ecrire ;\n"
                "load = lire le fichier + valider + decoder tous les assets.\n");
}

int main(int argc, char **argv) {
    const fs::path dir = argc > 1 ? fs::path(argv[1]) : fs::path("data");
    std::error_code ec;
    fs::create_directories(dir, ec);
    bool ok = DemoGeneral(dir) && DemoMigration(dir) && DemoPhysics(dir);
    DemoMeasures(dir);
    return ok ? 0 : 1;
}
