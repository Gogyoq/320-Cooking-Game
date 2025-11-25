#include "menu.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

using namespace std;
Menu::Menu(SDLState& state)
    : state(state),
    startButton(80, 280, 600, 100, "Start Game", "", [this]() { onStartClick(); }),
    settingsButton(10, 10, 60, 60, "Settings", "", [this]() { onSettingsClick(); }),
    exitButton(300, 360, 200, 80, "Exit", "", [this]() { onExitClick(); }) {

    loadTextures();
    configureLayout();
}

void Menu::update() {
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
    SDL_Event quitEvent;
    quitEvent.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quitEvent);
}

void Menu::handleEvent(const SDL_Event& event) {
    startButton.handleEvent(event);
    settingsButton.handleEvent(event);
    exitButton.handleEvent(event);
}

Menu::~Menu() {
    cleanupTextures();
}

void Menu::loadTextures() {
    backgroundTexture = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/menu_bg.PNG");

    // Load button textures
    startTexture = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/start_btn.PNG");
    settingTexture = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/setting_btn.PNG");
    exitTexture = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/exit.PNG");

    // Set loaded textures
    startButton.setTexture(startTexture);
    settingsButton.setTexture(settingTexture);
    exitButton.setTexture(exitTexture);



}

void Menu::cleanupTextures() {
    if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
    if (startTexture) SDL_DestroyTexture(startTexture);
    if (settingTexture) SDL_DestroyTexture(settingTexture);
    if (exitTexture) SDL_DestroyTexture(exitTexture);

    backgroundTexture = nullptr;
    startTexture = nullptr;
    settingTexture = nullptr;
    exitTexture = nullptr;
}

void Menu::configureLayout() {
    // No further layout changes are needed since buttons are already configured in their constructors
}


void Menu::render() {
    SDL_Renderer* renderer = state.renderer;

    // Render bg
    if (backgroundTexture) {
        SDL_RenderTexture(renderer, backgroundTexture, nullptr, nullptr);
    }
    else {
        SDL_FRect bg{ 0.0f, 0.0f, static_cast<float>(state.logW), static_cast<float>(state.logH) };
        SDL_SetRenderDrawColor(renderer, 40, 25, 50, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &bg);
    }

    // Render buttons
    startButton.render(state);
    settingsButton.render(state);
    exitButton.render(state);
}