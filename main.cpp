#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <unordered_map>
#include "menu_system.h"

#undef main

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 700;

// Drawing tools
enum DrawingTool
{
    TOOL_LINE,
    TOOL_PENCIL,
    TOOL_PEN, // Added pen tool
    TOOL_RECTANGLE,
    TOOL_CIRCLE,
    TOOL_ERASER // Added eraser tool
};

// Drawing state
struct DrawingState
{
    DrawingTool currentTool = TOOL_PENCIL;
    SDL_Color currentColor = {0, 0, 0, 255};
    int lineWidth = 2;
    bool isDrawing = false;
    int startX = 0, startY = 0;
    int currentX = 0, currentY = 0;
    std::vector<SDL_Point> pencilPoints;
    std::vector<SDL_FPoint> pencilThickPoints; // For thick lines

    // Store all drawn shapes for persistence
    struct DrawnShape
    {
        DrawingTool tool;
        SDL_Color color;
        int lineWidth;
        int x1, y1, x2, y2;
        std::vector<SDL_FPoint> points; // For pencil/pen
    };
    std::vector<DrawnShape> drawnShapes;
};

void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color)
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

int getTextWidth(TTF_Font *font, const std::string &text)
{
    int width, height;
    TTF_SizeText(font, text.c_str(), &width, &height);
    return width;
}

// Function to draw thick line
void drawThickLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int thickness, SDL_Color color)
{
    // Set color with alpha
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // For very thin lines, use the built-in line function
    if (thickness <= 1)
    {
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        return;
    }

    // For thicker lines, we'll use multiple offset lines
    // Calculate the angle of the line
    float angle = atan2(y2 - y1, x2 - x1);

    // Calculate the offset perpendicular to the line
    float dx = sin(angle) * (thickness / 2.0f);
    float dy = -cos(angle) * (thickness / 2.0f);

    // Draw multiple lines to create thickness
    for (int i = -thickness / 2; i <= thickness / 2; i++)
    {
        float offsetX = dx * (i / (thickness / 2.0f));
        float offsetY = dy * (i / (thickness / 2.0f));
        SDL_RenderDrawLine(renderer,
                           x1 + offsetX, y1 + offsetY,
                           x2 + offsetX, y2 + offsetY);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

// Function to draw thick rectangle
void drawThickRect(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int thickness, SDL_Color color)
{
    // Normalize coordinates
    int minX = std::min(x1, x2);
    int minY = std::min(y1, y2);
    int maxX = std::max(x1, x2);
    int maxY = std::max(y1, y2);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int i = 0; i < thickness; i++)
    {
        SDL_Rect rect = {minX + i, minY + i, maxX - minX - 2 * i, maxY - minY - 2 * i};
        SDL_RenderDrawRect(renderer, &rect);
    }
}

// Function to draw thick circle
void drawThickCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius, int thickness, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Draw multiple circles for thickness
    for (int r = radius - thickness / 2; r <= radius + thickness / 2; r++)
    {
        // Midpoint circle algorithm
        int x = r;
        int y = 0;
        int error = 0;

        while (x >= y)
        {
            SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
            SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
            SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
            SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
            SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);
            SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
            SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);

            if (error <= 0)
            {
                y += 1;
                error += 2 * y + 1;
            }
            else
            {
                x -= 1;
                error -= 2 * x + 1;
            }
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

// Function to draw persistent shapes
void drawPersistentShapes(SDL_Renderer *renderer, const std::vector<DrawingState::DrawnShape> &shapes)
{
    for (const auto &shape : shapes)
    {
        switch (shape.tool)
        {
        case TOOL_PENCIL:
        case TOOL_PEN:
            if (shape.points.size() < 2)
                continue;

            SDL_SetRenderDrawColor(renderer, shape.color.r, shape.color.g, shape.color.b, shape.color.a);

            for (size_t i = 1; i < shape.points.size(); i++)
            {
                if (shape.lineWidth <= 1)
                {
                    SDL_RenderDrawLine(renderer,
                                       shape.points[i - 1].x, shape.points[i - 1].y,
                                       shape.points[i].x, shape.points[i].y);
                }
                else
                {
                    drawThickLine(renderer,
                                  shape.points[i - 1].x, shape.points[i - 1].y,
                                  shape.points[i].x, shape.points[i].y,
                                  shape.lineWidth, shape.color);
                }
            }
            break;

        case TOOL_LINE:
            drawThickLine(renderer, shape.x1, shape.y1, shape.x2, shape.y2, shape.lineWidth, shape.color);
            break;

        case TOOL_RECTANGLE:
            drawThickRect(renderer, shape.x1, shape.y1, shape.x2, shape.y2, shape.lineWidth, shape.color);
            break;

        case TOOL_CIRCLE:
        {
            int dx = shape.x2 - shape.x1;
            int dy = shape.y2 - shape.y1;
            int radius = (int)sqrt(dx * dx + dy * dy);
            drawThickCircle(renderer, shape.x1, shape.y1, radius, shape.lineWidth, shape.color);
            break;
        }
        case TOOL_ERASER:
            // Eraser logic would be complex, implement later
            break;
        }
    }
}

// Main application
int main()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1)
    {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return 1;
    }

    // Create window
    SDL_Window *window = SDL_CreateWindow(
        "Animation Engine",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Load fonts
    TTF_Font *regularFont = TTF_OpenFont("OpenSans.ttf", 14);
    if (!regularFont)
    {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        // Try system font as fallback
        regularFont = TTF_OpenFont("Arial.ttf", 14);
        if (!regularFont)
        {
            std::cerr << "Failed to load fallback font! Exiting." << std::endl;
            return 1;
        }
    }

    // Create menu system
    MenuSystem menuSystem(regularFont, WINDOW_WIDTH);

    // App title
    std::string appTitle = "Animation Engine";
    int appTitleWidth = getTextWidth(regularFont, appTitle);

    // Initialize drawing state
    DrawingState drawingState;

    // Main loop
    bool running = true;
    SDL_Event event;

    Uint32 lastTime = SDL_GetTicks();
    int actualWindowWidth = WINDOW_WIDTH;
    int actualWindowHeight = WINDOW_HEIGHT;

    // Tool bar variables
    int toolBarHeight = 30; // Initial toolbar height
    bool isResizingToolBar = false;
    int resizeStartY;
    std::vector<std::string> toolBarItems = {"Pencil", "Pen", "Rectangle", "Circle", "Eraser", "Red", "Undo"};
    std::vector<SDL_Rect> toolBarItemRects(toolBarItems.size());
    std::vector<bool> toolBarItemHovered(toolBarItems.size(), false);

    while (running)
    {
        // Calculate delta time for smooth animations
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Handle events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                    event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    // Update window size for full-width menu bar
                    SDL_GetWindowSize(window, &actualWindowWidth, &actualWindowHeight);
                    menuSystem.recalculateMenuPositions();
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;

                // Update current drawing position
                drawingState.currentX = mouseX;
                drawingState.currentY = mouseY;

                // Add point to pencil stroke if drawing
                if (drawingState.isDrawing && (drawingState.currentTool == TOOL_PENCIL || drawingState.currentTool == TOOL_PEN))
                {
                    // Only add point if it's different enough from the last one (to reduce points)
                    if (drawingState.pencilThickPoints.empty() ||
                        std::fabs(mouseX - drawingState.pencilThickPoints.back().x) > 1 ||
                        std::fabs(mouseY - drawingState.pencilThickPoints.back().y) > 1)
                    {
                        drawingState.pencilThickPoints.push_back({(float)mouseX, (float)mouseY});
                    }
                }

                // Handle menu mouse motion
                menuSystem.handleMouseMotion(mouseX, mouseY);

                // Check for toolbar resize hover
                if (mouseY > actualWindowHeight - toolBarHeight - 5 && mouseY < actualWindowHeight - toolBarHeight + 5)
                {
                    SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS));
                }
                else
                {
                    SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
                }

                // Check for toolbar item hover
                for (size_t i = 0; i < toolBarItems.size(); i++)
                {
                    toolBarItemHovered[i] = (mouseX > toolBarItemRects[i].x && mouseX < toolBarItemRects[i].x + toolBarItemRects[i].w &&
                                             mouseY > toolBarItemRects[i].y && mouseY < toolBarItemRects[i].y + toolBarItemRects[i].h);
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                // Check if click is in drawing area (below menu bar)
                if (mouseY > menuSystem.getMenuBarHeight() && mouseY < actualWindowHeight - toolBarHeight)
                {
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        drawingState.isDrawing = true;
                        drawingState.startX = mouseX;
                        drawingState.startY = mouseY;

                        if (drawingState.currentTool == TOOL_PENCIL || drawingState.currentTool == TOOL_PEN)
                        {
                            drawingState.pencilThickPoints.clear();
                            drawingState.pencilThickPoints.push_back({(float)mouseX, (float)mouseY});
                        }
                    }
                }
                else if (mouseY > actualWindowHeight - toolBarHeight - 5 && mouseY < actualWindowHeight - toolBarHeight + 5)
                {
                    isResizingToolBar = true;
                    resizeStartY = mouseY;
                }
                else
                {
                    // Handle menu click
                    menuSystem.handleMouseClick(mouseX, mouseY);

                    // Process menu actions
                    for (const auto &item : menuSystem.getMenuItems())
                    {
                        if (item.text == "Tools" && item.isOpen)
                        {
                            for (const auto &dropdownItem : item.dropdownItems)
                            {
                                if (dropdownItem.isHovered)
                                {
                                    if (dropdownItem.text == "Pencil")
                                        drawingState.currentTool = TOOL_PENCIL;
                                    else if (dropdownItem.text == "Line")
                                        drawingState.currentTool = TOOL_LINE;
                                    else if (dropdownItem.text == "Rectangle")
                                        drawingState.currentTool = TOOL_RECTANGLE;
                                    else if (dropdownItem.text == "Circle")
                                        drawingState.currentTool = TOOL_CIRCLE;
                                    else if (dropdownItem.text == "Eraser")
                                        drawingState.currentTool = TOOL_ERASER;
                                }
                            }
                        }
                        // Add other menu actions here (File, Edit, etc.)
                    }

                    // Check for toolbar item clicks
                    for (size_t i = 0; i < toolBarItems.size(); i++)
                    {
                        if (toolBarItemHovered[i])
                        {
                            if (toolBarItems[i] == "Pencil")
                                drawingState.currentTool = TOOL_PENCIL;
                            else if (toolBarItems[i] == "Pen")
                                drawingState.currentTool = TOOL_PEN;
                            else if (toolBarItems[i] == "Rectangle")
                                drawingState.currentTool = TOOL_RECTANGLE;
                            else if (toolBarItems[i] == "Circle")
                                drawingState.currentTool = TOOL_CIRCLE;
                            else if (toolBarItems[i] == "Eraser")
                                drawingState.currentTool = TOOL_ERASER;
                            // Add red and undo logic later
                        }
                    }
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT && drawingState.isDrawing)
                {
                    // Finalize the drawing
                    drawingState.isDrawing = false;

                    // Store the drawn shape for persistence
                    DrawingState::DrawnShape shape;
                    shape.tool = drawingState.currentTool;
                    shape.color = drawingState.currentColor;
                    shape.lineWidth = drawingState.lineWidth;
                    shape.x1 = drawingState.startX;
                    shape.y1 = drawingState.startY;
                    shape.x2 = drawingState.currentX;
                    shape.y2 = drawingState.currentY;

                    if (drawingState.currentTool == TOOL_PENCIL || drawingState.currentTool == TOOL_PEN)
                    {
                        shape.points = drawingState.pencilThickPoints;
                    }

                    drawingState.drawnShapes.push_back(shape);
                }
                isResizingToolBar = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                // Keyboard shortcuts for tools
                switch (event.key.keysym.sym)
                {
                case SDLK_p:
                    drawingState.currentTool = TOOL_PENCIL;
                    break;
                case SDLK_n:
                    drawingState.currentTool = TOOL_PEN;
                    break;
                case SDLK_l:
                    drawingState.currentTool = TOOL_LINE;
                    break;
                case SDLK_r:
                    drawingState.currentTool = TOOL_RECTANGLE;
                    break;
                case SDLK_c:
                    drawingState.currentTool = TOOL_CIRCLE;
                    break;
                case SDLK_e:
                    drawingState.currentTool = TOOL_ERASER;
                    break;
                case SDLK_1:
                    drawingState.lineWidth = 1;
                    break;
                case SDLK_2:
                    drawingState.lineWidth = 2;
                    break;
                case SDLK_3:
                    drawingState.lineWidth = 3;
                    break;
                case SDLK_4:
                    drawingState.lineWidth = 4;
                    break;
                case SDLK_5:
                    // Line width shortcuts (1-5)
                    drawingState.lineWidth = event.key.keysym.sym - SDLK_0;
                    break;
                }
            }
        }

        // Update menu animations
        menuSystem.update(deltaTime);

        // Clear screen with canvas color (slightly off-white for drawing area)
        SDL_SetRenderDrawColor(renderer, 250, 250, 250, 255);
        SDL_RenderClear(renderer);

        // Draw menu bar and dropdowns
        menuSystem.render(renderer, actualWindowWidth);

        // Draw persistent shapes
        drawPersistentShapes(renderer, drawingState.drawnShapes);

        // Draw current shape if drawing
        if (drawingState.isDrawing)
        {
            switch (drawingState.currentTool)
            {
            case TOOL_PENCIL:
            case TOOL_PEN:
                // Draw the current pencil line
                if (drawingState.pencilThickPoints.size() >= 2)
                {
                    for (size_t i = 1; i < drawingState.pencilThickPoints.size(); i++)
                    {
                        drawThickLine(renderer,
                                      drawingState.pencilThickPoints[i - 1].x, drawingState.pencilThickPoints[i - 1].y,
                                      drawingState.pencilThickPoints[i].x, drawingState.pencilThickPoints[i].y,
                                      drawingState.lineWidth, drawingState.currentColor);
                    }
                }
                break;

            case TOOL_LINE:
                drawThickLine(renderer,
                              drawingState.startX, drawingState.startY,
                              drawingState.currentX, drawingState.currentY,
                              drawingState.lineWidth, drawingState.currentColor);
                break;

            case TOOL_RECTANGLE:
                drawThickRect(renderer,
                              drawingState.startX, drawingState.startY,
                              drawingState.currentX, drawingState.currentY,
                              drawingState.lineWidth, drawingState.currentColor);
                break;

            case TOOL_CIRCLE:
            {
                int dx = drawingState.currentX - drawingState.startX;
                int dy = drawingState.currentY - drawingState.startY;
                int radius = (int)sqrt(dx * dx + dy * dy);
                drawThickCircle(renderer,
                                drawingState.startX, drawingState.startY,
                                radius, drawingState.lineWidth, drawingState.currentColor);
                break;
            }
            case TOOL_ERASER:

                break;
            }
        }

        // Draw app title in center
        int titleX = (actualWindowWidth - appTitleWidth) / 2;
        renderText(renderer, regularFont, appTitle.c_str(), titleX,
                   (menuSystem.getMenuBarHeight() - TTF_FontHeight(regularFont)) / 2,
                   {100, 100, 100, 255});

        // Draw tool bar
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_Rect toolBarRect = {0, actualWindowHeight - toolBarHeight, actualWindowWidth, toolBarHeight};
        SDL_RenderFillRect(renderer, &toolBarRect);

        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_Rect toolBarTopBorder = {0, actualWindowHeight - toolBarHeight - 1, actualWindowWidth, 1};
        SDL_RenderFillRect(renderer, &toolBarTopBorder);

        // Draw toolbar items
        int itemX = 10;
        for (size_t i = 0; i < toolBarItems.size(); i++)
        {
            SDL_Color textColor = toolBarItemHovered[i] ? SDL_Color{50, 50, 50, 255} : SDL_Color{80, 80, 80, 255};
            renderText(renderer, regularFont, toolBarItems[i].c_str(), itemX, actualWindowHeight - toolBarHeight + (toolBarHeight - TTF_FontHeight(regularFont)) / 2, textColor);
            toolBarItemRects[i] = {itemX, actualWindowHeight - toolBarHeight, getTextWidth(regularFont, toolBarItems[i]), TTF_FontHeight(regularFont)};
            itemX += toolBarItemRects[i].w + 20;
        }

        // Handle toolbar resize
        if (isResizingToolBar)
        {
            toolBarHeight += resizeStartY - event.motion.y;
            if (toolBarHeight < 20)
                toolBarHeight = 20;
            if (toolBarHeight > actualWindowHeight - menuSystem.getMenuBarHeight() - 20)
                toolBarHeight = actualWindowHeight - menuSystem.getMenuBarHeight() - 20;
            resizeStartY = event.motion.y;
        }

        // Present the rendered frame
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    TTF_CloseFont(regularFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}