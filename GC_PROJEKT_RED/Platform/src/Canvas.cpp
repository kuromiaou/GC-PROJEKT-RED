#include "ui/Canvas.h"

namespace
{
    // Mélange une composante (0-255) de la source sur la destination
    inline uint32_t BlendChannel(uint32_t src, uint32_t dst, float alpha)
    {
        return static_cast<uint32_t>(src * alpha + dst * (1 - alpha));
    }


    inline uint32_t Blend(uint32_t src, uint32_t dst, float alpha)
    {
        const uint32_t r = BlendChannel((src >> 16) & 0xFF, (dst >> 16) & 0xFF, alpha);
        const uint32_t g = BlendChannel((src >> 8) & 0xFF, (dst >> 8) & 0xFF, alpha);
        const uint32_t b = BlendChannel(src & 0xFF, dst & 0xFF, alpha);
        return (0xFFu << 24) | (r << 16) | (g << 8) | b;
    }
}

void Canvas::AddImage(const Image& image)
{
    images.push_back(image);
}

void Canvas::Draw(std::vector<uint32_t>& framebuffer, int screenWidth, int screenHeight) const
{
    for (const Image& image : images)
        DrawImage(image, framebuffer, screenWidth, screenHeight);
}

void Canvas::DrawImage(const Image& image, std::vector<uint32_t>& framebuffer,
    int screenWidth, int screenHeight) const
{
    if (!image.texture || image.width <= 0 || image.height <= 0) return;

    for (int py = 0; py < image.height; py++) {
        for (int px = 0; px < image.width; px++) {
            const int sx = image.x + px;
            const int sy = image.y + py;
            if (sx < 0 || sy < 0 || sx >= screenWidth || sy >= screenHeight) continue;

            const float u = static_cast<float>(px) / static_cast<float>(image.width);
            const float v = 1 - static_cast<float>(py) / static_cast<float>(image.height);

            const uint32_t texel = image.texture->Sample(u, v);
            const float alpha = static_cast<float>((texel >> 24) & 0xFF) / 255.0f * image.opacity;
            if (alpha <= 0.0f) continue;

            const size_t idx = static_cast<size_t>(sy) * screenWidth + sx;
            framebuffer[idx] = Blend(texel, framebuffer[idx], alpha);
        }
    }
}