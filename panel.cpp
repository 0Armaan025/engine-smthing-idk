#include "panel.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// Timeline frame with additional animation properties
struct TimelineFrame
{
    int row;       // Timeline row (layer)
    int col;       // Timeline column (frame number)
    string name;   // Frame identifier
    bool keyframe; // Whether this is a keyframe
    int duration;  // Frame duration in frames
    bool selected; // Whether this frame is currently selected
};

Panel::Panel(int screenWidth, int screenHeight, int panelWidth)
    : screenWidth(screenWidth), screenHeight(screenHeight),
      panelWidth(panelWidth), scrollOffsetY(0), scrollOffsetX(0), panelHeight(screenHeight),
      font(nullptr), currentFrame(0), playing(false), playbackSpeed(1.0f)
{
    // Initialize with some default layers and frames
    layers = {"Layer 1", "Layer 2", "Layer 3"};
    activeLayerIndex = 0;

    // Add some initial frames
    addFrame(0, 0, true); // Keyframe on Layer 1, Frame 1
    addFrame(0, 3, true); // Keyframe on Layer 1, Frame 4
    addFrame(1, 0, true); // Keyframe on Layer 2, Frame 1
    addFrame(2, 5, true); // Keyframe on Layer 3, Frame 6
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

void Panel::renderKeyframe(SDL_Renderer *renderer, int xPos, int yPos, bool isSelected, bool isKeyframe, int duration)
{
    if (renderer == nullptr)
    {
        return;
    }

    int frameWidth = 15;
    int frameHeight = 25;

    SDL_Rect frameRect = {xPos, yPos, frameWidth * max(1, duration), frameHeight};

    // Frame background color
    if (isSelected)
    {
        // Selected frame - light blue
        SDL_SetRenderDrawColor(renderer, 120, 180, 230, 255);
    }
    else
    {
        // Normal frame - light gray
        SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    }
    SDL_RenderFillRect(renderer, &frameRect);

    // Frame border
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawRect(renderer, &frameRect);

    // Draw keyframe diamond if this is a keyframe
    if (isKeyframe)
    {
        // Draw diamond shape for keyframe
        int centerX = xPos + frameWidth / 2;
        int centerY = yPos + frameHeight / 2;
        int diamondSize = 6;

        SDL_Point diamond[5] = {
            {centerX, centerY - diamondSize}, // Top
            {centerX + diamondSize, centerY}, // Right
            {centerX, centerY + diamondSize}, // Bottom
            {centerX - diamondSize, centerY}, // Left
            {centerX, centerY - diamondSize}  // Back to top to close the shape
        };

        // Fill diamond
        if (isSelected)
        {
            SDL_SetRenderDrawColor(renderer, 50, 120, 180, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        }

        for (int y = centerY - diamondSize; y <= centerY + diamondSize; y++)
        {
            int width = diamondSize - abs(y - centerY);
            SDL_RenderDrawLine(renderer, centerX - width, y, centerX + width, y);
        }
    }

    // Draw duration indicators if duration > 1
    if (duration > 1)
    {
        for (int i = 1; i < duration; i++)
        {
            SDL_RenderDrawLine(renderer, xPos + i * frameWidth, yPos, xPos + i * frameWidth, yPos + frameHeight);
        }
    }
}

void Panel::renderLayerControl(SDL_Renderer *renderer, int xPos, int yPos, int row, bool isActive)
{
    if (renderer == nullptr)
    {
        return;
    }

    int layerHeight = 25;
    int layerWidth = 120;

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

    // Layer name text
    SDL_Color textColor = {50, 50, 50, 255};
    renderText(renderer, xPos + 10, yPos + 5, layers[row], textColor);

    // Visibility and lock icons
    int iconSize = 15;
    int iconY = yPos + 5;

    // Visibility icon (eye)
    SDL_Rect eyeRect = {xPos + layerWidth - iconSize * 2 - 5, iconY, iconSize, iconSize};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &eyeRect);

    // Draw eye shape
    int eyeCenterX = eyeRect.x + eyeRect.w / 2;
    int eyeCenterY = eyeRect.y + eyeRect.h / 2;
    int eyeWidth = 8;
    int eyeHeight = 5;

    // Eye outline
    SDL_RenderDrawLine(renderer, eyeCenterX - eyeWidth / 2, eyeCenterY, eyeCenterX + eyeWidth / 2, eyeCenterY);
    SDL_RenderDrawLine(renderer, eyeCenterX - eyeWidth / 2, eyeCenterY, eyeCenterX - eyeWidth / 2 + 2, eyeCenterY - eyeHeight / 2);
    SDL_RenderDrawLine(renderer, eyeCenterX + eyeWidth / 2, eyeCenterY, eyeCenterX + eyeWidth / 2 - 2, eyeCenterY - eyeHeight / 2);
    SDL_RenderDrawLine(renderer, eyeCenterX - eyeWidth / 2 + 2, eyeCenterY - eyeHeight / 2, eyeCenterX + eyeWidth / 2 - 2, eyeCenterY - eyeHeight / 2);

    // Pupil
    SDL_RenderDrawPoint(renderer, eyeCenterX, eyeCenterY);
    SDL_RenderDrawPoint(renderer, eyeCenterX, eyeCenterY - 1);
    SDL_RenderDrawPoint(renderer, eyeCenterX + 1, eyeCenterY);
    SDL_RenderDrawPoint(renderer, eyeCenterX, eyeCenterY + 1);
    SDL_RenderDrawPoint(renderer, eyeCenterX - 1, eyeCenterY);

    // Lock icon
    SDL_Rect lockRect = {xPos + layerWidth - iconSize - 2, iconY, iconSize, iconSize};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &lockRect);

    // Draw lock shape
    int lockCenterX = lockRect.x + lockRect.w / 2;
    int lockCenterY = lockRect.y + lockRect.h / 2;

    // Lock body
    SDL_Rect lockBody = {lockCenterX - 3, lockCenterY - 1, 6, 7};
    SDL_RenderDrawRect(renderer, &lockBody);

    // Lock arc
    SDL_RenderDrawLine(renderer, lockCenterX - 3, lockCenterY - 1, lockCenterX - 3, lockCenterY - 3);
    SDL_RenderDrawLine(renderer, lockCenterX + 3, lockCenterY - 1, lockCenterX + 3, lockCenterY - 3);
    SDL_RenderDrawLine(renderer, lockCenterX - 3, lockCenterY - 3, lockCenterX + 3, lockCenterY - 3);
}

void Panel::renderTimeline(SDL_Renderer *renderer, int xPos, int yPos, int width)
{
    if (renderer == nullptr)
    {
        return;
    }

    int headerHeight = 25;
    int frameWidth = 15;

    // Render timeline header with frame numbers
    SDL_Rect headerRect = {xPos, yPos, width, headerHeight};
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &headerRect);

    // Draw vertical lines for frame divisions
    int numFrames = width / frameWidth;
    for (int i = 0; i <= numFrames; i++)
    {
        int lineX = xPos + i * frameWidth;

        // Every 5th line is darker and taller
        if (i % 5 == 0)
        {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderDrawLine(renderer, lineX, yPos, lineX, yPos + headerHeight);

            // Draw frame number
            SDL_Color textColor = {50, 50, 50, 255};
            renderText(renderer, lineX + 2, yPos + 2, to_string(i + 1), textColor);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 130, 130, 130, 255);
            SDL_RenderDrawLine(renderer, lineX, yPos + headerHeight / 2, lineX, yPos + headerHeight);
        }
    }

    // Draw playhead position
    int playheadX = xPos + currentFrame * frameWidth;
    SDL_SetRenderDrawColor(renderer, 220, 60, 60, 255);

    // Playhead triangle
    SDL_Point playheadTriangle[3] = {
        {playheadX, yPos},         // Top point
        {playheadX - 5, yPos + 8}, // Bottom left
        {playheadX + 5, yPos + 8}  // Bottom right
    };

    for (int y = playheadTriangle[0].y; y <= playheadTriangle[1].y; y++)
    {
        float progress = (float)(y - playheadTriangle[0].y) / (playheadTriangle[1].y - playheadTriangle[0].y);
        int width = progress * 10;
        SDL_RenderDrawLine(renderer, playheadX - width / 2, y, playheadX + width / 2, y);
    }

    // Playhead line
    SDL_RenderDrawLine(renderer, playheadX, yPos + 8, playheadX, yPos + headerHeight + layers.size() * headerHeight);
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

void Panel::addFrame(int row, int col, bool isKeyframe)
{
    // Check if there's already a frame at this position
    for (auto &frame : frames)
    {
        if (frame.row == row && frame.col == col)
        {
            // Update existing frame
            frame.keyframe = isKeyframe;
            return;
        }
    }

    // Add new frame
    string frameName = "f" + to_string(frames.size() + 1);
    TimelineFrame newFrame = {row, col, frameName, isKeyframe, 1, false};
    // frames.push_back(newFrame);
}

void Panel::toggleKeyframe(int row, int col)
{
    for (auto &frame : frames)
    {
        if (frame.row == row && frame.col == col)
        {
            frame.keyframe = !frame.keyframe;
            return;
        }
    }

    // If no frame exists, create a new keyframe
    addFrame(row, col, true);
}

void Panel::selectFrame(int row, int col)
{
    // Deselect all frames first
    for (auto &frame : frames)
    {
        frame.selected = false;
    }

    // Select the specified frame if it exists
    for (auto &frame : frames)
    {
        if (frame.row == row && frame.col == col)
        {
            frame.selected = true;
            return;
        }
    }
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

            // Find max frame for looping
            int maxFrame = 0;
            for (const auto &frame : frames)
            {
                maxFrame = max(maxFrame, frame.col + frame.duration);
            }

            // Loop back to start if we've reached the end
            if (currentFrame >= maxFrame)
            {
                currentFrame = 0;
            }
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
    int frameWidth = 15;
    int rowHeight = 25;
    int timelineStartY = 40;
    int timelineFramesStartX = screenWidth - panelWidth + layerHeaderWidth;
    int timelineWidth = panelWidth - layerHeaderWidth;

    // Render timeline header with frame numbers
    renderTimeline(renderer, timelineFramesStartX, timelineStartY, timelineWidth);

    // Set clipping rectangle for layer area
    SDL_Rect layerClipRect = {screenWidth - panelWidth, timelineStartY + rowHeight,
                              layerHeaderWidth, panelHeight - timelineStartY - rowHeight - 30};
    SDL_RenderSetClipRect(renderer, &layerClipRect);

    // Render layer controls
    for (size_t i = 0; i < layers.size(); i++)
    {
        int layerY = timelineStartY + rowHeight + i * rowHeight - scrollOffsetY;
        renderLayerControl(renderer, screenWidth - panelWidth, layerY, i, i == activeLayerIndex);
    }

    // Reset clip rectangle for frames area
    SDL_Rect framesClipRect = {timelineFramesStartX, timelineStartY + rowHeight,
                               timelineWidth, panelHeight - timelineStartY - rowHeight - 30};
    SDL_RenderSetClipRect(renderer, &framesClipRect);

    // Render frame grid and keyframes
    int numVisibleCols = timelineWidth / frameWidth;

    // Draw vertical grid lines for all visible columns
    for (int col = 0; col <= numVisibleCols; col++)
    {
        int lineX = timelineFramesStartX + col * frameWidth - scrollOffsetX;

        // Skip if outside visible area
        if (lineX < timelineFramesStartX || lineX > timelineFramesStartX + timelineWidth)
        {
            continue;
        }

        // Every 5th line is darker
        if ((col + scrollOffsetX / frameWidth) % 5 == 0)
        {
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        }

        SDL_RenderDrawLine(renderer, lineX, timelineStartY + rowHeight,
                           lineX, timelineStartY + rowHeight + layers.size() * rowHeight);
    }

    // Draw horizontal grid lines for all layers
    for (size_t i = 0; i <= layers.size(); i++)
    {
        int lineY = timelineStartY + rowHeight + i * rowHeight - scrollOffsetY;

        // Skip if outside visible area
        if (lineY < timelineStartY + rowHeight || lineY > panelHeight - 30)
        {
            continue;
        }

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawLine(renderer, timelineFramesStartX, lineY,
                           screenWidth, lineY);
    }

    // Render all frames
    for (const auto &frame : frames)
    {
        int frameX = timelineFramesStartX + frame.col * frameWidth - scrollOffsetX;
        int frameY = timelineStartY + rowHeight + frame.row * rowHeight - scrollOffsetY;

        // Skip if outside visible area
        if (frameX + frameWidth * frame.duration < timelineFramesStartX ||
            frameX > timelineFramesStartX + timelineWidth ||
            frameY + rowHeight < timelineStartY + rowHeight ||
            frameY > panelHeight - 30)
        {
            continue;
        }

        renderKeyframe(renderer, frameX, frameY, frame.selected, frame.keyframe, frame.duration);
    }

    // Reset clip rectangle and render playback controls at bottom
    SDL_RenderSetClipRect(renderer, NULL);
    renderPlaybackControls(renderer, screenWidth - panelWidth, panelHeight - 30, panelWidth);
}

void Panel::handleEvent(SDL_Event &e)
{
    // Common dimensions
    int layerHeaderWidth = 120;
    int frameWidth = 15;
    int rowHeight = 25;
    int timelineStartY = 40;
    int timelineFramesStartX = screenWidth - panelWidth + layerHeaderWidth;

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

        // Check if click is within panel area
        if (mouseX >= screenWidth - panelWidth && mouseX <= screenWidth)
        {
            // Check if click is in layer control area
            if (mouseX < timelineFramesStartX && mouseY > timelineStartY + rowHeight && mouseY < panelHeight - 30)
            {
                int clickedRow = (mouseY - timelineStartY - rowHeight + scrollOffsetY) / rowHeight;
                if (clickedRow >= 0 && clickedRow < static_cast<int>(layers.size()))
                {
                    activeLayerIndex = clickedRow;
                    cout << "Selected layer: " << layers[clickedRow] << endl;
                }
            }

            // Check if click is in timeline frames area
            else if (mouseX >= timelineFramesStartX && mouseY > timelineStartY + rowHeight && mouseY < panelHeight - 30)
            {
                int clickedRow = (mouseY - timelineStartY - rowHeight + scrollOffsetY) / rowHeight;
                int clickedCol = (mouseX - timelineFramesStartX + scrollOffsetX) / frameWidth;

                if (clickedRow >= 0 && clickedRow < static_cast<int>(layers.size()) && clickedCol >= 0)
                {
                    if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        // Left click selects frame
                        selectFrame(clickedRow, clickedCol);
                        cout << "Selected frame at row " << clickedRow << ", col " << clickedCol << endl;
                    }
                    else if (e.button.button == SDL_BUTTON_RIGHT)
                    {
                        // Right click toggles keyframe
                        toggleKeyframe(clickedRow, clickedCol);
                        cout << "Toggled keyframe at row " << clickedRow << ", col " << clickedCol << endl;
                    }
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

        case SDLK_n: // N key adds new keyframe at current position
            if (activeLayerIndex >= 0 && activeLayerIndex < static_cast<int>(layers.size()))
            {
                toggleKeyframe(activeLayerIndex, currentFrame);
            }
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
        }
    }
}