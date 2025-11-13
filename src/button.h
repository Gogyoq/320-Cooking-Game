#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <functional>
#include "data_structs.h"

class Button {
public:
    using ClickCallback = function<void()>;

    Button(float x, float y, float w, float h, const string& text, ClickCallback onClick = nullptr);

    void render(SDLState& state);
    void handleEvent(const SDL_Event& event);
    void setCallback(ClickCallback callback);

private:
    float x, y, w, h;
    string text;
    bool isHovered;
    bool isPressed;
    ClickCallback onClick; //What function should be called when clicked

    bool isPointInside(float px, float py) const;
};