// application.cpp
#include "application.h"
#include <iostream>

Application::Application()
    : windowManager("Animation Engine", 1200, 700),
      resourceManager(nullptr),
      menuSystem(nullptr, 1200), // This will be updated after initialization
      toolbar({"Pencil", "Pen", "Rectangle", "Circle", "Eraser", "Redo", "Undo"}, 30),
      running(false),
      appTitle("Animation Engine"),
      lastTime(0),
      panel(1200, 700, 300)
{
}

bool Application::initialize()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize window
    if (!windowManager.initialize())
    {
        return false;
    }

    // Set renderer for resource manager
    resourceManager = ResourceManager(windowManager.getRenderer());

    // Initialize resource manager
    if (!resourceManager.initialize())
    {
        return false;
    }

    TTF_Font *regularFont = resourceManager.loadFont("regular", "OpenSans.ttf", 16);
    if (!regularFont)
    {

        regularFont = resourceManager.loadFont("regular", "Arial.ttf", 14);
        if (!regularFont)
        {
            std::cerr << "Failed to load fallback font! Exiting." << std::endl;
            return false;
        }
    }

    menuSystem = MenuSystem(regularFont, windowManager.getWidth());

    running = true;
    lastTime = SDL_GetTicks();

    return true;
}

void Application::handleEvents()
{
    SDL_Event event;

    int mouseY = 0;

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
                // Update window size
                windowManager.updateSize();
                menuSystem.recalculateMenuPositions();
            }
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
            int mouseX = event.motion.x;
            mouseY = event.motion.y;

            // Handle menu mouse motion
            menuSystem.handleMouseMotion(mouseX, mouseY);

            // Check for toolbar resize hover
            bool inResizeArea = toolbar.handleMouseMotion(mouseX, mouseY, windowManager.getHeight());

            // Set appropriate cursor
            if (inResizeArea)
            {
                SDL_SetCursor(resourceManager.getResizeCursor());
            }
            else
            {
                SDL_SetCursor(resourceManager.getArrowCursor());
            }

            if (toolbar.isResizingToolbar())
            {
                int deltaY = mouseY - toolbar.getResizeStartY();
                toolbar.setResizeStartY(mouseY);
                toolbar.adjustHeight(deltaY);
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            int mouseX = event.button.x;
            int mouseY = event.button.y;

            // Check for toolbar resize
            if (mouseY > windowManager.getHeight() - toolbar.getHeight() - 5 &&
                mouseY < windowManager.getHeight() - toolbar.getHeight() + 5)
            {
                toolbar.setResizing(true);
                toolbar.setResizeStartY(mouseY);
            }
            // Check if click is in top menu area
            else if (mouseY <= menuSystem.getMenuBarHeight())
            {
                menuSystem.handleMouseClick(mouseX, mouseY);
            }
            // Check if click is in toolbar
            else if (mouseY >= windowManager.getHeight() - toolbar.getHeight())
            {
                toolbar.handleMouseClick(mouseX, mouseY, windowManager.getHeight());
            }
            // Handle clicks outside menus - close any open dropdowns
            else
            {
                menuSystem.handleMouseClick(-1, -1); // Pass invalid coordinates to close menus
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            toolbar.handleMouseUp();
        }
    }

    // Handle toolbar resize
}

void Application::update(float deltaTime)
{
    // Update menu animations
    menuSystem.update(deltaTime);

    // Update toolbar
    toolbar.update(windowManager.getHeight(), menuSystem.getMenuBarHeight());
}

void Application::render()
{
    SDL_Renderer *renderer = windowManager.getRenderer();

    // Clear screen with canvas color
    SDL_SetRenderDrawColor(renderer, 250, 250, 250, 255);
    SDL_RenderClear(renderer);

    // Draw menu bar and dropdowns
    menuSystem.render(renderer, windowManager.getWidth());

    // Draw app title in center
    TTF_Font *regularFont = resourceManager.getFont("regular");

    // testing adding the Panel

    panel.render(renderer);
    // TESTING: drawing hi there or we can just make canvas.h

    if (regularFont)
    {

        std::string text = "Hi there, I'm Armaan!";

        int textWidth, textHeight;
        TTF_SizeText(regularFont, text.c_str(), &textWidth, &textHeight);

        int textX = (windowManager.getWidth() - textWidth) / 2;
        int textY = menuSystem.getMenuBarHeight() + (windowManager.getHeight() - menuSystem.getMenuBarHeight() - toolbar.getHeight()) / 2 - (textHeight / 2);

        std::string textKey = "greeting_text";
        SDL_Texture *textTexture = resourceManager.getCachedTexture(textKey);
        if (!textTexture)
        {
            resourceManager.cacheTextTexture(textKey, regularFont, text, {0, 0, 0, 255});
            textTexture = resourceManager.getCachedTexture(textKey);
        }

        if (textTexture)
        {
            SDL_Rect textRect = {
                textX,
                textY,
                textWidth,
                textHeight};
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        }

        int appTitleWidth, appTitleHeight;
        TTF_SizeText(regularFont, appTitle.c_str(), &appTitleWidth, &appTitleHeight);

        int titleX = (windowManager.getWidth() - appTitleWidth) / 2;

        // Check if we have a cached texture, otherwise create one
        std::string titleKey = "app_title";

        SDL_Texture *titleTexture = resourceManager.getCachedTexture(titleKey);
        if (!titleTexture)
        {
            resourceManager.cacheTextTexture(titleKey, regularFont, appTitle, {100, 100, 100, 255});
            titleTexture = resourceManager.getCachedTexture(titleKey);
        }

        if (titleTexture)
        {
            SDL_Rect titleRect = {
                titleX,
                (menuSystem.getMenuBarHeight() - appTitleHeight) / 2,
                appTitleWidth,
                appTitleHeight};
            SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
        }
    }

    // Draw toolbar
    toolbar.render(renderer, resourceManager.getFont("regular"),
                   windowManager.getWidth(), windowManager.getHeight());

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}

void Application::run()
{
    while (running)
    {
        // Calculate delta time for smooth animations
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Handle events
        handleEvents();

        // Update application state
        update(deltaTime);

        // Render frame
        render();
    }
}

void Application::cleanup()
{
}

Application::~Application()
{
    cleanup();
}