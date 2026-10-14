#pragma once
#include <cstdint>
#include <vector>

class Window
{
public:
    static constexpr int WIDTH  = 800;
    static constexpr int HEIGHT = 600;

    Window();
    int TestWindow();

private:
    static constexpr uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
    {
        return (0xFFu << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
    }

    void put_pixel(int x, int y, uint32_t color);

    std::vector<uint32_t> framebuffer;
};