#include "editor/Terrain.h"

Mesh GenerateTerrain(int size, float spacing)
{
    Mesh mesh;
    const int pointsPerRow = size + 1;
    const float halfWidth = size * spacing / 2.0f;

    for (int row = 0; row <= size; row++) {
        for (int col = 0; col <= size; col++) {
            const float x = col * spacing - halfWidth;
            const float y = 0.0f;
            const float z = row * spacing - halfWidth;
            mesh.verts.emplace_back(x, y, z);

            const float u = static_cast<float>(col) / static_cast<float>(size);
            const float v = static_cast<float>(row) / static_cast<float>(size);
            mesh.uvs.push_back({ u, v });
        }
    }
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            const int i = row * pointsPerRow + col;

            mesh.faces.push_back(i);
            mesh.faces.push_back(i + pointsPerRow);
            mesh.faces.push_back(i + pointsPerRow + 1);

            mesh.faces.push_back(i);
            mesh.faces.push_back(i + pointsPerRow + 1);
            mesh.faces.push_back(i + 1);
        }
    }

    mesh.faceUvs = mesh.faces;

    return mesh;
}