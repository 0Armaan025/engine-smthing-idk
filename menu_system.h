#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

struct DropdownItem
{
    std::string text;
    SDL_Rect rect;
    bool isHovered = false;
    float hoverAmount = 0.0f; // Added for animation (0.0 to 1.0)
};

struct MenuItem
{
    std::string text;
    SDL_Rect rect;
    bool isHovered = false;
    bool isOpen = false;
    float hoverAmount = 0.0f; // Added for animation (0.0 to 1.0)
    std::vector<DropdownItem> dropdownItems;
};

class MenuSystem
{
public:
    MenuSystem(TTF_Font *font, int windowWidth);
    ~MenuSystem(); // Added destructor to clean up cursor resources

    void handleMouseMotion(int mouseX, int mouseY);
    void handleMouseClick(int mouseX, int mouseY);
    void update(float deltaTime);
    void render(SDL_Renderer *renderer, int windowWidth);

    int getMenuBarHeight() const;
    const std::vector<MenuItem> &getMenuItems() const;
    void recalculateMenuPositions();

private:
    static const int menuBarHeight = 30;
    static const int dropdownWidth = 150;

    TTF_Font *font;
    std::vector<MenuItem> menuItems;
    SDL_Cursor *currentCursor = nullptr; // Added to store current cursor

    void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color);
};

#endif