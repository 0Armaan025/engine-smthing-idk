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

    // Animation related variables
    float targetHeight;
    float animationSpeed;
    Uint32 lastFrameTime;
    std::vector<float> itemHoverScale;
    std::vector<float> targetItemHoverScale;

private:
    SDL_Cursor *arrowCursor;
    SDL_Cursor *handCursor;
    SDL_Cursor *resizeCursor; // Added resize cursor

public:
    Toolbar(const std::vector<std::string> &items, int height);
    void render(SDL_Renderer *renderer, TTF_Font *font, int windowWidth, int windowHeight);
    bool handleMouseMotion(int x, int y, int windowHeight);
    bool handleMouseClick(int x, int y, int windowHeight);
    void handleMouseDown(int x, int y, int windowHeight); // New method
    void handleMouseUp();
    std::string chosenItem;
    void update(int windowHeight, int menuBarHeight);
    int getHeight() const { return height; }
    bool isResizingToolbar() const { return isResizing; }
    void setResizing(bool resizing) { isResizing = resizing; }
    void setResizeStartY(int y) { resizeStartY = y; }
    int getResizeStartY() const { return resizeStartY; }
    void adjustHeight(int deltaY);
};