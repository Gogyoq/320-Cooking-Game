#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <functional>
#include "button.h"
#include "../image_button.h"
#include "data_structs.h"

using namespace std;

class Menu {
public:
    Menu(SDLState& state);
    ~Menu();
    void render();
    void update();
    void handleEvent(const SDL_Event& event);

private:
    SDLState& state;
    ImageButton startButton;
    ImageButton settingsButton;
    ImageButton exitButton;

    // Textures
    SDL_Texture* backgroundTexture = nullptr;
    SDL_Texture* startTexture = nullptr;
    SDL_Texture* settingTexture = nullptr;
    SDL_Texture* exitTexture = nullptr;

    // Layout / assets
    void configureLayout();
    void loadTextures();
    void cleanupTextures();

    // Render
    void renderBackground();

    void onStartClick();
    void onSettingsClick();
    void onExitClick();
};
