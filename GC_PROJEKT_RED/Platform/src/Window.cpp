#include "Platform/Window.h"
#include <SDL3/SDL.h>
#include <iostream>

void Window::TestWindow() {
	SDL_Init(SDL_INIT_VIDEO);
	std::cout << SDL_GetVersion() << std::endl;
}