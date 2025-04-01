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

    // Initialize animation values
    for (auto &item : menuItems)
    {
        item.hoverAmount = 0.0f;
        for (auto &dropdownItem : item.dropdownItems)
        {
            dropdownItem.hoverAmount = 0.0f;
        }
    }

    // Set default cursor
    currentCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

    recalculateMenuPositions();
}

MenuSystem::~MenuSystem()
{
    // Clean up cursor resources
    if (currentCursor)
    {
        SDL_FreeCursor(currentCursor);
    }
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
    bool hoveringOnItem = false;

    // Check main menu items
    for (auto &item : menuItems)
    {
        bool prevHovered = item.isHovered;
        item.isHovered = (mouseX >= item.rect.x && mouseX <= item.rect.x + item.rect.w &&
                          mouseY >= item.rect.y && mouseY <= item.rect.y + item.rect.h);

        if (item.isHovered)
        {
            hoveringOnItem = true;
        }

        // Check dropdown items if menu is open
        if (item.isOpen)
        {
            for (auto &dropdownItem : item.dropdownItems)
            {
                dropdownItem.isHovered = (mouseX >= dropdownItem.rect.x &&
                                          mouseX <= dropdownItem.rect.x + dropdownItem.rect.w &&
                                          mouseY >= dropdownItem.rect.y &&
                                          mouseY <= dropdownItem.rect.y + dropdownItem.rect.h);

                if (dropdownItem.isHovered)
                {
                    hoveringOnItem = true;
                }
            }
        }
    }

    // Update cursor based on whether we're hovering on a menu item
    SDL_Cursor *newCursor = hoveringOnItem ? SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND) : SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

    // Only change the cursor if it's different
    if (newCursor != currentCursor)
    {
        if (currentCursor)
        {
            SDL_FreeCursor(currentCursor);
        }
        currentCursor = newCursor;
        SDL_SetCursor(currentCursor);
    }
    else
    {
        // If it's the same, free the new one since we don't need it
        SDL_FreeCursor(newCursor);
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
    // Update hover animations
    const float ANIMATION_SPEED = 8.0f; // Speed of hover animation

    for (auto &item : menuItems)
    {
        // Target is 1.0 if hovered, 0.0 if not
        float target = item.isHovered ? 1.0f : 0.0f;

        // Smoothly animate toward target
        if (item.hoverAmount < target)
        {
            item.hoverAmount = std::min(item.hoverAmount + deltaTime * ANIMATION_SPEED, 1.0f);
        }
        else if (item.hoverAmount > target)
        {
            item.hoverAmount = std::max(item.hoverAmount - deltaTime * ANIMATION_SPEED, 0.0f);
        }

        // Update dropdown items
        for (auto &dropdownItem : item.dropdownItems)
        {
            float dropdownTarget = dropdownItem.isHovered ? 1.0f : 0.0f;

            if (dropdownItem.hoverAmount < dropdownTarget)
            {
                dropdownItem.hoverAmount = std::min(dropdownItem.hoverAmount + deltaTime * ANIMATION_SPEED, 1.0f);
            }
            else if (dropdownItem.hoverAmount > dropdownTarget)
            {
                dropdownItem.hoverAmount = std::max(dropdownItem.hoverAmount - deltaTime * ANIMATION_SPEED, 0.0f);
            }
        }
    }
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
    SDL_Color normalBgColor = {230, 230, 230, 255};
    SDL_Color hoverBgColor = {200, 200, 200, 255};

    for (const auto &item : menuItems)
    {
        // Interpolate colors based on hover amount
        SDL_Color bgColor = {
            static_cast<Uint8>(normalBgColor.r + (hoverBgColor.r - normalBgColor.r) * item.hoverAmount),
            static_cast<Uint8>(normalBgColor.g + (hoverBgColor.g - normalBgColor.g) * item.hoverAmount),
            static_cast<Uint8>(normalBgColor.b + (hoverBgColor.b - normalBgColor.b) * item.hoverAmount),
            255};

        SDL_Color textColor = {
            static_cast<Uint8>(normalColor.r + (hoverColor.r - normalColor.r) * item.hoverAmount),
            static_cast<Uint8>(normalColor.g + (hoverColor.g - normalColor.g) * item.hoverAmount),
            static_cast<Uint8>(normalColor.b + (hoverColor.b - normalColor.b) * item.hoverAmount),
            255};

        // Draw background with interpolated color
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &item.rect);

        renderText(renderer, font, item.text.c_str(),
                   item.rect.x + 10,
                   (menuBarHeight - TTF_FontHeight(font)) / 2,
                   textColor);

        // Draw dropdown if open
        if (item.isOpen)
        {
            for (const auto &dropdownItem : item.dropdownItems)
            {
                // Interpolate dropdown colors
                SDL_Color dropdownBgColor = {
                    static_cast<Uint8>(240 + (hoverBgColor.r - 240) * dropdownItem.hoverAmount),
                    static_cast<Uint8>(240 + (hoverBgColor.g - 240) * dropdownItem.hoverAmount),
                    static_cast<Uint8>(240 + (hoverBgColor.b - 240) * dropdownItem.hoverAmount),
                    255};

                SDL_Color dropdownTextColor = {
                    static_cast<Uint8>(normalColor.r + (hoverColor.r - normalColor.r) * dropdownItem.hoverAmount),
                    static_cast<Uint8>(normalColor.g + (hoverColor.g - normalColor.g) * dropdownItem.hoverAmount),
                    static_cast<Uint8>(normalColor.b + (hoverColor.b - normalColor.b) * dropdownItem.hoverAmount),
                    255};

                // Draw dropdown background with interpolated color
                SDL_SetRenderDrawColor(renderer, dropdownBgColor.r, dropdownBgColor.g, dropdownBgColor.b, 255);
                SDL_RenderFillRect(renderer, &dropdownItem.rect);

                // Draw border
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_RenderDrawRect(renderer, &dropdownItem.rect);

                // Draw text with interpolated color
                renderText(renderer, font, dropdownItem.text.c_str(),
                           dropdownItem.rect.x + 10,
                           dropdownItem.rect.y + (30 - TTF_FontHeight(font)) / 2,
                           dropdownTextColor);
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