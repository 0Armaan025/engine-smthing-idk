#include "toolbar.h"

Toolbar::Toolbar(const std::vector<std::string> &items, int height)
    : items(items), height(height), isResizing(false), resizeStartY(0),
      minHeight(20), maxHeight(200)
{

    itemRects.resize(items.size());
    itemHovered.resize(items.size(), false);
}

void Toolbar::render(SDL_Renderer *renderer, TTF_Font *font, int windowWidth, int windowHeight)
{
    // Draw toolbar background
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

        itemX += itemRects[i].w + 20;
    }
}

bool Toolbar::handleMouseMotion(int x, int y, int windowHeight)
{
    // Check for toolbar resize hover area
    bool inResizeArea = (y > windowHeight - height - 5 && y < windowHeight - height + 5);

    // Check for item hover
    for (size_t i = 0; i < items.size(); i++)
    {
        itemHovered[i] = (x > itemRects[i].x && x < itemRects[i].x + itemRects[i].w &&
                          y > itemRects[i].y && y < itemRects[i].y + itemRects[i].h);
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
                // Handle tool selection here
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
    // Make sure toolbar doesn't get too big
    maxHeight = windowHeight - menuBarHeight - 20;
    if (height > maxHeight)
        height = maxHeight;
}