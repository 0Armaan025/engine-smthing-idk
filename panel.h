// panel.h
#ifndef PANEL_H
#define PANEL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

class Panel
{
public:
    Panel(int screenWidth, int screenHeight, int panelWidth);
    ~Panel();

    void setFont(TTF_Font *font);
    void render(SDL_Renderer *renderer);
    void handleEvent(SDL_Event &e);
    void updatePlayback(float deltaTime);

    void addLayer();
    void togglePlayback();

private:
    int screenWidth;
    int screenHeight;
    int panelWidth;
    int frameIndex;
    int layerIndex;
    int scrollOffsetY;
    int scrollOffsetX;
    int panelHeight;
    TTF_Font *font;
    int currentFrame;
    bool playing;
    float playbackSpeed;
    float playbackTimer;
    std::vector<std::string> layers;
    int activeLayerIndex;
    std::vector<std::string> columns; // Added for frame columns
    bool resizing;
    int resizeStartX;

    void renderText(SDL_Renderer *renderer, int xPos, int yPos, const std::string &text, SDL_Color color);
    void renderLayerControl(SDL_Renderer *renderer, int xPos, int yPos, int col, int layerIndex, int frameIndex, bool isActive);
    void renderTimeline(SDL_Renderer *renderer, int xPos, int yPos, int height);
    void renderPlaybackControls(SDL_Renderer *renderer, int xPos, int yPos, int width);
    bool isInsideRect(int mouseX, int mouseY, const SDL_Rect &rect);
    void addColumn(); // function to add a column
};

#endif // PANEL_H