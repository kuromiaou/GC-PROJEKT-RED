#pragma once
#include <vector>

#include "Maths.h"
#include "Platform/Mesh.h"
#include "Platform/Texture.h"

// Apparence d'un objet : sa texture et la façon dont il réagit à la lumière
struct Material
{
    const Texture* texture = nullptr;
    float ambient = 0.18f;            // lumière minimale, même dans l'ombre
    float specularStrength = 0.55f;   // intensité du reflet brillant (0 = mat)
    bool lighting = true;
    bool bilinear = true;
};

// Orientation d'un objet dans le monde
struct Transform
{
    Maths::Vec3f position;
    float rotationY = 0.0f;           // angle actuel, en radians
    float rotationSpeed = 0.0f;       // vitesse de rotation, en radians par seconde
};

// Ce qu'il faut pour dessiner un objet
struct MeshRenderer
{
    const Mesh* mesh = nullptr;
    std::vector<Maths::Vec3f> vertexNormals;
    Material material;
};