#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <thread>
#include <semaphore>
#include <array>

#include "Maths.h"
#include "Mesh.h"      // UV
#include "Texture.h"   // Texture
#include "Clip.h"      // ClipVertex

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

class ParticleSystem; //fwd declaration

class Window
{
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    Window();
    int TestWindow();

private:
    inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);
    inline int edge(int ax, int ay, int bx, int by, int px, int py);
    inline uint32_t shade(int e0, int e1, int e2, int wsum,
        uint32_t c0, uint32_t c1, uint32_t c2);
    inline void clear(uint32_t color);
    inline void clear_zbuffer();
    inline void put_triangle_textured(int x0, int y0, float z0, float iw0, UV uv0,
        int x1, int y1, float z1, float iw1, UV uv1,
        int x2, int y2, float z2, float iw2, UV uv2,
        const Texture* tex);

    struct ShadeParams
    {
        Maths::Vec3f L;
        Maths::Vec3f H;
        bool lighting = true;
        bool bilinear = true;
    };

    inline void compute_face_normals(const Mesh& mesh, std::vector<Maths::Vec3f>& out);
    inline void compute_vertex_normals(const Mesh& mesh, const std::vector<Maths::Vec3f>& faceNormals,
        std::vector<Maths::Vec3f>& out);
    inline void put_triangle_phong(
        int x0, int y0, float z0, float iw0, UV uv0, Maths::Vec3f n0,
        int x1, int y1, float z1, float iw1, UV uv1, Maths::Vec3f n1,
        int x2, int y2, float z2, float iw2, UV uv2, Maths::Vec3f n2,
        const Texture* tex, const ShadeParams& sp);

    inline void project_and_put(const ClipVertex* tri, const Texture* tex, const ShadeParams& sp);

    void put_particles(const ParticleSystem& ps, const Maths::Matrix4x4& viewProj);

    std::vector<uint32_t> framebuffer;
    std::vector<float>    zbuffer;

    struct JobVertex { int x, y; float z, iw; UV uv; Maths::Vec3f n; };

    struct TriangleJob {
        JobVertex v[3];
        const Texture texture;
        const ShadeParams shadeParams;
    };

    std::array<std::binary_semaphore, 4> m_workerStart{
           std::binary_semaphore{0}, std::binary_semaphore{0},
           std::binary_semaphore{0}, std::binary_semaphore{0}
    };
    std::counting_semaphore<4> m_endSemaphore{ 0 };

    std::vector<TriangleJob> m_jobs; //triangle temp

    void draw_mesh(const Mesh& mesh, const std::vector<Maths::Vec3f>& vertexNormals,
        const Maths::Matrix4x4& model, const Maths::Matrix4x4& viewProjection,
        const Texture* tex, const ShadeParams& sp);
};