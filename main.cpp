#include "application.h"
#include <iostream>

#undef main

int main()
{
    Application app;

    if (!app.initialize())
    {
        std::cerr << "Failed to initialize application!" << std::endl;
        return 1;
    }

    app.run();
    app.cleanup();

    return 0;
}