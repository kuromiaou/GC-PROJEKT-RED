#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

// A PNG decoded into ARGB8888 so a sampled texel can go straight into the
// framebuffer. U and V wrap into [0,1]; v is flipped because OBJ's v = 0 is
// the image bottom while row 0 of the pixel array is the top.
class Texture
{
public:
    bool Load(const std::string& path);

    int Width()  const { return width; }
    int Height() const { return height; }

    /// Nearest-neighbour: one texel. Blocky when magnified.
    inline uint32_t Sample(float u, float v) const
    {
        u -= std::floor(u);
        v -= std::floor(v);
        int x = static_cast<int>(u * static_cast<float>(width));
        int y = static_cast<int>((1.0f - v) * static_cast<float>(height));
        x = Clamp(x, 0, width - 1);
        y = Clamp(y, 0, height - 1);
        return pixels[static_cast<size_t>(y) * width + x];
    }

    /// Bilinear: blends the four nearest texels by the fractional position.
    inline uint32_t SampleBilinear(float u, float v) const
    {
        u -= std::floor(u);
        v -= std::floor(v);

        // (size-1) keeps u = v = 1 on the last texel instead of one past it.
        const float fx = u * static_cast<float>(width - 1);
        const float fy = (1.0f - v) * static_cast<float>(height - 1);

        int x0 = static_cast<int>(std::floor(fx));
        int y0 = static_cast<int>(std::floor(fy));
        const float tx = fx - static_cast<float>(x0);
        const float ty = fy - static_cast<float>(y0);
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        x0 = Clamp(x0, 0, width - 1);  x1 = Clamp(x1, 0, width - 1);
        y0 = Clamp(y0, 0, height - 1); y1 = Clamp(y1, 0, height - 1);

        const uint32_t c00 = pixels[static_cast<size_t>(y0) * width + x0];
        const uint32_t c10 = pixels[static_cast<size_t>(y0) * width + x1];
        const uint32_t c01 = pixels[static_cast<size_t>(y1) * width + x0];
        const uint32_t c11 = pixels[static_cast<size_t>(y1) * width + x1];

        const float w00 = (1.0f - tx) * (1.0f - ty);
        const float w10 = tx * (1.0f - ty);
        const float w01 = (1.0f - tx) * ty;
        const float w11 = tx * ty;

        const float r = w00 * ((c00 >> 16) & 0xFF) + w10 * ((c10 >> 16) & 0xFF)
                      + w01 * ((c01 >> 16) & 0xFF) + w11 * ((c11 >> 16) & 0xFF);
        const float g = w00 * ((c00 >> 8) & 0xFF) + w10 * ((c10 >> 8) & 0xFF)
                      + w01 * ((c01 >> 8) & 0xFF) + w11 * ((c11 >> 8) & 0xFF);
        const float b = w00 * (c00 & 0xFF) + w10 * (c10 & 0xFF)
                      + w01 * (c01 & 0xFF) + w11 * (c11 & 0xFF);

        return (0xFFu << 24) | (static_cast<uint32_t>(r) << 16)
             | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
    }

private:
    static inline int Clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

    std::vector<uint32_t> pixels;
    int width  = 0;
    int height = 0;
};
