#include "Platform/Window.h"
#include "Platform/Obj.h"
#include "Platform/Mesh.h"
#include "Platform/Texture.h"
#include "Platform/Clip.h"
#include "Platform/ParticleManager.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>

#include <cmath>
#include <cstdio>
#include <string>

#ifndef WARP_ASSET_DIR
#define WARP_ASSET_DIR "assets"
#endif

#define AMBIENT 0.18f

#define SPEC_STRENGTH 0.55f

namespace
{
    inline float pow32(float x)
    {
        const float x2 = x * x, x4 = x2 * x2, x8 = x4 * x4, x16 = x8 * x8;
        return x16 * x16;
    }

    inline uint32_t color_phong(uint32_t texel, float diffuse_k, float spec_k)
    {
        const float add = 255.0f * spec_k;
        const float r = std::min(255.0f, static_cast<float>((texel >> 16) & 0xFF) * diffuse_k + add);
        const float g = std::min(255.0f, static_cast<float>((texel >> 8) & 0xFF) * diffuse_k + add);
        const float b = std::min(255.0f, static_cast<float>(texel & 0xFF) * diffuse_k + add);
        return (0xFFu << 24) | (static_cast<uint32_t>(r) << 16)
            | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
    }
}

Window::Window()
    : framebuffer(static_cast<size_t>(WIDTH)* HEIGHT)
    , zbuffer(static_cast<size_t>(WIDTH)* HEIGHT)
{}
inline uint32_t Window::rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (0xFFu << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}
inline void Window::put_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    framebuffer[static_cast<size_t>(y) * WIDTH + x] = color;
}
inline void Window::put_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);             // negative on purpose
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;                   // = dx - |dy|

    for (;;) {
        put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }   // step in x
        if (e2 <= dx) { err += dx; y0 += sy; }   // step in y
    }
}
inline int Window::edge(int ax, int ay, int bx, int by, int px, int py) {
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}
inline uint32_t Window::shade(int e0, int e1, int e2, int wsum,
    uint32_t c0, uint32_t c1, uint32_t c2) {
    uint8_t r = (uint8_t)((e0 * ((c0 >> 16) & 0xFF)
        + e1 * ((c1 >> 16) & 0xFF)
        + e2 * ((c2 >> 16) & 0xFF)) / wsum);
    uint8_t g = (uint8_t)((e0 * ((c0 >> 8) & 0xFF)
        + e1 * ((c1 >> 8) & 0xFF)
        + e2 * ((c2 >> 8) & 0xFF)) / wsum);
    uint8_t b = (uint8_t)((e0 * (c0 & 0xFF)
        + e1 * (c1 & 0xFF)
        + e2 * (c2 & 0xFF)) / wsum);
    return rgb(r, g, b);
}
inline void Window::put_triangle(int x0, int y0, uint32_t c0,
    int x1, int y1, uint32_t c1,
    int x2, int y2, uint32_t c2) {
    int area = edge(x0, y0, x1, y1, x2, y2);
    if (area == 0) return;
    if (area < 0) {
        int tx = x1; int ty = y1; uint32_t tc = c1;
        x1 = x2; y1 = y2; c1 = c2;
        x2 = tx; y2 = ty; c2 = tc;
        area = -area;
    }

    int minx = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int miny = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int maxx = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int maxy = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);
    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if (maxx >= WIDTH)  maxx = WIDTH - 1;
    if (maxy >= HEIGHT) maxy = HEIGHT - 1;

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            int e0 = edge(x1, y1, x2, y2, x, y);
            int e1 = edge(x2, y2, x0, y0, x, y);
            int e2 = edge(x0, y0, x1, y1, x, y);
            if ((e0 | e1 | e2) >= 0) {
                framebuffer[static_cast<size_t>(y) * WIDTH + x] =
                    shade(e0, e1, e2, area, c0, c1, c2);
            }
        }
    }
}

void Window::put_circle(int cx, int cy, int r, uint32_t color) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    while (x <= y) {
        put_pixel(cx + x, cy + y, color);
        put_pixel(cx - x, cy + y, color);
        put_pixel(cx + x, cy - y, color);
        put_pixel(cx - x, cy - y, color);
        put_pixel(cx + y, cy + x, color);
        put_pixel(cx - y, cy + x, color);
        put_pixel(cx + y, cy - x, color);
        put_pixel(cx - y, cy - x, color);
        x++;
        if (d > 0) { y--; d += 4 * (x - y) + 10; }
        else { d += 4 * x + 6; }
    }
}

void Window::clear(uint32_t color) {
    std::fill(framebuffer.begin(), framebuffer.end(), color);
}

void Window::clear_zbuffer() {
    std::fill(zbuffer.begin(), zbuffer.end(), 1e30f);
}

void Window::put_triangle_z(int x0, int y0, float z0,
    int x1, int y1, float z1,
    int x2, int y2, float z2,
    float zmin, float zrange, bool useZBuffer) {
    int area = edge(x0, y0, x1, y1, x2, y2);
    if (area == 0) return;


    if (area < 0) {
        std::swap(x1, x2); std::swap(y1, y2); std::swap(z1, z2);
        area = -area;
    }

    const int minx = std::clamp(std::min({ x0, x1, x2 }), 0, WIDTH - 1);
    const int miny = std::clamp(std::min({ y0, y1, y2 }), 0, HEIGHT - 1);
    const int maxx = std::clamp(std::max({ x0, x1, x2 }), 0, WIDTH - 1);
    const int maxy = std::clamp(std::max({ y0, y1, y2 }), 0, HEIGHT - 1);

    const float invArea = 1.0f / static_cast<float>(area);

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            const int e0 = edge(x1, y1, x2, y2, x, y);
            const int e1 = edge(x2, y2, x0, y0, x, y);
            const int e2 = edge(x0, y0, x1, y1, x, y);
            if ((e0 | e1 | e2) < 0) continue;
            const float z = (e0 * z0 + e1 * z1 + e2 * z2) * invArea;

            const size_t idx = static_cast<size_t>(y) * WIDTH + x;
            if (useZBuffer) {
                if (z >= zbuffer[idx]) continue;
                zbuffer[idx] = z;
            }
            const float far01 = (z - zmin) / zrange;
            const int level = std::clamp(240 - static_cast<int>(far01 * 200.0f), 0, 255);
            framebuffer[idx] = rgb((uint8_t)level, (uint8_t)level, (uint8_t)level);
        }
    }
}
void Window::compute_face_normals(const Mesh& mesh, std::vector<Maths::Vec3f>& out)
{
    const int faceCount = mesh.FaceCount();
    out.assign(static_cast<size_t>(faceCount), Maths::Vec3f());      // zero vectors
    const int nv = mesh.VertexCount();
    for (int f = 0; f < faceCount; f++) {
        const int ia = mesh.faces[f * 3 + 0];
        const int ib = mesh.faces[f * 3 + 1];
        const int ic = mesh.faces[f * 3 + 2];
        if (ia < 0 || ib < 0 || ic < 0 || ia >= nv || ib >= nv || ic >= nv) continue;  // bad OBJ index

        const Maths::Vec3f n = (mesh.verts[ib] - mesh.verts[ia]).Cross(mesh.verts[ic] - mesh.verts[ia]);
        const float lenSq = n.MagnitudeSquared();
        if (lenSq > 1e-20f)
            out[static_cast<size_t>(f)] = n * (1.0f / std::sqrt(lenSq));
    }
}

void Window::compute_vertex_normals(const Mesh& mesh, const std::vector<Maths::Vec3f>& faceNormals,
    std::vector<Maths::Vec3f>& out)
{
    const int nv = mesh.VertexCount();
    out.assign(static_cast<size_t>(nv), Maths::Vec3f());
    for (int f = 0; f < mesh.FaceCount(); f++) {
        for (int k = 0; k < 3; k++) {
            const int v = mesh.faces[f * 3 + k];
            if (v < 0 || v >= nv) continue;
            out[static_cast<size_t>(v)] += faceNormals[static_cast<size_t>(f)];
        }
    }
    for (auto& n : out) {
        const float lenSq = n.MagnitudeSquared();
        n = (lenSq > 1e-20f) ? n * (1.0f / std::sqrt(lenSq)) : Maths::Vec3f();
    }
}

void Window::put_triangle_phong(
    int x0, int y0, float z0, float iw0, UV uv0, Maths::Vec3f n0,
    int x1, int y1, float z1, float iw1, UV uv1, Maths::Vec3f n1,
    int x2, int y2, float z2, float iw2, UV uv2, Maths::Vec3f n2,
    const Texture* tex, const ShadeParams& sp)
{
    int area = edge(x0, y0, x1, y1, x2, y2);
    if (area == 0) return;

    if (area < 0) {
        std::swap(x1, x2); std::swap(y1, y2); std::swap(z1, z2);
        std::swap(iw1, iw2); std::swap(uv1, uv2); std::swap(n1, n2);
        area = -area;
    }

    const int minx = std::clamp(std::min({ x0, x1, x2 }), 0, WIDTH - 1);
    const int miny = std::clamp(std::min({ y0, y1, y2 }), 0, HEIGHT - 1);
    const int maxx = std::clamp(std::max({ x0, x1, x2 }), 0, WIDTH - 1);
    const int maxy = std::clamp(std::max({ y0, y1, y2 }), 0, HEIGHT - 1);

    const float inv_area = 1.0f / static_cast<float>(area);

    const float u0w = uv0.u * iw0, v0w = uv0.v * iw0;
    const float u1w = uv1.u * iw1, v1w = uv1.v * iw1;
    const float u2w = uv2.u * iw2, v2w = uv2.v * iw2;
    const float n0x = n0.X() * iw0, n0y = n0.Y() * iw0, n0z = n0.Z() * iw0;
    const float n1x = n1.X() * iw1, n1y = n1.Y() * iw1, n1z = n1.Z() * iw1;
    const float n2x = n2.X() * iw2, n2y = n2.Y() * iw2, n2z = n2.Z() * iw2;

    const float Lx = sp.L.X(), Ly = sp.L.Y(), Lz = sp.L.Z();
    const float Hx = sp.H.X(), Hy = sp.H.Y(), Hz = sp.H.Z();

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            const int e0 = edge(x1, y1, x2, y2, x, y);
            const int e1 = edge(x2, y2, x0, y0, x, y);
            const int e2 = edge(x0, y0, x1, y1, x, y);
            if ((e0 | e1 | e2) < 0) continue;

            const float l0 = e0 * inv_area;
            const float l1 = e1 * inv_area;
            const float l2 = e2 * inv_area;

            const float z = l0 * z0 + l1 * z1 + l2 * z2;     // linear on screen (chapter 7)
            const size_t idx = static_cast<size_t>(y) * WIDTH + x;
            if (z >= zbuffer[idx]) continue;

            const float iw = l0 * iw0 + l1 * iw1 + l2 * iw2;
            const float inv_iw = 1.0f / iw;
            const float u = (l0 * u0w + l1 * u1w + l2 * u2w) * inv_iw;
            const float v = (l0 * v0w + l1 * v1w + l2 * v2w) * inv_iw;

            const uint32_t texel = sp.bilinear ? tex->SampleBilinear(u, v) : tex->Sample(u, v);

            float diffuse_k = 1.0f, spec_k = 0.0f;
            if (sp.lighting) {
                // Perspective-correct interpolated normal, then RE-NORMALISE:
                // blending unit vectors gives a vector shorter than 1.
                float Nx = (l0 * n0x + l1 * n1x + l2 * n2x) * inv_iw;
                float Ny = (l0 * n0y + l1 * n1y + l2 * n2y) * inv_iw;
                float Nz = (l0 * n0z + l1 * n1z + l2 * n2z) * inv_iw;
                const float lenSq = Nx * Nx + Ny * Ny + Nz * Nz;
                if (lenSq > 1e-12f) {
                    const float k = 1.0f / std::sqrt(lenSq);
                    Nx *= k; Ny *= k; Nz *= k;
                }
                else {
                    Nx = Ny = Nz = 0.0f;      // normals cancelled out: ambient only
                }

                const float ndotl = std::max(0.0f, Nx * Lx + Ny * Ly + Nz * Lz);
                diffuse_k = AMBIENT + (1.0f - AMBIENT) * ndotl;

                const float ndoth = std::max(0.0f, Nx * Hx + Ny * Hy + Nz * Hz);
                spec_k = pow32(ndoth) * SPEC_STRENGTH;
            }

            zbuffer[idx] = z;
            framebuffer[idx] = color_phong(texel, diffuse_k, spec_k);
        }
    }
}

void Window::project_and_put(const ClipVertex* tri, const Texture* tex, const ShadeParams& sp)
{
    int   sx[3], sy[3];
    float nz[3], iw[3];
    for (int k = 0; k < 3; k++) {
        const float inv = 1.0f / tri[k].w;     // w > 0 guaranteed: the clipper kept only the front side
        sx[k] = static_cast<int>((tri[k].x * inv + 1.0f) * 0.5f * WIDTH);
        sy[k] = static_cast<int>((1.0f - (tri[k].y * inv + 1.0f) * 0.5f) * HEIGHT);   // screen Y points down
        nz[k] = tri[k].z * inv;
        iw[k] = inv;
    }

    if (edge(sx[0], sy[0], sx[1], sy[1], sx[2], sy[2]) >= 0) return;

    put_triangle_phong(
        sx[0], sy[0], nz[0], iw[0], UV{ tri[0].u, tri[0].v }, Maths::Vec3f(tri[0].nx, tri[0].ny, tri[0].nz),
        sx[1], sy[1], nz[1], iw[1], UV{ tri[1].u, tri[1].v }, Maths::Vec3f(tri[1].nx, tri[1].ny, tri[1].nz),
        sx[2], sy[2], nz[2], iw[2], UV{ tri[2].u, tri[2].v }, Maths::Vec3f(tri[2].nx, tri[2].ny, tri[2].nz),
        tex, sp);
}

void Window::put_particles(const ParticleSystem& ps, const Maths::Matrix4x4& viewProj)
{
    const int half = 1;
    auto to8 = [](float f) { return static_cast<uint8_t>(std::clamp(f, 0.0f, 1.0f) * 255.0f); };

    for (const Particle& p : ps.particles) {
        const Maths::Vec4f c4 = viewProj * Maths::Vec4f(p.position[0], p.position[1], p.position[2], 1.0f);
        float c[4];
        _mm_storeu_ps(c, c4.reg);             // c = x, y, z, w
        const float w = c[3];

        //if (c[0] < -w || c[0] > w || c[1] < -w || c[1] > w || c[2] < -w || c[2] > w) continue;

        const float inv = 1.0f / w;
        const int   sx  = static_cast<int>((c[0] * inv + 1.0f) * 0.5f * WIDTH);
        const int   sy  = static_cast<int>((1.0f - (c[1] * inv + 1.0f) * 0.5f) * HEIGHT);
        const float z   = c[2] * inv;

        float fade = 1.0f;
        if (p.initialLifetime > 0.0f)
            fade = std::clamp(p.lifetime / p.initialLifetime, 0.0f, 1.0f);
        const uint32_t col = rgb(to8(p.color[0] * fade), to8(p.color[1] * fade), to8(p.color[2] * fade));

        const int x0 = std::max(sx - half, 0), x1 = std::min(sx + half, WIDTH - 1);
        const int y0 = std::max(sy - half, 0), y1 = std::min(sy + half, HEIGHT - 1);
        for (int y = y0; y <= y1; y++) {
            for (int x = x0; x <= x1; x++) {
                const size_t idx = static_cast<size_t>(y) * WIDTH + x;
                if (z >= zbuffer[idx]) continue;
                framebuffer[idx] = col;
            }
        }
    }
}

int Window::TestWindow()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Particle System", WIDTH, HEIGHT, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (!texture) {
        SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // ---------------------------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------------------------

    Mesh mesh;
    if (!mesh.Load(std::string(WARP_ASSET_DIR) + "/spot.obj")) {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::printf("loaded spot.obj: %d vertices, %d faces\n", mesh.VertexCount(), mesh.FaceCount());

    const float aspect = (float)WIDTH / (float)HEIGHT;
    const float zNear = 0.1f;

    const Maths::Matrix4x4 projection = Maths::Matrix4x4::perspective(
        Maths::Matrix4x4::radians(50.0f), aspect, zNear, 100.0f);

    const size_t nv = mesh.verts.size();
    std::vector<ClipVertex> clipVerts(nv);

    Texture tex;
    if (!tex.Load(std::string(WARP_ASSET_DIR) + "/spot_texture.png")) {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<Maths::Vec3f> faceNormals;
    compute_face_normals(mesh, faceNormals);
    std::vector<Maths::Vec3f> vertexNormals;
    compute_vertex_normals(mesh, faceNormals, vertexNormals);

    const Maths::Vec3f light = Maths::Vec3f(0.4f, 0.8f, 0.45f).Normalize();

    ShadeParams sp;
    sp.L = light;
    sp.H = (light + Maths::Vec3f(0.0f, 0.0f, 1.0f)).Normalize();
    sp.lighting = true;
    sp.bilinear = true;
    float eyeZ = 3.0f;
    const Uint64 freq = SDL_GetPerformanceFrequency();
    const Uint64 start = SDL_GetPerformanceCounter();
    Uint64 last = start;
    double fpsAccum = 0.0;
    int    fpsCount = 0;
    char   title[160];

	ParticleSystem particleSystem;
	//for (int i = 0; i < 1000000; ++i) {
	//	Particle p;
 //       p.velocity[0] = static_cast<float>(rand()) / RAND_MAX - 0.5f;
 //       p.velocity[1] = static_cast<float>(rand()) / RAND_MAX - 0.5f;
 //       p.velocity[2] = static_cast<float>(rand()) / RAND_MAX - 0.5f;
 //       p.lifetime = static_cast<float>(rand()) / RAND_MAX * 5.0f;
 //       p.initialLifetime = p.lifetime;
	//	particleSystem.emit(p);
	//}
    // ---------------------------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------------------------

    bool running = true;
    SDL_Event e;
	float deltaTime = 0.0f;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) running = false;
            if (e.type == SDL_EVENT_MOUSE_WHEEL && e.wheel.y > 0) {
                eyeZ -= 0.15f;
            }
            if (e.type == SDL_EVENT_MOUSE_WHEEL && e.wheel.y < 0) {
                eyeZ += 0.15f;
            }
            if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_L) sp.lighting = !sp.lighting;
                if (e.key.key == SDLK_W) eyeZ -= 0.15f;
                if (e.key.key == SDLK_S) eyeZ += 0.15f;
                if (e.key.key == SDLK_P) {
                    Particle p;
                    p.velocity[0] = static_cast<float>(rand()) / RAND_MAX - 0.5f;
                    p.velocity[1] = static_cast<float>(rand()) / RAND_MAX - 0.5f;
                    p.velocity[2] = static_cast<float>(rand()) / RAND_MAX - 0.5f;
                    p.lifetime = static_cast<float>(rand()) / RAND_MAX * 5.0f;
                    p.initialLifetime = p.lifetime;
                    particleSystem.emit(p);
                }
            }
        }

        // ---------------------------------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------------------------------------------------

        const double seconds = static_cast<double>(SDL_GetPerformanceCounter() - start) / static_cast<double>(freq);
        const float angle = static_cast<float>(seconds);


        const Maths::Matrix4x4 view = Maths::Matrix4x4::LookAt(
            Maths::Vec3f(0, 0, eyeZ), Maths::Vec3f(0, 0, eyeZ - 1.0f), Maths::Vec3f(0, 1, 0));

        const Maths::Matrix4x4 model = Maths::Matrix4x4::RotationY(angle * 0.7f);
        const Maths::Matrix4x4 mvp = projection * (view * model);

        for (size_t i = 0; i < nv; i++) {
            const Maths::Vec3f& p = mesh.verts[i];
            const Maths::Vec4f clip = mvp * Maths::Vec4f(p.X(), p.Y(), p.Z(), 1.0f);
            float cv[4];
            _mm_storeu_ps(cv, clip.reg);            // cv = x, y, z, w

            const Maths::Vec3f& vn = vertexNormals[i];
            const Maths::Vec3f nw((model * Maths::Vec4f(vn.X(), vn.Y(), vn.Z(), 0.0f)).reg);   // w = 0: direction

            ClipVertex& c = clipVerts[i];
            c.x = cv[0]; c.y = cv[1]; c.z = cv[2]; c.w = cv[3];
            c.nx = nw.X(); c.ny = nw.Y(); c.nz = nw.Z();
            c.u = c.v = 0.0f;                       // set per face below
        }

		particleSystem.update(std::min(deltaTime, 0.05f));   // clamp: a stalled frame must not teleport everything


        clear(rgb(16, 16, 24));
        clear_zbuffer();


        const int nvi = static_cast<int>(nv);
        for (int f = 0; f < mesh.FaceCount(); f++) {
            const int a = mesh.faces[f * 3 + 0];
            const int b = mesh.faces[f * 3 + 1];
            const int c = mesh.faces[f * 3 + 2];
            if (a < 0 || b < 0 || c < 0 || a >= nvi || b >= nvi || c >= nvi) continue;   // bad OBJ index

            const int ua = mesh.faceUvs[f * 3 + 0];
            const int ub = mesh.faceUvs[f * 3 + 1];
            const int uc = mesh.faceUvs[f * 3 + 2];

            ClipVertex v0 = clipVerts[a], v1 = clipVerts[b], v2 = clipVerts[c];
            if (ua >= 0) { v0.u = mesh.uvs[ua].u; v0.v = mesh.uvs[ua].v; }
            if (ub >= 0) { v1.u = mesh.uvs[ub].u; v1.v = mesh.uvs[ub].v; }
            if (uc >= 0) { v2.u = mesh.uvs[uc].u; v2.v = mesh.uvs[uc].v; }

            ClipVertex out[6];
            const int nTris = Clip::ClipTriangleNear(v0, v1, v2, out);
            for (int t = 0; t < nTris; t++)
                project_and_put(&out[t * 3], &tex, sp);
        }


        put_particles(particleSystem, projection * view);

        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // ---------------------------------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------------------------------------------------
        const Uint64 now = SDL_GetPerformanceCounter();
        deltaTime = static_cast<float>(now - last) / static_cast<float>(freq);
        fpsAccum += static_cast<double>(now - last) / static_cast<double>(freq);
        last = now;
        fpsCount += 1;
        if (fpsAccum >= 1.0) {
            const double fps = fpsCount / fpsAccum;
            const double ms = 1000.0 * fpsAccum / fpsCount;
            snprintf(title, sizeof title,
                "Particle System %s  |  eye z=%.2f  |  %.1f FPS  (%.2f ms/frame)",
                sp.lighting ? "ON" : "OFF", eyeZ, fps, ms);
            SDL_SetWindowTitle(window, title);
            fpsAccum = 0.0;
            fpsCount = 0;
        }

        // ---------------------------------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------------------------------------------------
        
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}