#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#undef main

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 400;
const int MENU_BAR_HEIGHT = 26;

struct MenuItem
{
    std::string text;
    int x, width;
    bool isHovered = false;
    float hoverAnimation = 0.0f; // 0.0 to 1.0 for animation smoothness
};

SDL_Texture *renderTextToTexture(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color)
{
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
    if (!surface)
        return nullptr;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color)
{
    SDL_Texture *texture = renderTextToTexture(renderer, font, text, color);
    if (!texture)
        return;

    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect destRect = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
}

int getTextWidth(TTF_Font *font, const std::string &text)
{
    int width, height;
    TTF_SizeText(font, text.c_str(), &width, &height);
    return width;
}

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

    // Menu items
    std::vector<MenuItem> menuItems = {
        {"File", 0, 0},
        {"Edit", 0, 0},
        {"View", 0, 0},
        {"Window", 0, 0},
        {"Help", 0, 0}};

    // Calculate menu item positions and widths
    // Start right after the traffic light buttons with less padding
    int menuX = 10; // Reduced from 20 to align with left edge
    for (auto &item : menuItems)
    {
        item.x = menuX;
        item.width = getTextWidth(regularFont, item.text) + 16; // Slightly less padding for macOS feel
        menuX += item.width;
    }

    // App title
    std::string appTitle = "Animation Engine";
    int appTitleWidth = getTextWidth(regularFont, appTitle);

    // Main loop
    bool running = true;
    SDL_Event event;

    Uint32 lastTime = SDL_GetTicks();
    int actualWindowWidth = WINDOW_WIDTH;
    int actualWindowHeight = WINDOW_HEIGHT;

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
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;

                // Check for menu item hover
                for (auto &item : menuItems)
                {
                    bool hovering = (mouseX >= item.x && mouseX < item.x + item.width &&
                                     mouseY >= 0 && mouseY <= MENU_BAR_HEIGHT);

                    item.isHovered = hovering;
                }
            }
        }

        // Update animations - macOS style (faster appear, slightly slower disappear)
        for (auto &item : menuItems)
        {
            if (item.isHovered)
            {
                // macOS animations appear quickly
                item.hoverAnimation = std::min(1.0f, item.hoverAnimation + deltaTime * 8.0f);
            }
            else
            {
                // but fade out more smoothly
                item.hoverAnimation = std::max(0.0f, item.hoverAnimation - deltaTime * 3.0f);
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Draw menu bar background with subtle gradient (full width)
        SDL_SetRenderDrawColor(renderer, 236, 236, 236, 255);
        SDL_Rect menuBarTop = {0, 0, actualWindowWidth, MENU_BAR_HEIGHT / 2};
        SDL_RenderFillRect(renderer, &menuBarTop);

        SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
        SDL_Rect menuBarBottom = {0, MENU_BAR_HEIGHT / 2, actualWindowWidth, MENU_BAR_HEIGHT / 2};
        SDL_RenderFillRect(renderer, &menuBarBottom);

        // Draw menu bar bottom border (full width)
        SDL_SetRenderDrawColor(renderer, 210, 210, 210, 255);
        SDL_Rect menuBarBorder = {0, MENU_BAR_HEIGHT - 1, actualWindowWidth, 1};
        SDL_RenderFillRect(renderer, &menuBarBorder);

        // Draw menu items
        for (const auto &item : menuItems)
        {
            // More faithful macOS hover effect
            if (item.hoverAnimation > 0.01f)
            {
                // macOS uses a light blue highlight
                SDL_SetRenderDrawColor(renderer, 0, 120, 215, (Uint8)(item.hoverAnimation * 40));
                SDL_Rect hoverRect = {item.x, 2, item.width, MENU_BAR_HEIGHT - 4};
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

                // macOS has rounded rectangles for menu highlights
                // Using pill shape as approximation
                SDL_RenderFillRect(renderer, &hoverRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }

            // Calculate text color (gets slightly brighter on hover - macOS behavior)
            Uint8 textBrightness = (Uint8)(40 + item.hoverAnimation * 20);
            SDL_Color textColor = {textBrightness, textBrightness, textBrightness, 255};

            // Draw text
            renderText(renderer, regularFont, item.text.c_str(),
                       item.x + (item.width - getTextWidth(regularFont, item.text)) / 2,
                       (MENU_BAR_HEIGHT - TTF_FontHeight(regularFont)) / 2,
                       textColor);
        }

        // Draw app title in center (precisely centered regardless of window size)
        int titleX = (actualWindowWidth - appTitleWidth) / 2;
        renderText(renderer, regularFont, appTitle.c_str(), titleX,
                   (MENU_BAR_HEIGHT - TTF_FontHeight(regularFont)) / 2,
                   {100, 100, 100, 255});

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