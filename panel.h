#ifndef PANEL_H
#define PANEL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

class Panel
{
public:
    Panel(int screenWidth, int screenHeight, int panelWidth);
    ~Panel();

    void setFont(TTF_Font *newFont);
    void render(SDL_Renderer *renderer);
    void handleEvent(SDL_Event &e);
    void updatePlayback(float deltaTime);

private:
    int screenWidth;
    int screenHeight;
    int panelWidth;
    int panelHeight;

    int scrollOffsetY;
    int scrollOffsetX;

    TTF_Font *font;
    std::vector<std::string> layers;
    std::vector<struct TimelineFrame> frames;
    int activeLayerIndex;

    int currentFrame;
    bool playing;
    float playbackSpeed;
    float playbackTimer;

    struct TimelineFrame
    {
        int row;
        int col;
        std::string name;
        bool keyframe;
        int duration;
        bool selected;
    };

    void renderText(SDL_Renderer *renderer, int xPos, int yPos, const std::string &text, SDL_Color color);
    void renderKeyframe(SDL_Renderer *renderer, int xPos, int yPos, bool isSelected, bool isKeyframe, int duration);
    void renderLayerControl(SDL_Renderer *renderer, int xPos, int yPos, int row, bool isActive);
    void renderTimeline(SDL_Renderer *renderer, int xPos, int yPos, int width);
    void renderPlaybackControls(SDL_Renderer *renderer, int xPos, int yPos, int width);

    bool isInsideRect(int mouseX, int mouseY, const SDL_Rect &rect);

    void addFrame(int row, int col, bool isKeyframe);
    void toggleKeyframe(int row, int col);
    void selectFrame(int row, int col);
    void addLayer();
    void togglePlayback();
};

#endif // PANEL_H