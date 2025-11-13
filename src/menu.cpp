#include "menu.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

using namespace std;

// Menu Implementation
Menu::Menu(SDLState& state)
    : state(state),
    startButton(300, 80, 200, 60, "Start Game", [this]() { onStartClick(); }),
    settingsButton(300, 160, 200, 60, "Settings", [this]() { onSettingsClick(); }),
    exitButton(300, 240, 200, 60, "Exit", [this]() { onExitClick(); }) {
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

