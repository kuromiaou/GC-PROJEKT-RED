#pragma once

#include <string>
#include <vector>

#include "Maths.h"
#include "Texture.h"

// A texture coordinate (OBJ 'vt'). v = 0 is the BOTTOM of the image.
struct UV
{
    float u = 0.0f;
    float v = 0.0f;
};

struct Mesh
{
    std::vector<Maths::Vec3f> verts;
    std::vector<UV>           uvs;
    std::vector<int>          faces;     // 3 vertex indices per face (0-based)
    std::vector<int>          faceUvs;   // 3 UV indices per face (0-based, or -1)

    int VertexCount() const { return static_cast<int>(verts.size()); }
    int FaceCount()   const { return static_cast<int>(faces.size() / 3); }

    bool Load(const std::string& path);
};
