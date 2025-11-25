#pragma once
#include "./src/data_structs.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <functional>

class ImageButton {
public:
    using ClickCallback = std::function<void()>;

    ImageButton(float x, float y, float w, float h, const std::string& text, const std::string& imagePath, ClickCallback onClick = nullptr);
    ~ImageButton();

    void setTexture(SDL_Texture* texture);
    void render(SDLState& state);
    void handleEvent(const SDL_Event& event);
    void setCallback(ClickCallback callback);
 

private:
    float x, y, w, h;
    std::string text;
    SDL_Texture* buttonTexture;
    bool isHovered;
    bool isPressed;
    ClickCallback onClick;

    bool isPointInside(float px, float py) const;
    SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer);
};
