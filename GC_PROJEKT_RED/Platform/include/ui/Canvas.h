#pragma once
#include <cstdint>
#include <vector>
#include "ui/Image.h"
#include "ui/Text.h"

class Canvas
{
public:
    void AddImage(const Image& image);
    void Draw(std::vector<uint32_t>& framebuffer, int screenWidth, int screenHeight) const;
    void AddText(const Text& text);

private:
    void DrawImage(const Image& image, std::vector<uint32_t>& framebuffer, int screenWidth, int screenHeight) const;
    void DrawText(const Text& text, std::vector<uint32_t>& framebuffer, int screenWidth, int screenHeight) const;

    std::vector<Image> images;
    std::vector<Text> texts;
};