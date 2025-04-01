#include "window_manager.h"
#include <iostream>

WindowManager::WindowManager(const std::string &title, int width, int height)
    : window(nullptr), renderer(nullptr), width(width), height(height), title(title) {}

WindowManager::~WindowManager()
{
    cleanup();
}

bool WindowManager::initialize()
{
    // Create window
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void WindowManager::updateSize()
{
    SDL_GetWindowSize(window, &width, &height);
}

void WindowManager::cleanup()
{
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}
