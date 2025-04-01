#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <string>
#include <unordered_map>
#include <memory>

class ResourceManager
{
private:
    std::unordered_map<std::string, TTF_Font *> fonts;
    std::unordered_map<std::string, SDL_Texture *> textures;
    SDL_Renderer *renderer;

    // System cursors
    SDL_Cursor *arrowCursor;
    SDL_Cursor *resizeCursor;

public:
    ResourceManager(SDL_Renderer *renderer);
    ~ResourceManager();

    bool initialize();
    void cleanup();

    TTF_Font *loadFont(const std::string &fontName, const std::string &path, int size);
    TTF_Font *getFont(const std::string &fontName);

    SDL_Texture *createTextTexture(TTF_Font *font, const std::string &text, SDL_Color color);
    void cacheTextTexture(const std::string &key, TTF_Font *font, const std::string &text, SDL_Color color);
    SDL_Texture *getCachedTexture(const std::string &key);

    SDL_Cursor *getArrowCursor() { return arrowCursor; }
    SDL_Cursor *getResizeCursor() { return resizeCursor; }
};
