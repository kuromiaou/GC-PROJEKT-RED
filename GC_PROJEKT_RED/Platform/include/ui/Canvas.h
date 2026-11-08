#pragma once
#include <cstdint>
#include <vector>
#include "ui/Image.h"

class Canvas
{
public:
    void AddImage(const Image& image);
    void Draw(std::vector<uint32_t>& framebuffer, int screenWidth, int screenHeight) const;

private:
    void DrawImage(const Image& image, std::vector<uint32_t>& framebuffer,
        int screenWidth, int screenHeight) const;

    std::vector<Image> images;
};