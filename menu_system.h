#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <vector>
#include <string>

// Forward declarations
struct DropdownItem;

struct MenuItem
{
    std::string text;
    int x, width;
    bool isHovered = false;
    bool isOpen = false;
    float hoverAnimation = 0.0f; // 0.0 to 1.0 for animation smoothness
    std::vector<DropdownItem> dropdownItems;
};

struct DropdownItem
{
    std::string text;
    bool isHovered = false;
    float hoverAnimation = 0.0f;
};

class MenuSystem
{
public:
    MenuSystem(TTF_Font *font, int windowWidth);
    ~MenuSystem();

    void handleMouseMotion(int mouseX, int mouseY);
    void handleMouseClick(int mouseX, int mouseY);
    void update(float deltaTime);
    void render(SDL_Renderer *renderer, int windowWidth);

    // Accessors
    const std::vector<MenuItem> &getMenuItems() const { return menuItems; }
    MenuItem *getMenuItemByName(const std::string &name);

    // Menu item management
    void addMenuItem(const std::string &text, const std::vector<std::string> &dropdownItems);
    void recalculateMenuPositions();

    // Utility functions
    int getMenuBarHeight() const { return MENU_BAR_HEIGHT; }
    int getDropdownItemHeight() const { return DROPDOWN_ITEM_HEIGHT; }

private:
    std::vector<MenuItem> menuItems;
    TTF_Font *font;

    // Text rendering utilities
    SDL_Texture *renderTextToTexture(SDL_Renderer *renderer, const char *text, SDL_Color color);
    void renderText(SDL_Renderer *renderer, const char *text, int x, int y, SDL_Color color);
    int getTextWidth(const std::string &text);

    // Constants
    static const int MENU_BAR_HEIGHT = 26;
    static const int DROPDOWN_ITEM_HEIGHT = 24;
    static const int DROPDOWN_WIDTH = 150;
};