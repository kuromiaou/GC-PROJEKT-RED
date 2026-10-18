#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

#include "Maths.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

class Window
{
public:
    static constexpr int WIDTH  = 800;
    static constexpr int HEIGHT = 600;

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
        
    std::vector<uint32_t> framebuffer;
};