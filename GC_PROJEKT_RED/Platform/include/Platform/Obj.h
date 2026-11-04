#pragma once
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