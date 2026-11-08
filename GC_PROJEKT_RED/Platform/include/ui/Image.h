#pragma once
#include "Platform/Texture.h"

struct Image
{
    const Texture* texture = nullptr;
    int x = 0; 
    int y = 0;
    int width = 0, height = 0;   
    float opacity = 1.0f;        
};