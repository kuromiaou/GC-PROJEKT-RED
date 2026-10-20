#include "Platform/Window.h"
#include "Platform/Obj.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <cmath>
#include <cstdio>
#include <string>

#ifndef WARP_ASSET_DIR
#define WARP_ASSET_DIR "assets"
#endif

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


int Window::TestWindow()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Chapter 07 - Z-buffer", WIDTH, HEIGHT, 0);
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

    const Maths::Matrix4x4 view = Maths::Matrix4x4::LookAt(
        Maths::Vec3f(0, 0, 3), Maths::Vec3f(0, 0, 0), Maths::Vec3f(0, 1, 0));

    const size_t nv = mesh.verts.size();
    std::vector<int>   screenX(nv), screenY(nv);
    std::vector<float> ndcZ(nv);
    std::vector<char>  visible(nv);
    bool useZBuffer = true;   
    const Uint64 freq = SDL_GetPerformanceFrequency();
    const Uint64 start = SDL_GetPerformanceCounter();
    Uint64 last = start;
    double fpsAccum = 0.0;
    int    fpsCount = 0;
    char   title[160];

    // ---------------------------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------------------------

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_Z) useZBuffer = !useZBuffer;
        }

        // ---------------------------------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------------------------------------------------

        const double seconds = static_cast<double>(SDL_GetPerformanceCounter() - start) / static_cast<double>(freq);
        const float angle = static_cast<float>(seconds);


        const Maths::Matrix4x4 model = Maths::Matrix4x4::RotationY(angle * 0.7f);
        const Maths::Matrix4x4 mvp = projection * (view * model);

        float zmin = 1e30f, zmax = -1e30f;
        for (size_t i = 0; i < nv; i++) {
            const Maths::Vec3f v = mesh.verts[i];
            const Maths::Vec4f clip = mvp * Maths::Vec4f(v.X(), v.Y(), v.Z(), 1.0f);
            float c[4];
            _mm_storeu_ps(c, clip.reg);       // c = {x, y, z, w}
            if (c[3] <= zNear) { visible[i] = 0; continue; }
            const float ndcX = c[0] / c[3];
            const float ndcY = c[1] / c[3];
            const float nz = c[2] / c[3];
            screenX[i] = static_cast<int>((ndcX + 1.0f) * 0.5f * WIDTH);
            screenY[i] = static_cast<int>((1.0f - (ndcY + 1.0f) * 0.5f) * HEIGHT); // screen Y points down
            ndcZ[i] = nz;
            visible[i] = 1;
            zmin = std::min(zmin, nz);
            zmax = std::max(zmax, nz);
        }
        const float zrange = (zmax > zmin) ? (zmax - zmin) : 1.0f;

        clear(rgb(16, 16, 24));
        clear_zbuffer();

        const int faceCount = mesh.FaceCount();
        for (int f = 0; f < faceCount; f++) {
            const int a = mesh.faces[f * 3 + 0];
            const int b = mesh.faces[f * 3 + 1];
            const int c = mesh.faces[f * 3 + 2];
            if (a < 0 || b < 0 || c < 0 || a >= (int)nv || b >= (int)nv || c >= (int)nv) continue; // bad OBJ index
            if (!visible[a] || !visible[b] || !visible[c]) continue;

            put_triangle_z(screenX[a], screenY[a], ndcZ[a],
                screenX[b], screenY[b], ndcZ[b],
                screenX[c], screenY[c], ndcZ[c],
                zmin, zrange, useZBuffer);
        }

        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // ---------------------------------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------------------------------------------------

        const Uint64 now = SDL_GetPerformanceCounter();
        fpsAccum += static_cast<double>(now - last) / static_cast<double>(freq);
        last = now;
        fpsCount += 1;
        if (fpsAccum >= 1.0) {
            const double fps = fpsCount / fpsAccum;
            const double ms = 1000.0 * fpsAccum / fpsCount;
            snprintf(title, sizeof title,
                "Chapter 07 - Z-buffer %s  |  %d tris  |  %.1f FPS  (%.2f ms/frame)",
                useZBuffer ? "ON" : "OFF", mesh.FaceCount(), fps, ms);
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