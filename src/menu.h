#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <functional>
#include "button.h"
#include "data_structs.h"

using namespace std;

class Menu {
public:
    Menu(SDLState& state);

    void render();
    void update();
    void handleEvent(const SDL_Event& event); 

private:
    SDLState& state;
    Button startButton;
    Button settingsButton;
    Button exitButton;

    void onStartClick();
    void onSettingsClick();
    void onExitClick();
};
