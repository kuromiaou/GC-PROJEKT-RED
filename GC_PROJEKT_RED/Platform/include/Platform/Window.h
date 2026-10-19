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
    uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);

    void put_pixel(int x, int y, uint32_t color);

    void put_line(int x0, int y0, int x1, int y1, uint32_t color);

    std::vector<uint32_t> framebuffer;
};