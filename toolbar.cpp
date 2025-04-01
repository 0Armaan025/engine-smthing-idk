#include "toolbar.h"
#include <cmath>

Toolbar::Toolbar(const std::vector<std::string> &items, int height)
    : items(items), height(height), isResizing(false), resizeStartY(0),
      minHeight(20), maxHeight(200), targetHeight(height)
{
    itemRects.resize(items.size());
    itemHovered.resize(items.size(), false);
    chosenItem = items[0]; // Default selected item

    // Cursor initialization
    arrowCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    handCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    resizeCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
}

void Toolbar::render(SDL_Renderer *renderer, TTF_Font *font, int windowWidth, int windowHeight)
{
    // Draw toolbar background with subtle gradient (macOS style)
    SDL_SetRenderDrawColor(renderer, 245, 245, 247, 255);
    SDL_Rect toolBarRect = {0, windowHeight - height, windowWidth, height};
    SDL_RenderFillRect(renderer, &toolBarRect);

    // Draw toolbar top border with subtle line
    SDL_SetRenderDrawColor(renderer, 220, 220, 225, 255);
    SDL_Rect toolBarTopBorder = {0, windowHeight - height, windowWidth, 1};
    SDL_RenderFillRect(renderer, &toolBarTopBorder);

    // Draw toolbar items
    int itemX = 10;
    for (size_t i = 0; i < items.size(); i++)
    {
        // Calculate colors based on hover state with smoother transitions
        SDL_Color textColor;
        SDL_Color bgColor;

        if (items[i] == chosenItem)
        {
            // Selected item styling (macOS blue)
            textColor = {0, 122, 255, 255};
            bgColor = {230, 240, 252, 255};
        }
        else if (itemHovered[i])
        {
            // Hover styling (subtle)
            textColor = {50, 50, 50, 255};
            bgColor = {235, 235, 237, 255};
        }
        else
        {
            // Default styling
            textColor = {80, 80, 80, 255};
            bgColor = {245, 245, 247, 0}; // Transparent background by default (macOS style)
        }

        // Render text without scaling effect (constant size for premium feel)
        SDL_Surface *toolSurface = TTF_RenderText_Blended(font, items[i].c_str(), textColor);
        if (toolSurface)
        {
            int textWidth, textHeight;
            TTF_SizeText(font, items[i].c_str(), &textWidth, &textHeight);

            // Calculate background dimensions (fixed, no scaling)
            int bgWidth = textWidth + 20;
            int bgHeight = textHeight + 10;

            // Render background only for selected or hovered items
            if (itemHovered[i] || items[i] == chosenItem)
            {
                SDL_Rect bgRect = {
                    itemX,
                    windowHeight - height + (height - bgHeight) / 2,
                    bgWidth,
                    bgHeight};

                SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);

                // Draw rounded rectangle effect (macOS style)
                // For actual rounded corners, you'd need a more complex method with SDL2_gfx
                // This is a basic approximation
                SDL_RenderFillRect(renderer, &bgRect);
            }

            // Create texture from surface and render
            SDL_Texture *toolTexture = SDL_CreateTextureFromSurface(renderer, toolSurface);
            if (toolTexture)
            {
                itemRects[i] = {
                    itemX + 10, // Consistent padding
                    windowHeight - height + (height - textHeight) / 2,
                    textWidth,
                    textHeight};

                SDL_RenderCopy(renderer, toolTexture, NULL, &itemRects[i]);
                SDL_DestroyTexture(toolTexture);
            }
            SDL_FreeSurface(toolSurface);

            itemX += textWidth + 30; // Consistent spacing (macOS style)
        }
    }

    // Display selected tool with macOS style
    SDL_Color selectedTextColor = {0, 122, 255, 255}; // macOS blue
    SDL_Surface *selectedSurface = TTF_RenderText_Blended(font,
                                                          ("Selected: " + chosenItem).c_str(), selectedTextColor);

    if (selectedSurface)
    {
        SDL_Texture *selectedTexture = SDL_CreateTextureFromSurface(renderer, selectedSurface);
        if (selectedTexture)
        {
            int textWidth, textHeight;
            TTF_SizeText(font, ("Selected: " + chosenItem).c_str(), &textWidth, &textHeight);

            // Position on right side
            SDL_Rect selectedRect = {
                windowWidth - textWidth - 20,
                windowHeight - height + (height - textHeight) / 2,
                textWidth,
                textHeight};

            // Draw subtle background (macOS style pill shape approximation)
            SDL_SetRenderDrawColor(renderer, 240, 245, 250, 255);
            SDL_Rect selectedBg = {
                selectedRect.x - 10,
                selectedRect.y - 5,
                selectedRect.w + 20,
                selectedRect.h + 10};
            SDL_RenderFillRect(renderer, &selectedBg);

            SDL_RenderCopy(renderer, selectedTexture, NULL, &selectedRect);
            SDL_DestroyTexture(selectedTexture);
        }
        SDL_FreeSurface(selectedSurface);
    }

    // Draw resize handle indicator at the top of toolbar (minimal macOS style)
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);
    SDL_Rect handleLine = {
        windowWidth / 2 - 15,
        windowHeight - height + 5,
        30,
        2};
    SDL_RenderFillRect(renderer, &handleLine);
}

bool Toolbar::handleMouseMotion(int x, int y, int windowHeight)
{
    // Check for toolbar resize hover area
    bool inResizeArea = (y > windowHeight - height - 5 && y < windowHeight - height + 5);

    // Change cursor for resize area
    if (inResizeArea && !isResizing)
    {
        SDL_SetCursor(resizeCursor);
        return true;
    }

    // Check for item hover
    bool anyItemHovered = false;

    for (size_t i = 0; i < items.size(); i++)
    {
        itemHovered[i] = (x > itemRects[i].x - 10 && x < itemRects[i].x + itemRects[i].w + 10 &&
                          y > itemRects[i].y - 5 && y < itemRects[i].y + itemRects[i].h + 5);

        if (itemHovered[i])
        {
            SDL_SetCursor(handCursor);
            anyItemHovered = true;
        }
    }

    // If no items are hovered and not in resize area, set cursor to default
    if (!anyItemHovered && !inResizeArea)
    {
        SDL_SetCursor(arrowCursor);
    }

    return inResizeArea;
}

bool Toolbar::handleMouseClick(int x, int y, int windowHeight)
{
    // Check if click is in toolbar
    if (y >= windowHeight - height)
    {
        // Check for toolbar item clicks
        for (size_t i = 0; i < items.size(); i++)
        {
            if (itemHovered[i])
            {
                chosenItem = items[i]; // Update selected item
                return true;
            }
        }
    }
    return false;
}

void Toolbar::handleMouseDown(int x, int y, int windowHeight)
{
    bool inResizeArea = (y > windowHeight - height - 5 && y < windowHeight - height + 5);

    if (inResizeArea)
    {
        isResizing = true;
        resizeStartY = y;
    }
}

void Toolbar::handleMouseUp()
{
    isResizing = false;
}

void Toolbar::adjustHeight(int deltaY)
{
    // Direct height adjustment without animation
    height -= deltaY;

    // Enforce limits
    if (height < minHeight)
        height = minHeight;
    if (height > maxHeight)
        height = maxHeight;

    // Keep target height in sync with actual height
    targetHeight = height;
}

void Toolbar::update(int windowHeight, int menuBarHeight)
{
    // Update maximum height to 40% of available screen space
    maxHeight = (windowHeight - menuBarHeight) * 0.4;

    if (height > maxHeight)
        height = maxHeight;

    // Keep target height in sync with actual height
    targetHeight = height;
}