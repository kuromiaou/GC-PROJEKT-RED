#pragma once
#include <string>
#include "Platform/Texture.h"

struct Text
{
    const Texture* font = nullptr;   
    std::string content;
    int x = 0, y = 0;                
    int charSize = 16;               
    float opacity = 1.0f;
    // police bitmap : grille de 16 × 16 caractères ASCII
};