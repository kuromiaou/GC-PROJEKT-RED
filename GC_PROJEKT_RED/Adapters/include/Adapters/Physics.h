#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "AssetEngine/BinaryReader.h"
#include "AssetEngine/BinaryWriter.h"
#include "AssetEngine/Sequence.h"

// Structures d'un monde physique minimal, et leur adaptateur d'archive.
// Notre moteur (rendu CPU + ECS) n'a pas encore de moteur physique complet : comme le
// sujet le permet, on démontre la sauvegarde et la restauration avec ces structures.
//
// Conventions (documentées dans le README) :
//   - unités SI : mètres, secondes, kilogrammes ; axe Y vers le haut ;
//   - orientation = quaternion unitaire (x, y, z, w) ;
//   - inertie = diagonale du tenseur d'inertie dans le repère local (Ixx, Iyy, Izz) ;
//   - un corps statique a une masse ignorée (0) et ne bouge pas.

namespace asset_types {
inline constexpr std::uint32_t PhysicsScene = 3;    // configuration initiale d'une scène
inline constexpr std::uint32_t PhysicsSnapshot = 4; // état à la fin d'un pas de simulation
} // namespace asset_types

struct Vec3 {
    float x = 0, y = 0, z = 0;
    bool operator==(const Vec3 &) const = default;
};

struct Quat {
    float x = 0, y = 0, z = 0, w = 1;
    bool operator==(const Quat &) const = default;
};

enum class ColliderType : std::uint8_t { None = 0, Sphere = 1, Box = 2 };

struct Collider {
    ColliderType type = ColliderType::None;
    float radius = 0.0f; // Sphere
    Vec3 halfExtents;    // Box
    bool operator==(const Collider &) const = default;
};

struct RigidBody {
    std::uint64_t id = 0; // ID persistant : c'est lui qu'on sauvegarde, jamais une adresse
    bool isStatic = false;
    Vec3 position;
    Quat orientation;
    float mass = 1.0f;
    Vec3 inertia{1, 1, 1};
    Vec3 linearVelocity;
    Vec3 angularVelocity;
    Collider collider;
    bool operator==(const RigidBody &) const = default;
};

// Une contrainte qui relie deux corps : la référence persistante est l'ID.
struct DistanceJoint {
    std::uint64_t bodyA = 0;
    std::uint64_t bodyB = 0;
    float restLength = 1.0f;
    // Données dérivées, jamais sauvegardées : reconstruites après le chargement.
    std::size_t indexA = 0;
    std::size_t indexB = 0;

    bool operator==(const DistanceJoint &o) const {
        return bodyA == o.bodyA && bodyB == o.bodyB && restLength == o.restLength;
    }
};

struct PhysicsWorld {
    Vec3 gravity{0.0f, -9.81f, 0.0f};
    float fixedTimestep = 1.0f / 60.0f;
    std::uint64_t stepIndex = 0;
    std::vector<RigidBody> bodies;
    std::vector<DistanceJoint> joints;

    // Données dérivées : ID -> position dans bodies.
    std::unordered_map<std::uint64_t, std::size_t> bodyIndex;

    bool operator==(const PhysicsWorld &o) const {
        return gravity == o.gravity && fixedTimestep == o.fixedTimestep && stepIndex == o.stepIndex &&
               bodies == o.bodies && joints == o.joints;
    }

    // Reconstruit l'index et les références des joints à partir des ID.
    // Renvoie false si un ID est en double ou si un joint désigne un corps inexistant.
    bool RebuildDerived() {
        std::unordered_map<std::uint64_t, std::size_t> index;
        for (std::size_t i = 0; i < bodies.size(); ++i) {
            if (!index.emplace(bodies[i].id, i).second) {
                return false;
            }
        }
        for (DistanceJoint &joint : joints) {
            auto a = index.find(joint.bodyA);
            auto b = index.find(joint.bodyB);
            if (a == index.end() || b == index.end() || joint.bodyA == joint.bodyB) {
                return false;
            }
            joint.indexA = a->second;
            joint.indexB = b->second;
        }
        bodyIndex = std::move(index);
        return true;
    }

    RigidBody *FindBody(std::uint64_t id) {
        auto it = bodyIndex.find(id);
        return it == bodyIndex.end() ? nullptr : &bodies[it->second];
    }

    // Un pas de simulation très simple (Euler semi-implicite, sans collisions) :
    // suffisant pour produire un état différent de l'état initial.
    void Step() {
        const float dt = fixedTimestep;
        for (RigidBody &body : bodies) {
            if (body.isStatic) {
                continue;
            }
            body.linearVelocity.x += gravity.x * dt;
            body.linearVelocity.y += gravity.y * dt;
            body.linearVelocity.z += gravity.z * dt;
            body.position.x += body.linearVelocity.x * dt;
            body.position.y += body.linearVelocity.y * dt;
            body.position.z += body.linearVelocity.z * dt;
        }
        ++stepIndex;
    }
};

// ---------------------------------------------------------------- encodage
// Payload : U32 version de schéma (1), gravity (3 × F32), fixedTimestep (F32), stepIndex (U64),
// séquence de corps, séquence de joints.
// Corps : id U64, isStatic Bool, position 3×F32, orientation 4×F32, mass F32, inertia 3×F32,
//         linearVelocity 3×F32, angularVelocity 3×F32, colliderType U8, radius F32,
//         halfExtents 3×F32  = 94 octets.
// Joint : bodyA U64, bodyB U64, restLength F32 = 20 octets.

inline constexpr std::uint32_t PhysicsSchemaV1 = 1;
inline constexpr std::uint32_t MaxBodies = 1'000'000;
inline constexpr std::uint32_t MaxJoints = 1'000'000;
inline constexpr std::size_t BodySize = 94;
inline constexpr std::size_t JointSize = 20;

namespace physics_detail {
inline void WriteVec3(BinaryWriter &w, const Vec3 &v) {
    w.WriteF32(v.x);
    w.WriteF32(v.y);
    w.WriteF32(v.z);
}
inline bool ReadFinite(BinaryReader &r, float &f) { return r.ReadF32(f) && std::isfinite(f); }
inline bool ReadVec3(BinaryReader &r, Vec3 &v) { return ReadFinite(r, v.x) && ReadFinite(r, v.y) && ReadFinite(r, v.z); }

inline void WriteBody(BinaryWriter &w, const RigidBody &b) {
    w.WriteU64(b.id);
    w.WriteBool(b.isStatic);
    WriteVec3(w, b.position);
    w.WriteF32(b.orientation.x);
    w.WriteF32(b.orientation.y);
    w.WriteF32(b.orientation.z);
    w.WriteF32(b.orientation.w);
    w.WriteF32(b.mass);
    WriteVec3(w, b.inertia);
    WriteVec3(w, b.linearVelocity);
    WriteVec3(w, b.angularVelocity);
    w.WriteU8(static_cast<std::uint8_t>(b.collider.type));
    w.WriteF32(b.collider.radius);
    WriteVec3(w, b.collider.halfExtents);
}

inline bool ReadBody(BinaryReader &r, RigidBody &b) {
    std::uint8_t colliderType = 0;
    const bool ok = r.ReadU64(b.id) && r.ReadBool(b.isStatic) && ReadVec3(r, b.position) &&
                    ReadFinite(r, b.orientation.x) && ReadFinite(r, b.orientation.y) &&
                    ReadFinite(r, b.orientation.z) && ReadFinite(r, b.orientation.w) && ReadFinite(r, b.mass) &&
                    ReadVec3(r, b.inertia) && ReadVec3(r, b.linearVelocity) && ReadVec3(r, b.angularVelocity) &&
                    r.ReadU8(colliderType) && ReadFinite(r, b.collider.radius) && ReadVec3(r, b.collider.halfExtents);
    if (!ok || colliderType > static_cast<std::uint8_t>(ColliderType::Box)) {
        return false;
    }
    b.collider.type = static_cast<ColliderType>(colliderType);
    // Règles de cohérence : un corps dynamique a une masse strictement positive.
    return b.isStatic || b.mass > 0.0f;
}
} // namespace physics_detail

inline void Write(BinaryWriter &writer, const PhysicsWorld &world) {
    using namespace physics_detail;
    writer.WriteU32(PhysicsSchemaV1);
    WriteVec3(writer, world.gravity);
    writer.WriteF32(world.fixedTimestep);
    writer.WriteU64(world.stepIndex);
    WriteSequence(writer, world.bodies, WriteBody);
    WriteSequence(writer, world.joints, [](BinaryWriter &w, const DistanceJoint &j) {
        w.WriteU64(j.bodyA);
        w.WriteU64(j.bodyB);
        w.WriteF32(j.restLength);
    });
}

// Décode dans un monde temporaire, valide tout, reconstruit les données dérivées,
// et ne remplace world qu'à la fin : jamais d'état partiellement chargé.
inline bool Read(BinaryReader &reader, PhysicsWorld &world) {
    using namespace physics_detail;
    const std::size_t start = reader.Position();
    PhysicsWorld temp;
    std::uint32_t schema = 0;
    bool ok = reader.ReadU32(schema) && schema == PhysicsSchemaV1 && ReadVec3(reader, temp.gravity) &&
              ReadFinite(reader, temp.fixedTimestep) && temp.fixedTimestep > 0.0f && reader.ReadU64(temp.stepIndex) &&
              ReadSequence(reader, temp.bodies, MaxBodies, BodySize, ReadBody) &&
              ReadSequence(reader, temp.joints, MaxJoints, JointSize, [](BinaryReader &r, DistanceJoint &j) {
                  return r.ReadU64(j.bodyA) && r.ReadU64(j.bodyB) && ReadFinite(r, j.restLength);
              });
    ok = ok && temp.RebuildDerived(); // ID uniques, références valides
    if (!ok) {
        reader.Rewind(start);
        return false;
    }
    world = std::move(temp);
    return true;
}
