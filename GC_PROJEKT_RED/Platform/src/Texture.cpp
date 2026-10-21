#include "Platform/Texture.h"

#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_PNG
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4244 4456 4457 4459 4505 4267 4100 4127 4702)
#endif
#include "stb_image.h"
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

bool Texture::Load(const std::string& path)
{
    int w = 0, h = 0, channels = 0;
    // Ask for 4 channels; stb fills alpha = 255 when the image has none.
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data)
    {
        std::fprintf(stderr, "Texture::Load: failed on '%s': %s\n", path.c_str(), stbi_failure_reason());
        return false;
    }

    width = w;
    height = h;
    pixels.resize(static_cast<size_t>(w) * h);
    for (size_t i = 0; i < pixels.size(); ++i)
    {
        const uint32_t r = data[i * 4 + 0];
        const uint32_t g = data[i * 4 + 1];
        const uint32_t b = data[i * 4 + 2];
        const uint32_t a = data[i * 4 + 3];
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;   // RGBA8 -> ARGB8888
    }
    stbi_image_free(data);
    return true;
}
