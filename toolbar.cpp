#include "toolbar.h"

Toolbar::Toolbar(const std::vector<std::string> &items, int height)
    : items(items), height(height), isResizing(false), resizeStartY(0),
      minHeight(20), maxHeight(200)
{

    itemRects.resize(items.size());
    itemHovered.resize(items.size(), false);

    arrowCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    handCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
}

void Toolbar::render(SDL_Renderer *renderer, TTF_Font *font, int windowWidth, int windowHeight)
{

    bool cursorChanged = false;

    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_Rect toolBarRect = {0, windowHeight - height, windowWidth, height};
    SDL_RenderFillRect(renderer, &toolBarRect);

    // Draw toolbar top border
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_Rect toolBarTopBorder = {0, windowHeight - height - 1, windowWidth, 1};
    SDL_RenderFillRect(renderer, &toolBarTopBorder);

    // Draw toolbar items
    int itemX = 10;
    for (size_t i = 0; i < items.size(); i++)
    {

        SDL_Color textColor = itemHovered[i] ? SDL_Color{50, 50, 50, 255} : SDL_Color{80, 80, 80, 255};
        SDL_Color bgColor = !itemHovered[i] ? SDL_Color{240, 240, 240, 255} : SDL_Color{255, 255, 255, 255};

        // render bg color

        SDL_Rect bgRect = {itemX, windowHeight - height, itemRects[i].w + 20, height};
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &bgRect);

        // Render text
        SDL_Surface *toolSurface = TTF_RenderText_Blended(font, items[i].c_str(), textColor);
        if (toolSurface)
        {
            SDL_Texture *toolTexture = SDL_CreateTextureFromSurface(renderer, toolSurface);
            if (toolTexture)
            {
                int textWidth, textHeight;
                TTF_SizeText(font, items[i].c_str(), &textWidth, &textHeight);
                itemRects[i] = {itemX, windowHeight - height + (height - textHeight) / 2, textWidth, textHeight};
                SDL_RenderCopy(renderer, toolTexture, NULL, &itemRects[i]);
                SDL_DestroyTexture(toolTexture);
            }
            SDL_FreeSurface(toolSurface);
        }

        if (itemHovered[i] && !cursorChanged)
        {

            SDL_SetCursor(handCursor);
            cursorChanged = true;
        }

        itemX += itemRects[i].w + 20;
    }

    if (!cursorChanged)
    {
        SDL_SetCursor(arrowCursor);
    }
}

bool Toolbar::handleMouseMotion(int x, int y, int windowHeight)
{
    // Check for toolbar resize hover area
    bool inResizeArea = (y > windowHeight - height - 5 && y < windowHeight - height + 5);

    // Check for item hover
    bool cursorChanged = false; // Flag to check if cursor needs to be updated

    for (size_t i = 0; i < items.size(); i++)
    {
        itemHovered[i] = (x > itemRects[i].x && x < itemRects[i].x + itemRects[i].w &&
                          y > itemRects[i].y && y < itemRects[i].y + itemRects[i].h);

        if (itemHovered[i] && !cursorChanged)
        {
            SDL_SetCursor(handCursor); // Change cursor to hand
            cursorChanged = true;      // Ensure the cursor is only changed once
        }
    }

    // If no items are hovered, set the cursor to default
    if (!cursorChanged)
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

        // update the cursor

        // Check for toolbar item clicks
        for (size_t i = 0; i < items.size(); i++)
        {
            if (itemHovered[i])
            {

                return true;
            }
        }
    }
    return false;
}

void Toolbar::handleMouseUp()
{
    isResizing = false;
}

void Toolbar::adjustHeight(int deltaY)
{
    height -= deltaY;
    if (height < minHeight)
        height = minHeight;
    if (height > maxHeight)
        height = maxHeight;
}

void Toolbar::update(int windowHeight, int menuBarHeight)
{

    maxHeight = windowHeight - menuBarHeight - 20;
    if (height > maxHeight)
        height = maxHeight;
}
