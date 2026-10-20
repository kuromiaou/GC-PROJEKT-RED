#pragma once
// Thin C-style convenience wrappers over Mesh::Load (see Mesh.cpp).
// The parsing lives in ONE place (Mesh::Load) - this header no longer
// duplicates it, and no longer uses the old chapter-6 C types
// (Vec3f3, vert_count, face_count) that don't exist in the C++ Mesh.
#include "Mesh.h"

// Returns 1 on success, 0 on failure (same contract as the chapter text).
inline int mesh_load(const char* path, Mesh* out)
{
    return out && out->Load(path) ? 1 : 0;
}

// Mesh owns std::vectors now, so "free" is just a reset.
inline void mesh_free(Mesh* m)
{
    if (!m) return;
    m->verts.clear();   m->verts.shrink_to_fit();
    m->uvs.clear();     m->uvs.shrink_to_fit();
    m->faces.clear();   m->faces.shrink_to_fit();
    m->faceUvs.clear(); m->faceUvs.shrink_to_fit();
}