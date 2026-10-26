#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

#include "Maths.h"
#include "Mesh.h"      // UV
#include "Texture.h"   // Texture
#include "Clip.h"      // ClipVertex

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

class Window
{
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;
    const Maths::Vec3f CUBE_VERTS[8] = {
    {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1},
    };

    const int CUBE_EDGES[12][3] = {
        {0,1,0}, {3,2,0}, {7,6,0}, {4,5,0},   // edges along x
        {0,3,1}, {1,2,1}, {5,6,1}, {4,7,1},   // edges along y
        {0,4,2}, {1,5,2}, {2,6,2}, {3,7,2},   // edges along z
    };

    Window();
    int TestWindow();

private:
    inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);
    inline void put_pixel(int x, int y, uint32_t color);
    inline void put_line(int x0, int y0, int x1, int y1, uint32_t color);
    inline int edge(int ax, int ay, int bx, int by, int px, int py);
    inline uint32_t shade(int e0, int e1, int e2, int wsum,
        uint32_t c0, uint32_t c1, uint32_t c2);
    inline void put_triangle(int x0, int y0, uint32_t c0,
        int x1, int y1, uint32_t c1,
        int x2, int y2, uint32_t c2);
    inline void put_circle(int cx, int cy, int r, uint32_t color);
    inline void clear(uint32_t color);
    inline void clear_zbuffer();
    inline void put_triangle_z(int x0, int y0, float z0,
        int x1, int y1, float z1,
        int x2, int y2, float z2,
        float zmin, float zrange, bool useZBuffer);
    inline void draw_triangle_textured(int x0, int y0, float z0, float iw0, UV uv0,
        int x1, int y1, float z1, float iw1, UV uv1,
        int x2, int y2, float z2, float iw2, UV uv2,
        const Texture* tex);

    /// Everything the per-pixel shader needs besides the triangle itself.
    struct ShadeParams
    {
        Maths::Vec3f L;          // unit vector surface -> light (world space)
        Maths::Vec3f H;          // Blinn-Phong half-vector = normalize(L + V)
        bool lighting = true;   // false: show the raw texture (chapter 9 look)
        bool bilinear = true;   // false: nearest-neighbour (chapter 9 look)
    };

    /// One unit normal per triangle (model space). Degenerate triangles get a
    /// zero normal instead of NaN.
    inline void compute_face_normals(const Mesh& mesh, std::vector<Maths::Vec3f>& out);
    /// One unit normal per VERTEX (model space): the normalised sum of the
    /// normals of every triangle touching it. This is what makes shading smooth.
    inline void compute_vertex_normals(const Mesh& mesh, const std::vector<Maths::Vec3f>& faceNormals,
        std::vector<Maths::Vec3f>& out);
    /// Phong-shaded textured triangle. Vertices arrive in screen space with
    /// depth, 1/w, UV and WORLD-space normal; normals and UVs are interpolated
    /// perspective-correctly, the normal is re-normalised per pixel, and
    /// Lambert diffuse + Blinn-Phong specular are evaluated per pixel.
    inline void draw_triangle_phong(
        int x0, int y0, float z0, float iw0, UV uv0, Maths::Vec3f n0,
        int x1, int y1, float z1, float iw1, UV uv1, Maths::Vec3f n1,
        int x2, int y2, float z2, float iw2, UV uv2, Maths::Vec3f n2,
        const Texture* tex, const ShadeParams& sp);
    /// Takes one clipper-output triangle (3 clip-space vertices, all in front of
    /// the near plane) through perspective divide, viewport, backface cull and
    /// the rasterizer.
    inline void project_and_draw(const ClipVertex* tri, const Texture* tex, const ShadeParams& sp);

    std::vector<uint32_t> framebuffer;
    std::vector<float>    zbuffer;
};