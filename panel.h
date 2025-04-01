#pragma once

#include <SDL2/SDL.h>
#include <iostream>

class Panel
{
private:
    int screenWidth;
    int screenHeight;
    int panelWidth;
    int scrollOffset;
    
    const int panelHeight;

public:
    Panel(int screenWidth, int screenHeight, int panelWidth);

    void render(SDL_Renderer *renderer);
    void handleEvent(SDL_Event &e);
};