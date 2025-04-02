#include "panel.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

Panel::Panel(int screenWidth, int screenHeight, int panelWidth)
    : screenWidth(screenWidth), screenHeight(screenHeight),
      panelWidth(panelWidth), scrollOffsetY(0), scrollOffsetX(0), panelHeight(screenHeight),
      font(nullptr), currentFrame(0), playing(false), playbackSpeed(1.0f), resizing(false)
{
    // Initialize with some default layers
    layers = {"Layer 1", "Layer 2", "Layer 3"};
    activeLayerIndex = 0;
    columns = {"Frame 1", "Frame 2"}; // Initial columns
}

Panel::~Panel()
{
    // No need to clean up font - ResourceManager will handle it
}

void Panel::setFont(TTF_Font *newFont)
{
    font = newFont;
}

void Panel::renderText(SDL_Renderer *renderer, int xPos, int yPos, const std::string &text, SDL_Color color)
{
    if (renderer == nullptr || font == nullptr)
    {
        return;
    }

    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (textSurface == nullptr)
    {
        cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << endl;
        return;
    }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr)
    {
        cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect renderQuad = {xPos, yPos, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void Panel::renderLayerControl(SDL_Renderer *renderer, int xPos, int yPos, int col, bool isActive)
{
    if (renderer == nullptr)
    {
        return;
    }

    int layerHeight = 25; // Changed to 25 for vertical
    int layerWidth = 120; // chnaged to 120

    SDL_Rect layerRect = {xPos, yPos, layerWidth, layerHeight};

    // Layer background
    if (isActive)
    {
        // Active layer
        SDL_SetRenderDrawColor(renderer, 210, 230, 250, 255);
    }
    else
    {
        // Inactive layer
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    }
    SDL_RenderFillRect(renderer, &layerRect);

    // Layer border
    SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
    SDL_RenderDrawRect(renderer, &layerRect);

    // Layer name text.  y pos changed.
    SDL_Color textColor = {50, 50, 50, 255};
    renderText(renderer, xPos + 5, yPos + 5, layers[col], textColor);

    // Visibility and lock icons. xpos changed.
    int iconSize = 15;
    int iconX = xPos + 90;

    // Visibility icon (eye). y pos changed.
    SDL_Rect eyeRect = {iconX, yPos + 5, iconSize, iconSize};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &eyeRect);

    // Draw eye shape
    int eyeCenterX = eyeRect.x + eyeRect.w / 2;
    int eyeCenterY = eyeRect.y + eyeRect.h / 2;
    int eyeWidth = 8;
    int eyeHeight = 5;

    // Eye outline
    SDL_RenderDrawLine(renderer, eyeCenterX - eyeWidth / 2, eyeCenterY,
                       eyeCenterX + eyeWidth / 2, eyeCenterY);
    SDL_RenderDrawLine(renderer, eyeCenterX - eyeWidth / 2, eyeCenterY,
                       eyeCenterX - eyeWidth / 2 + 2, eyeCenterY - eyeHeight / 2);
    SDL_RenderDrawLine(renderer, eyeCenterX + eyeWidth / 2, eyeCenterY,
                       eyeCenterX + eyeWidth / 2 - 2, eyeCenterY - eyeHeight / 2);
    SDL_RenderDrawLine(renderer, eyeCenterX - eyeWidth / 2 + 2, eyeCenterY - eyeHeight / 2,
                       eyeCenterX + eyeWidth / 2 - 2, eyeCenterY - eyeHeight / 2);

    // Pupil
    SDL_RenderDrawPoint(renderer, eyeCenterX, eyeCenterY);
    SDL_RenderDrawPoint(renderer, eyeCenterX, eyeCenterY - 1);
    SDL_RenderDrawPoint(renderer, eyeCenterX + 1, eyeCenterY);
    SDL_RenderDrawPoint(renderer, eyeCenterX, eyeCenterY + 1);
    SDL_RenderDrawPoint(renderer, eyeCenterX - 1, eyeCenterY);

    // Lock icon. y pos changed.
    SDL_Rect lockRect = {iconX + iconSize + 2, yPos + 5, iconSize, iconSize};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &lockRect);

    // Draw lock shape
    int lockCenterX = lockRect.x + lockRect.w / 2;
    int lockCenterY = lockRect.y + lockRect.h / 2;

    // Lock body
    SDL_Rect lockBody = {lockCenterX - 3, lockCenterY - 1, 6, 7};
    SDL_RenderDrawRect(renderer, &lockBody);

    // Lock arc
    SDL_RenderDrawLine(renderer, lockCenterX - 3, lockCenterY - 1,
                       lockCenterX - 3, lockCenterY - 3);
    SDL_RenderDrawLine(renderer, lockCenterX + 3, lockCenterY - 1,
                       lockCenterX + 3, lockCenterY - 3);
    SDL_RenderDrawLine(renderer, lockCenterX - 3, lockCenterY - 3,
                       lockCenterX + 3, lockCenterY - 3);
}

void Panel::renderTimeline(SDL_Renderer *renderer, int xPos, int yPos, int height)
{
    if (renderer == nullptr)
    {
        return;
    }

    int headerWidth = 120; // Changed to 120
    int frameHeight = 25;  // Changed to 25

    // Render timeline header with frame numbers
    SDL_Rect headerRect = {xPos, yPos, headerWidth, height};
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &headerRect);

    // draw the columns.
    for (size_t i = 0; i < columns.size(); i++)
    {
        int columnX = xPos + i * headerWidth;
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderDrawLine(renderer, columnX, yPos, columnX, yPos + height);
        SDL_Color textColor = {50, 50, 50, 255};
        renderText(renderer, columnX + 5, yPos + 5, columns[i], textColor);
    }
    // Draw horizontal lines for frame divisions
    int numFrames = height / frameHeight;
    for (int i = 0; i <= numFrames; i++)
    {
        int lineY = yPos + i * frameHeight;

        // Every 5th line is darker and taller
        if (i % 5 == 0)
        {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderDrawLine(renderer, xPos, lineY, xPos + panelWidth, lineY); // changed
            // Draw frame number
            SDL_Color textColor = {50, 50, 50, 255};
            renderText(renderer, xPos + 2, lineY + 2, to_string(i + 1), textColor);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 130, 130, 130, 255);
            SDL_RenderDrawLine(renderer, xPos, lineY, xPos + panelWidth, lineY); // changed.
        }
    }

    // Draw playhead position
    int playheadY = yPos + currentFrame * frameHeight;
    SDL_SetRenderDrawColor(renderer, 220, 60, 60, 255);

    // Playhead triangle. x pos changed.
    SDL_Point playheadTriangle[3] = {
        {xPos, playheadY},         // Left point
        {xPos + 8, playheadY - 4}, // Top right
        {xPos + 8, playheadY + 4}  // Bottom right
    };

    for (int x = playheadTriangle[0].x; x <= playheadTriangle[1].x; x++)
    {
        float progress = (float)(x - playheadTriangle[0].x) / (playheadTriangle[1].x - playheadTriangle[0].x);
        int height = progress * 8;
        SDL_RenderDrawLine(renderer, x, playheadY - height, x, playheadY + height);
    }
    // Playhead line
    SDL_RenderDrawLine(renderer, xPos, playheadY, xPos + panelWidth, playheadY); // changed
}

void Panel::renderPlaybackControls(SDL_Renderer *renderer, int xPos, int yPos, int width)
{
    if (renderer == nullptr)
    {
        return;
    }

    int controlHeight = 30;
    SDL_Rect controlRect = {xPos, yPos, width, controlHeight};

    // Background for controls
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderFillRect(renderer, &controlRect);

    // Define button dimensions
    int buttonWidth = 25;
    int buttonHeight = 20;
    int buttonY = yPos + 5;
    int buttonSpacing = 10;
    int currentX = xPos + 10;

    // Rewind button
    SDL_Rect rewindRect = {currentX, buttonY, buttonWidth, buttonHeight};
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &rewindRect);

    // Rewind symbol
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    for (int i = 0; i < 2; i++)
    {
        int triangleX = currentX + 5 + i * 7;
        SDL_Point rewindTriangle[3] = {
            {triangleX + 5, buttonY + 4},               // Top right
            {triangleX, buttonY + buttonHeight / 2},    // Middle left
            {triangleX + 5, buttonY + buttonHeight - 4} // Bottom right
        };

        for (int j = 0; j < 2; j++)
        {
            SDL_RenderDrawLine(renderer,
                               rewindTriangle[j].x, rewindTriangle[j].y,
                               rewindTriangle[j + 1].x, rewindTriangle[j + 1].y);
        }
        SDL_RenderDrawLine(renderer,
                           rewindTriangle[2].x, rewindTriangle[2].y,
                           rewindTriangle[0].x, rewindTriangle[0].y);
    }
    currentX += buttonWidth + buttonSpacing;

    // Play/pause button
    SDL_Rect playRect = {currentX, buttonY, buttonWidth, buttonHeight};
    SDL_SetRenderDrawColor(renderer, playing ? 100, 180, 100, 255 : 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &playRect);

    // Play/pause symbol
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    if (playing)
    {
        // Pause symbol (two vertical bars)
        SDL_Rect pauseBar1 = {currentX + 7, buttonY + 5, 3, buttonHeight - 10};
        SDL_Rect pauseBar2 = {currentX + 15, buttonY + 5, 3, buttonHeight - 10};
        SDL_RenderFillRect(renderer, &pauseBar1);
        SDL_RenderFillRect(renderer, &pauseBar2);
    }
    else
    {
        // Play symbol (triangle)
        SDL_Point playTriangle[3] = {
            {currentX + 7, buttonY + 4},                // Top left
            {currentX + 7, buttonY + buttonHeight - 4}, // Bottom left
            {currentX + 18, buttonY + buttonHeight / 2} // Middle right
        };

        for (int y = playTriangle[0].y; y <= playTriangle[1].y; y++)
        {
            float progress = (float)(y - playTriangle[0].y) / (playTriangle[1].y - playTriangle[0].y);
            int endX = playTriangle[0].x + progress * (playTriangle[2].x - playTriangle[0].x);
            SDL_RenderDrawLine(renderer, playTriangle[0].x, y, endX, y);
        }
    }
    currentX += buttonWidth + buttonSpacing;

    // Forward button
    SDL_Rect forwardRect = {currentX, buttonY, buttonWidth, buttonHeight};
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &forwardRect);

    // Forward symbol
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    for (int i = 0; i < 2; i++)
    {
        int triangleX = currentX + 5 + i * 7;
        SDL_Point forwardTriangle[3] = {
            {triangleX, buttonY + 4},                    // Top left
            {triangleX + 5, buttonY + buttonHeight / 2}, // Middle right
            {triangleX, buttonY + buttonHeight - 4}      // Bottom left
        };

        for (int j = 0; j < 2; j++)
        {
            SDL_RenderDrawLine(renderer,
                               forwardTriangle[j].x, forwardTriangle[j].y,
                               forwardTriangle[j + 1].x, forwardTriangle[j + 1].y);
        }
        SDL_RenderDrawLine(renderer,
                           forwardTriangle[2].x, forwardTriangle[2].y,
                           forwardTriangle[0].x, forwardTriangle[0].y);
    }
    currentX += buttonWidth + buttonSpacing;

    // Frame counter
    string frameText = "Frame: " + to_string(currentFrame + 1);
    SDL_Color textColor = {50, 50, 50, 255};
    renderText(renderer, currentX + 10, buttonY + 3, frameText, textColor);
}

bool Panel::isInsideRect(int mouseX, int mouseY, const SDL_Rect &rect)
{
    return (mouseX >= rect.x && mouseX <= rect.x + rect.w &&
            mouseY >= rect.y && mouseY <= rect.y + rect.h);
}

void Panel::addLayer()
{
    string newLayerName = "Layer " + to_string(layers.size() + 1);
    layers.push_back(newLayerName);
    cout << "Added new layer: " << newLayerName << endl;
}

void Panel::togglePlayback()
{
    playing = !playing;
    cout << (playing ? "Started" : "Stopped") << " animation playback" << endl;
}

void Panel::updatePlayback(float deltaTime)
{
    if (playing)
    {
        playbackTimer += deltaTime;

        // Advance frame at proper speed (assuming 24fps)
        if (playbackTimer >= 1.0f / (24.0f * playbackSpeed))
        {
            currentFrame++;
            playbackTimer = 0;
            // No Frame calculation required.
        }
    }
}

void Panel::render(SDL_Renderer *renderer)
{
    if (renderer == nullptr || font == nullptr)
    {
        return;
    }

    // Draw panel background
    SDL_Rect panelRect = {screenWidth - panelWidth, 0, panelWidth, panelHeight};
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_RenderFillRect(renderer, &panelRect);

    // Set common dimensions
    int layerHeaderWidth = 120;
    int rowHeight = 25;
    int timelineStartY = 40;

    // Render timeline header with frame numbers
    renderTimeline(renderer, screenWidth - panelWidth, timelineStartY, panelHeight - 30);

    // Set clipping rectangle for layer area
    SDL_Rect layerClipRect = {screenWidth - panelWidth, timelineStartY,
                              layerHeaderWidth, panelHeight - timelineStartY - 30};
    SDL_RenderSetClipRect(renderer, &layerClipRect);

    // Render layer controls vertically
    for (size_t i = 0; i < layers.size(); i++)
    {
        int layerY = timelineStartY + i * rowHeight - scrollOffsetY;
        renderLayerControl(renderer, screenWidth - panelWidth, layerY, i, i == activeLayerIndex);
    }

    // Reset clip rectangle and render playback controls at bottom
    SDL_RenderSetClipRect(renderer, NULL);
    renderPlaybackControls(renderer, screenWidth - panelWidth, panelHeight - 30, panelWidth);
    // Render the resize handle
    int resizeHandleWidth = 10;
    int resizeHandleX = screenWidth - panelWidth - resizeHandleWidth;
    int resizeHandleY = 0;
    int resizeHandleHeight = screenHeight;

    SDL_Rect resizeHandleRect = {resizeHandleX, resizeHandleY, resizeHandleWidth, resizeHandleHeight};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Darker color for handle
    SDL_RenderFillRect(renderer, &resizeHandleRect);

    // Draw a few lines on the handle to make it more visible
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    for (int i = 0; i < 3; ++i)
    {
        SDL_RenderDrawLine(renderer, resizeHandleX + 2 + i * 2, resizeHandleY + resizeHandleHeight / 4,
                           resizeHandleX + 2 + i * 2, resizeHandleY + 3 * resizeHandleHeight / 4);
    }
}

void Panel::handleEvent(SDL_Event &e)
{
    // Common dimensions
    int layerHeaderWidth = 120;
    int rowHeight = 25;
    int timelineStartY = 40;
    int resizeHandleWidth = 10;

    // Handle mouse wheel scrolling
    if (e.type == SDL_MOUSEWHEEL)
    {
        // Vertical scrolling with mouse wheel
        if (e.wheel.y != 0)
        {
            int scrollAmount = e.wheel.y * 20;
            scrollOffsetY -= scrollAmount;
            scrollOffsetY = max(0, scrollOffsetY);
        }

        // Horizontal scrolling with Shift + mouse wheel
        if (e.wheel.x != 0 || (SDL_GetModState() & KMOD_SHIFT))
        {
            int scrollAmount = e.wheel.x * 20;
            if (SDL_GetModState() & KMOD_SHIFT)
            {
                scrollAmount = e.wheel.y * 20;
            }

            scrollOffsetX -= scrollAmount;
            scrollOffsetX = max(0, scrollOffsetX);
        }
    }

    // Handle mouse button events
    if (e.type == SDL_MOUSEBUTTONDOWN)
    {
        int mouseX = e.button.x;
        int mouseY = e.button.y;

        // Check for resize handle click
        if (mouseX >= screenWidth - panelWidth - resizeHandleWidth &&
            mouseX <= screenWidth - panelWidth &&
            mouseY >= 0 && mouseY <= screenHeight)
        {
            resizing = true;
            resizeStartX = mouseX;
        }
        // Check if click is within panel area
        else if (mouseX >= screenWidth - panelWidth && mouseX <= screenWidth)
        {
            // Check if click is in layer control area. y check changed.
            if (mouseX < screenWidth - panelWidth + layerHeaderWidth && mouseY > timelineStartY && mouseY < timelineStartY + layers.size() * rowHeight)
            {
                int clickedRow = (mouseY - timelineStartY + scrollOffsetY) / rowHeight;
                if (clickedRow >= 0 && clickedRow < static_cast<int>(layers.size()))
                {
                    activeLayerIndex = clickedRow;
                    cout << "Selected layer: " << layers[clickedRow] << endl;
                }
            }

            // Check if click is in playback controls area
            else if (mouseY >= panelHeight - 30)
            {
                // Play/pause button clicked (just a simple detection for demo)
                int controlCenterX = screenWidth - panelWidth + 45;
                if (abs(mouseX - controlCenterX) < 15)
                {
                    togglePlayback();
                }

                // Rewind button
                int rewindCenterX = screenWidth - panelWidth + 20;
                if (abs(mouseX - rewindCenterX) < 15)
                {
                    currentFrame = 0;
                    cout << "Rewinded to start" << endl;
                }
            }
            // Check if add column button is clicked (assuming it's at the bottom right)
            else if (mouseY >= panelHeight - 30 && mouseX >= screenWidth - panelWidth + layerHeaderWidth - 50)
            { // made it smaller
                addColumn();
            }
        }
    }
    else if (e.type == SDL_MOUSEBUTTONUP)
    {
        resizing = false;
    }
    else if (e.type == SDL_MOUSEMOTION)
    {
        if (resizing)
        {
            int mouseX = e.motion.x;
            int deltaX = mouseX - resizeStartX;
            panelWidth -= deltaX;
            if (panelWidth < 50)
                panelWidth = 50; // Minimum width
            if (panelWidth > screenWidth)
                panelWidth = screenWidth; // maximum width.
            resizeStartX = mouseX;
        }
    }

    // Handle keyboard shortcuts
    if (e.type == SDL_KEYDOWN)
    {
        switch (e.key.keysym.sym)
        {
        case SDLK_SPACE: // Spacebar toggles playback
            togglePlayback();
            break;

        case SDLK_l: // L key adds new layer
            addLayer();
            break;

        case SDLK_RIGHT: // Right arrow moves playhead forward
            currentFrame++;
            break;

        case SDLK_LEFT: // Left arrow moves playhead backward
            if (currentFrame > 0)
                currentFrame--;
            break;
        case SDLK_a:
            addColumn();
            break;
        }
    }
}

void Panel::addColumn()
{
    string newColumnName = "Frame " + to_string(columns.size() + 1);
    columns.push_back(newColumnName);
    cout << "Added column: " << newColumnName << endl;
}
