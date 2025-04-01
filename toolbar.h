#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <vector>
#include <string>

class Toolbar
{
private:
    std::vector<std::string> items;
    std::vector<SDL_Rect> itemRects;
    std::vector<bool> itemHovered;
    int height;
    bool isResizing;
    int resizeStartY;
    int minHeight;
    int maxHeight;

public:
    Toolbar(const std::vector<std::string> &items, int height);

    void render(SDL_Renderer *renderer, TTF_Font *font, int windowWidth, int windowHeight);
    bool handleMouseMotion(int x, int y, int windowHeight);
    bool handleMouseClick(int x, int y, int windowHeight);
    void handleMouseUp();

    void update(int windowHeight, int menuBarHeight);

    int getHeight() const { return height; }
    bool isResizingToolbar() const { return isResizing; }
    void setResizing(bool resizing) { isResizing = resizing; }
    void setResizeStartY(int y) { resizeStartY = y; }
    int getResizeStartY() const { return resizeStartY; }

    void adjustHeight(int deltaY);
};