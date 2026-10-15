#include "Platform/Window.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>

Window::Window()
    : framebuffer(static_cast<size_t>(WIDTH)* HEIGHT)
{
}
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
    if (area == 0) return;                 // degenerate sliver

    // Force one winding so "inside" means "all weights >= 0".
    if (area < 0) {
        int tx = x1; int ty = y1; uint32_t tc = c1;
        x1 = x2; y1 = y2; c1 = c2;
        x2 = tx; y2 = ty; c2 = tc;
        area = -area;
    }

    // Bounding box, clamped to the screen.
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
            // Inside iff all three weights are non-negative.
            if ((e0 | e1 | e2) >= 0) {
                framebuffer[y * WIDTH + x] =
                    shade(e0, e1, e2, area, c0, c1, c2);
            }
        }
    }
}

void Window::clear(uint32_t color) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) framebuffer[i] = color;
}
int Window::TestWindow()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    //SDL_Window* window = SDL_CreateWindow("Chapter 01 - Framebuffer", WIDTH, HEIGHT, 0);
    //if (!window) {
    //    SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
    //    SDL_Quit();
    //    return 1;
    //}

    //SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    //if (!renderer) {
    //    SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
    //    SDL_DestroyWindow(window);
    //    SDL_Quit();
    //    return 1;
    //}
    //SDL_SetRenderVSync(renderer, 1);

    //SDL_Texture* texture = SDL_CreateTexture(
    //    renderer, SDL_PIXELFORMAT_ARGB8888,
    //    SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    //if (!texture) {
    //    SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
    //    SDL_DestroyRenderer(renderer);
    //    SDL_DestroyWindow(window);
    //    SDL_Quit();
    //    return 1;
    //}

    SDL_Window* window = SDL_CreateWindow("Chapter 02 — Lines", WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    //const Uint64 freq = SDL_GetPerformanceFrequency();
    //Uint64 last = SDL_GetPerformanceCounter();
    //double fps_accum = 0.0;
    //int    fps_count = 0;
    //char   title[160];

    //double angle_offset = 0.0;                  
    //const double theta = 0.002;                  
    //const int every_n_frames = 1;               
    //int frame = 0;

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE)
                running = false;
        }

        std::fill(framebuffer.begin(), framebuffer.end(), rgb(0, 0, 0));
        clear(rgb(16, 16, 24));

        //for (int y = 0; y < HEIGHT; y++) {
        //    for (int x = 0; x < WIDTH; x++) {
        //        uint8_t r = static_cast<uint8_t>(x * 255 / WIDTH);
        //        uint8_t b = static_cast<uint8_t>(y * 255 / HEIGHT);
        //        put_pixel(x, y, rgb(r, 64, b));
        //    }
        //}
        //for (int x = 0; x < WIDTH; x++) {
        //    put_pixel(x, 300, rgb(255, 64, 0));
        //}
        
        //const int cx = WIDTH / 2;
        //const int cy = HEIGHT / 2;
        //const int spokes = 64;
        //const int radius = (HEIGHT / 2) - 20;
        //const double TWO_PI = 6.2831853071795864;

        //for (int i = 0; i < spokes; i++) {
        //    double t0 = (double)i / (double)spokes * TWO_PI;   
        //    double t = t0 + angle_offset;
        //    double ct = SDL_cos(t);
        //    double st = SDL_sin(t);
        //    int ex = cx + (int)(radius * ct);
        //    int ey = cy + (int)(radius * st);
        //    uint8_t r = (uint8_t)(127.0 + 127.0 * ct);
        //    uint8_t g = (uint8_t)(127.0 + 127.0 * st);
        //    put_line(cx, cy, ex, ey, rgb(r, g, 200));
        //}
        //frame++;
        //if (frame % every_n_frames == 0) {
        //    angle_offset += theta;
        //    if (angle_offset >= TWO_PI) angle_offset -= TWO_PI;  
        //}


        //Uint64 now = SDL_GetPerformanceCounter();
        //double dt = (double)(now - last) / (double)freq;
        //last = now;
        //fps_accum += dt;
        //fps_count += 1;
        //if (fps_accum >= 1.0) {
        //    double fps = (double)fps_count / fps_accum;
        //    double ms = 1000.0 * fps_accum / (double)fps_count;
        //    snprintf(title, sizeof title,
        //        "Chapter X — <...>  |  %.1f FPS  (%.2f ms/frame)",
        //        fps, ms);
        //    SDL_SetWindowTitle(window, title);
        //    fps_accum = 0.0;
        //    fps_count = 0;
        //}
        
        // 1) Big triangle with red / green / blue corners. The
        // smooth gradient inside IS the barycentric blend.
        put_triangle(150, 480, rgb(255, 0, 0),
            400, 80, rgb(0, 255, 0),
            650, 480, rgb(0, 0, 255));

        // 2) Small flat-shaded triangle (same colour x3).
        put_triangle(60, 60, rgb(240, 240, 240),
            200, 60, rgb(240, 240, 240),
            130, 170, rgb(240, 240, 240));

        // 3) White wireframe over the big triangle's edges.
        put_line(150, 480, 400, 80, rgb(255, 255, 255));
        put_line(400, 80, 650, 480, rgb(255, 255, 255));
        put_line(650, 480, 150, 480, rgb(255, 255, 255));

        //SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t));
        //SDL_RenderClear(renderer);
        //SDL_RenderCopy(renderer, texture, NULL, NULL);
        //SDL_RenderPresent(renderer);
        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}