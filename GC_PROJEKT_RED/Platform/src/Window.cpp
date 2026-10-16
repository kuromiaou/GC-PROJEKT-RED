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