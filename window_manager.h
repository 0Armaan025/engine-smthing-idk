#pragma once

#include "SDL2/SDL.h"
#include <string>

class WindowManager
{
private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    int width;
    int height;
    std::string title;

public:
    WindowManager(const std::string &title, int width, int height);
    ~WindowManager();

    bool initialize();
    void cleanup();

    SDL_Renderer *getRenderer() { return renderer; }
    SDL_Window *getWindow() { return window; }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void updateSize();
};
