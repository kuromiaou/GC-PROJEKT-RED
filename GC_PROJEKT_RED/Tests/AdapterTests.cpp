// Tests des adaptateurs : deux types generaux (Player, Inventory) et la scene physique.
// Allers-retours, migration de schema, versions inconnues, references invalides,
// et restauration d'un etat physique sans etat partiellement charge.

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#include "Adapters/GameTypes.h"
#include "Adapters/Physics.h"
#include "AssetEngine/Archive.h"

static int failures = 0;

static void check(bool condition, const char *description) {
    if (condition) {
        std::printf("[OK]    %s\n", description);
    } else {
        std::printf("[ECHEC] %s\n", description);
        failures++;
    }
}

template <typename T> static bool roundTrip(const T &original, T &copy) {
    BinaryWriter writer;
    Write(writer, original);
    BinaryReader reader(writer.Bytes());
    return Read(reader, copy) && reader.Remaining() == 0;
}

static void testPlayer() {
    std::printf("\n--- Player ---\n");
    const Player bob{100, 5.5f, "Bob", 7};
    Player copy;
    check(roundTrip(bob, copy) && copy == bob, "Player : aller-retour identique");

    BinaryWriter oldWriter;
    Write(oldWriter, PlayerV1{80, 3.0f, "Alice"});
    BinaryReader oldReader(oldWriter.Bytes());
    Player migrated;
    check(Read(oldReader, migrated) && migrated.health == 80 && migrated.speed == 3.0f && migrated.name == "Alice",
          "Player V1 relu par le lecteur V2");
    check(migrated.level == PlayerDefaultLevel, "Migration V1 -> V2 : level = 1 (valeur par defaut)");

    BinaryWriter futureWriter;
    futureWriter.WriteU32(99); // version de schema inconnue
    futureWriter.WriteU32(100);
    BinaryReader futureReader(futureWriter.Bytes());
    Player untouched{1, 1.0f, "avant", 1};
    check(!Read(futureReader, untouched), "Player : version de schema 99 refusee");
    check(untouched.name == "avant" && futureReader.Position() == 0, "Player : rien n'est modifie apres le refus");
}

static void testInventory() {
    std::printf("\n--- Inventory ---\n");
    const Inventory inv{"Bob", {{1, 3}, {42, 1}, {7, 99}}};
    Inventory copy;
    check(roundTrip(inv, copy) && copy == inv, "Inventory : aller-retour avec chaine et collection");

    const Inventory empty{"", {}};
    Inventory emptyCopy{"x", {{1, 1}}};
    check(roundTrip(empty, emptyCopy) && emptyCopy == empty, "Inventory vide : aller-retour identique");

    BinaryWriter lying;
    lying.WriteString("Eve");
    lying.WriteU32(50'000); // annonce 50 000 objets, mais aucun ne suit
    BinaryReader lyingReader(lying.Bytes());
    Inventory untouched{"avant", {}};
    check(!Read(lyingReader, untouched) && untouched.owner == "avant", "Inventory : nombre d'objets impossible refuse");
}

static PhysicsWorld makeScene() {
    PhysicsWorld world;
    world.gravity = {0.0f, -9.81f, 0.0f};
    world.fixedTimestep = 1.0f / 60.0f;
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
    crate.orientation = {0.0f, 0.38268343f, 0.0f, 0.9238795f}; // 45 degres autour de Y
    crate.mass = 10.0f;
    crate.inertia = {1.6f, 1.6f, 1.6f};
    crate.angularVelocity = {0.0f, 1.0f, 0.0f};
    crate.collider = {ColliderType::Box, 0.0f, {0.5f, 0.5f, 0.5f}};
    world.bodies = {ground, ball, crate};
    world.joints = {{2, 3, 2.5f}};
    world.RebuildDerived();
    return world;
}

static void testPhysics() {
    std::printf("\n--- Physique ---\n");
    std::string error;
    PhysicsWorld scene = makeScene();

    PhysicsWorld copy;
    check(roundTrip(scene, copy) && copy == scene, "Scene : aller-retour identique (gravite, pas, corps, joints)");
    check(copy.FindBody(3) && copy.FindBody(3)->mass == 10.0f, "Scene : index ID -> corps reconstruit");
    check(copy.joints[0].indexA == 1 && copy.joints[0].indexB == 2, "Scene : references du joint reconstruites");

    // Snapshot a la fin d'un pas de simulation
    PhysicsWorld running = makeScene();
    for (int i = 0; i < 30; ++i) {
        running.Step();
    }
    ArchiveWriter writer;
    writer.Add(100, asset_types::PhysicsScene, scene);
    writer.Add(101, asset_types::PhysicsSnapshot, running);
    ArchiveReader archive;
    check(archive.Open(writer.Build(), error), "Archive scene + snapshot ouverte");

    PhysicsWorld emptyWorld;
    check(archive.Load(101, asset_types::PhysicsSnapshot, emptyWorld, error) && emptyWorld == running &&
              emptyWorld.stepIndex == 30,
          "Snapshot charge dans un monde vide : etat identique au pas 30");

    PhysicsWorld existing = makeScene();
    check(archive.Load(101, asset_types::PhysicsSnapshot, existing, error) && existing == running,
          "Snapshot charge a la place d'un monde existant");

    existing.Step();
    running.Step();
    check(existing == running, "La simulation reprend a l'identique apres restauration");

    PhysicsWorld wrongType;
    const bool wrongTypeRefused = !archive.Load(101, asset_types::PhysicsScene, wrongType, error);
    check(wrongTypeRefused, ("Mauvais type refuse -> \"" + error + "\"").c_str());

    // Reference invalide : un joint vers un corps qui n'existe pas
    PhysicsWorld broken = makeScene();
    broken.joints.push_back({2, 999, 1.0f});
    ArchiveWriter badWriter;
    badWriter.Add(1, asset_types::PhysicsScene, broken);
    ArchiveReader badArchive;
    badArchive.Open(badWriter.Build(), error);
    PhysicsWorld target = makeScene();
    target.stepIndex = 12345;
    const bool refused = !badArchive.Load(1, asset_types::PhysicsScene, target, error);
    check(refused, ("Joint vers un corps inexistant refuse -> \"" + error + "\"").c_str());
    check(target.stepIndex == 12345 && target.joints.size() == 1, "Le monde existant n'est pas modifie apres le refus");

    PhysicsWorld duplicateIds = makeScene();
    duplicateIds.bodies[2].id = 2;
    PhysicsWorld out;
    check(!roundTrip(duplicateIds, out), "Deux corps avec le meme ID refuses");

    PhysicsWorld massless = makeScene();
    massless.bodies[1].mass = 0.0f; // corps dynamique sans masse
    check(!roundTrip(massless, out), "Corps dynamique de masse nulle refuse");
}

static void testMixedArchive() {
    std::printf("\n--- Archive mixte ---\n");
    std::string error;
    ArchiveWriter writer;
    writer.Add(1, asset_types::Player, Player{100, 5.0f, "Bob", 3});
    writer.Add(2, asset_types::Inventory, Inventory{"Bob", {{10, 2}}});
    writer.Add(3, asset_types::Player, PlayerV1{50, 2.0f, "Old"});
    writer.Add(4, asset_types::PhysicsScene, makeScene());

    const std::filesystem::path path = std::filesystem::temp_directory_path() / "adapter_tests.gast";
    check(writer.SaveToFile(path, error), "Archive de 4 assets de 3 types ecrite");
    ArchiveReader reader;
    check(reader.OpenFile(path, error) && reader.Entries().size() == 4, "Archive relue : 4 entrees");

    Inventory inv;
    check(reader.Load(2, asset_types::Inventory, inv, error) && inv.items.size() == 1 && inv.items[0].count == 2,
          "Chargement de l'inventaire seul, par son ID");
    Player old;
    check(reader.Load(3, asset_types::Player, old, error) && old.name == "Old" && old.level == 1,
          "Player V1 dans l'archive : migre a la lecture");
    std::filesystem::remove(path);
}

int main() {
    testPlayer();
    testInventory();
    testPhysics();
    testMixedArchive();
    std::printf("\n%s (%d echec(s))\n", failures == 0 ? "TOUT EST BON" : "IL Y A DES ERREURS", failures);
    return failures == 0 ? 0 : 1;
}
