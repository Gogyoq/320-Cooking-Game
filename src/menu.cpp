#include "menu.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

using namespace std;

// Menu Implementation
Menu::Menu(SDLState& state)
    : state(state),
    startButton(220, 80, 200, 60, "Start Game", [this]() { onStartClick(); }),
    settingsButton(220, 160, 200, 60, "Settings", [this]() { onSettingsClick(); }),
    exitButton(220, 240, 200, 60, "Exit", [this]() { onExitClick(); }) {
}

void Menu::render() {
    startButton.render(state);
    settingsButton.render(state);
    exitButton.render(state);
}

void Menu::update() {
    //Anything that shouldnt be tied to framerate goes here
}

void Menu::onStartClick() {
    cout << "Start button clicked!" << endl;
    state.gameState = GameState::PLAYING;
}

void Menu::onSettingsClick() {
    cout << "Settings button clicked!" << endl;
    // Open settings menu
}

void Menu::onExitClick() {
    cout << "Exit button clicked!" << endl;
    // Trigger exit event
    SDL_Event quitEvent;
    quitEvent.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quitEvent);
}

void Menu::handleEvent(const SDL_Event& event) {
    startButton.handleEvent(event);
    settingsButton.handleEvent(event);
    exitButton.handleEvent(event);
}

// Button Implementation
Button::Button(float x, float y, float w, float h, const string& text, ClickCallback onClick)
    : x(x), y(y), w(w), h(h), text(text), onClick(onClick), isHovered(false), isPressed(false) {
}

void Button::setCallback(ClickCallback callback) {
    onClick = callback;
}

bool Button::isPointInside(float px, float py) const {
    return px >= x && px <= x + w && py >= y && py <= y + h;
}

void Button::handleEvent(const SDL_Event& event) {
    float mouseX, mouseY;

    switch (event.type) {
    case SDL_EVENT_MOUSE_MOTION:
        mouseX = event.motion.x;
        mouseY = event.motion.y;
        isHovered = isPointInside(mouseX, mouseY);
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        mouseX = event.button.x;
        mouseY = event.button.y;
        if (isPointInside(mouseX, mouseY)) {
            isPressed = true;
        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        mouseX = event.button.x;
        mouseY = event.button.y;
        // Only trigger click if mouse is released over button AND was pressed on it
        if (isPressed && isPointInside(mouseX, mouseY)) {
            if (onClick) {
                onClick();  // Execute callback
            }
        }
        isPressed = false;
        break;
    }
}

void Button::render(SDLState& state) {
    SDL_Renderer* renderer = state.renderer;
    SDL_FRect rect = { x, y, w, h };

    // Set color based on state
    if (isPressed) {
        SDL_SetRenderDrawColor(renderer, 30, 30, 120, SDL_ALPHA_OPAQUE);  // Darker when pressed
    }
    else if (isHovered) {
        SDL_SetRenderDrawColor(renderer, 130, 170, 255, SDL_ALPHA_OPAQUE);  // Lighter when hovered
    }
    else {
        SDL_SetRenderDrawColor(renderer, 100, 150, 255, SDL_ALPHA_OPAQUE);  // Normal
    }

    SDL_RenderFillRect(renderer, &rect);

    // Draw border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &rect);

    // Render text
    if (!text.empty() && state.font) {
        SDL_Color textColor = { 255, 255, 255, SDL_ALPHA_OPAQUE };
        SDL_Surface* textSurface = TTF_RenderText_Solid(state.font, text.c_str(), 0, textColor);

        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

            if (textTexture) {
                float textW, textH;
                SDL_GetTextureSize(textTexture, &textW, &textH);

                SDL_FRect textRect = {
                    x + (w - textW) / 2,
                    y + (h - textH) / 2,
                    textW,
                    textH
                };

                SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_DestroySurface(textSurface);
        }
    }
}
