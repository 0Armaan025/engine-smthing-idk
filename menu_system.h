#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <vector>
#include <string>

// Menu item structure
struct MenuItem
{
    std::string text;
    SDL_Rect rect;
    bool isHovered = false;
    bool isOpen = false;
    std::vector<MenuItem> dropdownItems;
};

class MenuSystem
{
private:
    TTF_Font *font;
    std::vector<MenuItem> menuItems;
    int menuBarHeight = 30;
    int dropdownWidth = 150;

    void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color);

public:
    MenuSystem(TTF_Font *f, int windowWidth);

    void recalculateMenuPositions();
    void handleMouseMotion(int mouseX, int mouseY);
    void handleMouseClick(int mouseX, int mouseY);
    void update(float deltaTime);
    void render(SDL_Renderer *renderer, int windowWidth);

    int getMenuBarHeight() const;
    const std::vector<MenuItem> &getMenuItems() const;
};