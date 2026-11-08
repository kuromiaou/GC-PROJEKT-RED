#pragma once
#include <vector>

#include "Maths.h"
#include "Platform/Mesh.h"
#include "Platform/Texture.h"


struct Material
{
    const Texture* texture = nullptr;
    float ambient = 0.18f;            
    float specularStrength = 0.55f;   
    bool lighting = true;
    bool bilinear = true;
};


struct Transform
{
    Maths::Vec3f position;
    float rotationY = 0.0f;           
    float rotationSpeed = 0.0f;       
};


struct MeshRenderer
{
    const Mesh* mesh = nullptr;
    std::vector<Maths::Vec3f> vertexNormals;
    Material material;
};