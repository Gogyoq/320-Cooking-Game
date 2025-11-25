#include "image_button.h"
#include <iostream>
#include <string>       
#include <SDL3_image/SDL_image.h>

ImageButton::ImageButton(float x, float y, float w, float h, const std::string& text, const std::string& imagePath, ClickCallback onClick)
    : x(x), y(y), w(w), h(h), text(text), onClick(onClick), isHovered(false), isPressed(false) {
    buttonTexture = loadTexture(imagePath, SDL_GetRenderer(SDL_GetWindowFromID(1))); // Load texture
}

ImageButton::~ImageButton() {
    if (buttonTexture) {
        SDL_DestroyTexture(buttonTexture);
    }
}
SDL_Texture* ImageButton::loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    return texture;
}

bool ImageButton::isPointInside(float px, float py) const {
    return px >= x && px <= x + w && py >= y && py <= y + h;
}
void ImageButton::handleEvent(const SDL_Event& event) {
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
        if (isPressed && isPointInside(mouseX, mouseY)) {
            if (onClick) {
                onClick(); 
            }
        }
        isPressed = false;
        break;
    }
}
void ImageButton::setTexture(SDL_Texture* texture) {
    if (buttonTexture) {
        SDL_DestroyTexture(buttonTexture); 
    }
    buttonTexture = texture; // Set texture
}
void ImageButton::render(SDLState& state) {
    SDL_Renderer* renderer = state.renderer;


    if (buttonTexture) {
        SDL_FRect rect = { x, y, w, h };
        SDL_RenderTexture(renderer, buttonTexture, nullptr, &rect);
    }
    else {
        SDL_FRect rect = { x, y, w, h };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); // Set color to white if fail
        SDL_RenderFillRect(renderer, &rect);

    }
}