#include "menu_system.h"
#include <iostream>

MenuSystem::MenuSystem(TTF_Font *f, int windowWidth) : font(f)
{
    // Initialize main menu items
    MenuItem fileMenu;
    fileMenu.text = "File";

    MenuItem editMenu;
    editMenu.text = "Edit";

    MenuItem toolsMenu;
    toolsMenu.text = "Tools";
    toolsMenu.dropdownItems.push_back({"Pencil"});
    toolsMenu.dropdownItems.push_back({"Line"});
    toolsMenu.dropdownItems.push_back({"Rectangle"});
    toolsMenu.dropdownItems.push_back({"Circle"});
    toolsMenu.dropdownItems.push_back({"Eraser"});

    MenuItem viewMenu;
    viewMenu.text = "View";

    MenuItem helpMenu;
    helpMenu.text = "Help";

    menuItems.push_back(fileMenu);
    menuItems.push_back(editMenu);
    menuItems.push_back(toolsMenu);
    menuItems.push_back(viewMenu);
    menuItems.push_back(helpMenu);

    recalculateMenuPositions();
}

void MenuSystem::recalculateMenuPositions()
{
    int x = 10;
    for (auto &item : menuItems)
    {
        int textWidth, textHeight;
        TTF_SizeText(font, item.text.c_str(), &textWidth, &textHeight);
        item.rect = {x, 0, textWidth + 20, menuBarHeight};
        x += textWidth + 20;

        // Recalculate dropdown positions
        for (size_t i = 0; i < item.dropdownItems.size(); i++)
        {
            item.dropdownItems[i].rect = {
                item.rect.x,
                static_cast<int>(menuBarHeight + i * 30), // Explicitly cast to int
                static_cast<int>(dropdownWidth),
                30};
        }
    }
}

void MenuSystem::handleMouseMotion(int mouseX, int mouseY)
{
    // Check main menu items
    for (auto &item : menuItems)
    {
        bool prevHovered = item.isHovered;
        item.isHovered = (mouseX >= item.rect.x && mouseX <= item.rect.x + item.rect.w &&
                          mouseY >= item.rect.y && mouseY <= item.rect.y + item.rect.h);

        // Check dropdown items if menu is open
        if (item.isOpen)
        {
            for (auto &dropdownItem : item.dropdownItems)
            {
                dropdownItem.isHovered = (mouseX >= dropdownItem.rect.x &&
                                          mouseX <= dropdownItem.rect.x + dropdownItem.rect.w &&
                                          mouseY >= dropdownItem.rect.y &&
                                          mouseY <= dropdownItem.rect.y + dropdownItem.rect.h);
            }
        }
    }
}

void MenuSystem::handleMouseClick(int mouseX, int mouseY)
{
    bool clickedOnMenu = false;

    // Check main menu items
    for (auto &item : menuItems)
    {
        bool clickedOnThis = (mouseX >= item.rect.x &&
                              mouseX <= item.rect.x + item.rect.w &&
                              mouseY >= item.rect.y &&
                              mouseY <= item.rect.y + item.rect.h);

        if (clickedOnThis)
        {
            clickedOnMenu = true;
            item.isOpen = !item.isOpen;

            // Close other menus
            for (auto &otherItem : menuItems)
            {
                if (&otherItem != &item)
                {
                    otherItem.isOpen = false;
                }
            }
        }

        // Check dropdown items if menu is open
        if (item.isOpen)
        {
            for (auto &dropdownItem : item.dropdownItems)
            {
                bool clickedOnDropdown = (mouseX >= dropdownItem.rect.x &&
                                          mouseX <= dropdownItem.rect.x + dropdownItem.rect.w &&
                                          mouseY >= dropdownItem.rect.y &&
                                          mouseY <= dropdownItem.rect.y + dropdownItem.rect.h);

                if (clickedOnDropdown)
                {
                    clickedOnMenu = true;
                    item.isOpen = false;
                    // Tool selection logic would go here
                }
            }
        }
    }

    // Close all menus if clicked outside
    if (!clickedOnMenu)
    {
        for (auto &item : menuItems)
        {
            item.isOpen = false;
        }
    }
}

void MenuSystem::update(float deltaTime)
{
    // No animations needed for this simplified version
}

void MenuSystem::render(SDL_Renderer *renderer, int windowWidth)
{
    // Draw menu bar background
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_Rect menuBarRect = {0, 0, windowWidth, menuBarHeight};
    SDL_RenderFillRect(renderer, &menuBarRect);

    // Draw menu items
    SDL_Color normalColor = {50, 50, 50, 255};
    SDL_Color hoverColor = {0, 0, 0, 255};

    for (const auto &item : menuItems)
    {
        // Draw menu item
        renderText(renderer, font, item.text.c_str(),
                   item.rect.x + 10,
                   (menuBarHeight - TTF_FontHeight(font)) / 2,
                   item.isHovered ? hoverColor : normalColor);

        // Draw dropdown if open
        if (item.isOpen)
        {
            for (const auto &dropdownItem : item.dropdownItems)
            {
                // Draw dropdown background
                SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
                SDL_RenderFillRect(renderer, &dropdownItem.rect);

                // Draw border
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_RenderDrawRect(renderer, &dropdownItem.rect);

                // Draw text
                renderText(renderer, font, dropdownItem.text.c_str(),
                           dropdownItem.rect.x + 10,
                           dropdownItem.rect.y + (30 - TTF_FontHeight(font)) / 2,
                           dropdownItem.isHovered ? hoverColor : normalColor);
            }
        }
    }
}

int MenuSystem::getMenuBarHeight() const
{
    return menuBarHeight;
}

const std::vector<MenuItem> &MenuSystem::getMenuItems() const
{
    return menuItems;
}

void MenuSystem::renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color)
{
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    if (!surface)
        return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        SDL_FreeSurface(surface);
        return;
    }

    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect destRect = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}