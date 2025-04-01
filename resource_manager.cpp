#include "resource_manager.h"
#include <iostream>

ResourceManager::ResourceManager(SDL_Renderer *renderer)
    : renderer(renderer), arrowCursor(nullptr), resizeCursor(nullptr) {}

ResourceManager::~ResourceManager()
{
    cleanup();
}

bool ResourceManager::initialize()
{
    // Initialize TTF
    if (TTF_Init() == -1)
    {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Create system cursors
    arrowCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    resizeCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);

    if (!arrowCursor || !resizeCursor)
    {
        std::cerr << "Failed to create system cursors! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

TTF_Font *ResourceManager::loadFont(const std::string &fontName, const std::string &path, int size)
{
    // First check if we already have this font
    auto it = fonts.find(fontName);
    if (it != fonts.end())
    {
        return it->second;
    }

    // Load the font
    TTF_Font *font = TTF_OpenFont(path.c_str(), size);
    if (!font)
    {
        std::cerr << "Failed to load font " << path << "! TTF_Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    // Store the font
    fonts[fontName] = font;
    return font;
}

TTF_Font *ResourceManager::getFont(const std::string &fontName)
{
    auto it = fonts.find(fontName);
    if (it != fonts.end())
    {
        return it->second;
    }
    return nullptr;
}

SDL_Texture *ResourceManager::createTextTexture(TTF_Font *font, const std::string &text, SDL_Color color)
{
    if (!font)
        return nullptr;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface)
    {
        std::cerr << "Failed to create text surface! TTF_Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture)
    {
        std::cerr << "Failed to create texture from surface! SDL_Error: " << SDL_GetError() << std::endl;
    }

    return texture;
}

void ResourceManager::cacheTextTexture(const std::string &key, TTF_Font *font, const std::string &text, SDL_Color color)
{
    // First check if this texture is already cached
    auto it = textures.find(key);
    if (it != textures.end())
    {
        // Destroy the old texture
        SDL_DestroyTexture(it->second);
    }

    // Create and cache the new texture
    textures[key] = createTextTexture(font, text, color);
}

SDL_Texture *ResourceManager::getCachedTexture(const std::string &key)
{
    auto it = textures.find(key);
    if (it != textures.end())
    {
        return it->second;
    }
    return nullptr;
}

void ResourceManager::cleanup()
{
    // Free all textures
    for (auto &pair : textures)
    {
        if (pair.second)
        {
            SDL_DestroyTexture(pair.second);
        }
    }
    textures.clear();

    // Free all fonts
    for (auto &pair : fonts)
    {
        if (pair.second)
        {
            TTF_CloseFont(pair.second);
        }
    }
    fonts.clear();

    // Free cursors
    if (arrowCursor)
    {
        SDL_FreeCursor(arrowCursor);
        arrowCursor = nullptr;
    }

    if (resizeCursor)
    {
        SDL_FreeCursor(resizeCursor);
        resizeCursor = nullptr;
    }

    // Quit TTF
    TTF_Quit();
}