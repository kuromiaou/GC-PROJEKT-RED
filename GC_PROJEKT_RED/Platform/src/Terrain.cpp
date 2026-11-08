#include "editor/Terrain.h"
#include <cmath>

namespace
{
    float Hash(int x, int z)
    {
        unsigned int h = static_cast<unsigned int>(x) * 374761393u
            + static_cast<unsigned int>(z) * 668265263u;
        h = (h ^ (h >> 13)) * 1274126177u;
        h = h ^ (h >> 16);
        return static_cast<float>(h & 0xFFFF) / 65535.0f;
    }

    float ValueNoise(float x, float z)
    {
        const int x0 = static_cast<int>(std::floor(x));
        const int z0 = static_cast<int>(std::floor(z));

        // Position à l'intérieur de la case, entre 0 et 1
        float tx = x - static_cast<float>(x0);
        float tz = z - static_cast<float>(z0);

        // Hauteurs aléatoires aux quatre coins
        const float c00 = Hash(x0, z0);
        const float c10 = Hash(x0 + 1, z0);
        const float c01 = Hash(x0, z0 + 1);
        const float c11 = Hash(x0 + 1, z0 + 1);

        // Lissage (smoothstep) : évite les cassures aux coins
        tx = tx * tx * (3.0f - 2.0f * tx);
        tz = tz * tz * (3.0f - 2.0f * tz);

        // Mélange horizontal sur les deux lignes, puis mélange vertical
        const float bottom = c00 + tx * (c10 - c00);
        const float top = c01 + tx * (c11 - c01);
        return bottom + tz * (top - bottom);
    }

    float FractalNoise(float x, float z, int octaves)
    {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; i++) {
            total += ValueNoise(x * frequency, z * frequency) * amplitude;
            maxValue += amplitude;
            frequency *= 2.0f;
            amplitude *= 0.5f;
        }

        // Ramène le résultat entre 0 et 1
        return total / maxValue;
    }
}
const float textureRepeat = 2.0f;
Mesh GenerateTerrain(int size, float spacing)
{
    Mesh mesh;
    const int pointsPerRow = size + 1;
    const float halfWidth = size * spacing / 2.0f;

    for (int row = 0; row <= size; row++) {
        for (int col = 0; col <= size; col++) {
            const float z = row * spacing - halfWidth;
            const float x = col * spacing - halfWidth;
            const float y = FractalNoise(x * 0.3f, z * 0.3f, 4) * 1.5f;
            mesh.verts.emplace_back(x, y, z);

            const float u = static_cast<float>(col) / static_cast<float>(size) * textureRepeat;
            const float v = static_cast<float>(row) / static_cast<float>(size) * textureRepeat;
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