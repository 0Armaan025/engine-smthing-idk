#include "panel.h"

Panel::Panel(int screenWidth, int screenHeight, int panelWidth) : screenWidth(screenWidth), screenHeight(screenHeight), panelWidth(panelWidth), scrollOffset(0), panelHeight(screenHeight) {}

void Panel::render(SDL_Renderer *renderer)
{
    // bg first
    SDL_Rect panelRect = {screenWidth - panelWidth, 30, panelWidth, panelHeight};
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // grey
    SDL_RenderFillRect(renderer, &panelRect);

    for (int i = 0; i < 30; i++)
    {
        SDL_Rect itemRect = {screenWidth - panelWidth + 10, 20 + (i * 50) - scrollOffset, panelWidth - 20, 40};
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255); // Blue items
        SDL_RenderFillRect(renderer, &itemRect);
    }
}

void Panel::handleEvent(SDL_Event &e)
{
    if (e.type == SDL_MOUSEWHEEL)
    {
        scrollOffset -= e.wheel.y * 20;
        if (scrollOffset < 0)
            scrollOffset = 0;
        if (scrollOffset > 400)
            scrollOffset = 400; // Adjust based on content height
    }
}