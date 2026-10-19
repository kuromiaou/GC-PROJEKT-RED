#include "Platform/Window.h"
#include <SDL3/SDL.h>

Window::Window()
    : framebuffer(static_cast<size_t>(WIDTH)* HEIGHT)
{
}
uint32_t Window::rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (0xFFu << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}
void Window::put_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    framebuffer[static_cast<size_t>(y) * WIDTH + x] = color;
}

void Window::put_line(int x, int y, uint32 color)
{
    void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
        int dx = abs(x1 - x0);
        int dy = -abs(y1 - y0);             
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx + dy;                 

        for (;;) {
            put_pixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }   
            if (e2 <= dx) { err += dx; y0 += sy; }  
        }
    }
}

static inline int edge(int ax, int ay, int bx, int by, int px, int py) {
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}

// Blend three corner colours by the barycentric weights e0, e1, e2.
// wsum is the total triangle area (e0 + e1 + e2 for an inside pixel).
// We blend each channel independently.
static inline uint32_t shade(int e0, int e1, int e2, int wsum,
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

// Filled triangle with per-corner colours interpolated via
// barycentric coordinates. Pass the same colour three times for a
// flat-shaded triangle. See chapter README section 3.9.
static void draw_triangle(int x0, int y0, uint32_t c0,
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

int HandleButtonEvent(SDL_MouseButtonEvent& Event) {
    if (Event.button == SDL_BUTTON_LEFT) {
        return 1
    }
    else if (Event.button == SDL_BUTTON_RIGHT) {
        return 2
    }

    if (
        Event.button == SDL_BUTTON_LEFT &&
        Event.down
        ) {
    }
    return 0
}

int Window::TestWindow()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Chapter 01 - Framebuffer", WIDTH, HEIGHT, 0);
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
    SDL_SetRenderVSync(renderer, 1);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (!texture) {
        SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE)
                running = false;
        }
        for (int x = 0; x < WIDTH; x++) {
            put_pixel(x, 300, rgb(255, 64, 0));
        }
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