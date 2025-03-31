#include "menu_system.h"
#include <iostream>
#include <cmath>

MenuSystem::MenuSystem(TTF_Font* font, int windowWidth) : font(font) {
    // Initialize with default menu items
    addMenuItem("File", {"New", "Open", "Save", "Export", "Exit"});
    addMenuItem("Edit", {"Undo", "Redo", "Copy", "Paste"});
    addMenuItem("View", {"Zoom In", "Zoom Out", "Reset View"});
    addMenuItem("Tools", {"Pencil", "Line", "Rectangle", "Circle"});
    addMenuItem("Help", {"About", "Documentation"});
    
    // Calculate initial positions
    recalculateMenuPositions();
}

MenuSystem::~MenuSystem() {
    // No dynamic resources to clean up in this class
}

void MenuSystem::addMenuItem(const std::string& text, const std::vector<std::string>& dropdownItems) {
    MenuItem item;
    item.text = text;
    item.x = 0;  // Will be calculated in recalculateMenuPositions
    item.width = 0;  // Will be calculated in recalculateMenuPositions
    
    // Add dropdown items
    for (const auto& dropdownText : dropdownItems) {
        DropdownItem dropdownItem;
        dropdownItem.text = dropdownText;
        item.dropdownItems.push_back(dropdownItem);
    }
    
    menuItems.push_back(item);
}

void MenuSystem::recalculateMenuPositions() {
    int menuX = 10;
    for (auto& item : menuItems) {
        item.x = menuX;
        item.width = getTextWidth(item.text) + 16;
        menuX += item.width;
    }
}

MenuItem* MenuSystem::getMenuItemByName(const std::string& name) {
    for (auto& item : menuItems) {
        if (item.text == name) {
            return &item;
        }
    }
    return nullptr;
}

void MenuSystem::handleMouseMotion(int mouseX, int mouseY) {
    // Check for menu item hover
    for (auto& item : menuItems) {
        bool hovering = (mouseX >= item.x && mouseX < item.x + item.width &&
                         mouseY >= 0 && mouseY <= MENU_BAR_HEIGHT);

        item.isHovered = hovering;

        // Check dropdown items if menu is open
        if (item.isOpen) {
            for (size_t i = 0; i < item.dropdownItems.size(); i++) {
                auto& dropdownItem = item.dropdownItems[i];
                int itemY = MENU_BAR_HEIGHT + i * DROPDOWN_ITEM_HEIGHT;

                dropdownItem.isHovered = (mouseX >= item.x && mouseX < item.x + DROPDOWN_WIDTH &&
                                          mouseY >= itemY && mouseY < itemY + DROPDOWN_ITEM_HEIGHT);
            }
        }
    }
}

void MenuSystem::handleMouseClick(int mouseX, int mouseY) {
    // Check menu item clicks
    for (auto& item : menuItems) {
        if (mouseX >= item.x && mouseX < item.x + item.width &&
            mouseY >= 0 && mouseY <= MENU_BAR_HEIGHT) {

            // Close all other menus
            for (auto& otherItem : menuItems) {
                if (&otherItem != &item) {
                    otherItem.isOpen = false;
                }
            }

            // Toggle current menu
            item.isOpen = !item.isOpen;
        }
    }
}

void MenuSystem::update(float deltaTime) {
    // Update animations - macOS style
    for (auto& item : menuItems) {
        if (item.isHovered || item.isOpen) {
            item.hoverAnimation = std::min(1.0f, item.hoverAnimation + deltaTime * 8.0f);
        } else {
            item.hoverAnimation = std::max(0.0f, item.hoverAnimation - deltaTime * 3.0f);
        }

        // Update dropdown item animations
        for (auto& dropdownItem : item.dropdownItems) {
            if (dropdownItem.isHovered) {
                dropdownItem.hoverAnimation = std::min(1.0f, dropdownItem.hoverAnimation + deltaTime * 8.0f);
            } else {
                dropdownItem.hoverAnimation = std::max(0.0f, dropdownItem.hoverAnimation - deltaTime * 3.0f);
            }
        }
    }
}

SDL_Texture* MenuSystem::renderTextToTexture(SDL_Renderer* renderer, const char* text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface)
        return nullptr;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void MenuSystem::renderText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color) {
    SDL_Texture* texture = renderTextToTexture(renderer, text, color);
    if (!texture)
        return;

    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect destRect = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
}

int MenuSystem::getTextWidth(const std::string& text) {
    int width, height;
    TTF_SizeText(font, text.c_str(), &width, &height);
    return width;
}

void MenuSystem::render(SDL_Renderer* renderer, int windowWidth) {
    // Draw menu bar background with subtle gradient (full width)
    SDL_SetRenderDrawColor(renderer, 236, 236, 236, 255);
    SDL_Rect menuBarTop = {0, 0, windowWidth, MENU_BAR_HEIGHT / 2};
    SDL_RenderFillRect(renderer, &menuBarTop);

    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_Rect menuBarBottom = {0, MENU_BAR_HEIGHT / 2, windowWidth, MENU_BAR_HEIGHT / 2};
    SDL_RenderFillRect(renderer, &menuBarBottom);

    // Draw menu bar bottom border (full width)
    SDL_SetRenderDrawColor(renderer, 210, 210, 210, 255);
    SDL_Rect menuBarBorder = {0, MENU_BAR_HEIGHT - 1, windowWidth, 1};
    SDL_RenderFillRect(renderer, &menuBarBorder);

    // Draw menu items
    for (const auto &item : menuItems) {
        // Hover effect
        if (item.hoverAnimation > 0.01f || item.isOpen) {
            // Use a stronger highlight for open menus
            Uint8 alpha = item.isOpen ? 100 : (Uint8)(item.hoverAnimation * 40);
            SDL_SetRenderDrawColor(renderer, 0, 120, 215, alpha);
            SDL_Rect hoverRect = {item.x, 2, item.width, MENU_BAR_HEIGHT - 4};
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderFillRect(renderer, &hoverRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        // Calculate text color
        Uint8 textBrightness = (Uint8)(40 + item.hoverAnimation * 20);
        SDL_Color textColor = {textBrightness, textBrightness, textBrightness, 255};

        // Draw text
        renderText(renderer, item.text.c_str(),
                   item.x + (item.width - getTextWidth(item.text)) / 2,
                   (MENU_BAR_HEIGHT - TTF_FontHeight(font)) / 2,
                   textColor);

        // Draw dropdown if menu is open
        if (item.isOpen) {
            int dropdownHeight = item.dropdownItems.size() * DROPDOWN_ITEM_HEIGHT;

            // Draw dropdown background
            SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
            SDL_Rect dropdownRect = {item.x, MENU_BAR_HEIGHT, DROPDOWN_WIDTH, dropdownHeight};
            SDL_RenderFillRect(renderer, &dropdownRect);

            // Draw dropdown border
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_Rect borderRect = {item.x, MENU_BAR_HEIGHT, DROPDOWN_WIDTH, 1}; // Top
            SDL_RenderFillRect(renderer, &borderRect);
            borderRect = {item.x, MENU_BAR_HEIGHT, 1, dropdownHeight}; // Left
            SDL_RenderFillRect(renderer, &borderRect);
            borderRect = {item.x + DROPDOWN_WIDTH - 1, MENU_BAR_HEIGHT, 1, dropdownHeight}; // Right
            SDL_RenderFillRect(renderer, &borderRect);
            borderRect = {item.x, MENU_BAR_HEIGHT + dropdownHeight - 1, DROPDOWN_WIDTH, 1}; // Bottom
            SDL_RenderFillRect(renderer, &borderRect);

            // Draw dropdown items
            for (size_t i = 0; i < item.dropdownItems.size(); i++) {
                const auto &dropdownItem = item.dropdownItems[i];
                int itemY = MENU_BAR_HEIGHT + i * DROPDOWN_ITEM_HEIGHT;

                // Highlight background if hovered
                if (dropdownItem.hoverAnimation > 0.01f) {
                    SDL_SetRenderDrawColor(renderer, 0, 120, 215, (Uint8)(dropdownItem.hoverAnimation * 40));
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_Rect itemRect = {item.x + 1, itemY, DROPDOWN_WIDTH - 2, DROPDOWN_ITEM_HEIGHT};
                    SDL_RenderFillRect(renderer, &itemRect);
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                }

                // Draw separator line between items
                if (i > 0) {
                    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
                    SDL_Rect separatorRect = {item.x + 5, itemY, DROPDOWN_WIDTH - 10, 1};
                    SDL_RenderFillRect(renderer, &separatorRect);
                }

                // Draw item text
                SDL_Color itemColor = {50, 50, 50, 255};
                renderText(renderer, dropdownItem.text.c_str(),
                           item.x + 10, itemY + (DROPDOWN_ITEM_HEIGHT - TTF_FontHeight(font)) / 2,
                           itemColor);
            }
        }
    }
}