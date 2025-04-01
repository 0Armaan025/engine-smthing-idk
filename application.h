#pragma once

#include "window_manager.h"
#include "resource_manager.h"
#include "toolbar.h"
#include "menu_system.h"
#include "panel.h"
#include <string>

class Application
{
private:
    WindowManager windowManager;
    ResourceManager resourceManager;
    MenuSystem menuSystem;
    Toolbar toolbar;
    Panel panel;

    bool running;
    std::string appTitle;

    Uint32 lastTime;

    // Handle all SDL events
    void handleEvents();

    // Render the application
    void render();

    // Update application state
    void update(float deltaTime);

public:
    Application();
    ~Application();
    bool initialize();
    void run();
    void cleanup();
};
