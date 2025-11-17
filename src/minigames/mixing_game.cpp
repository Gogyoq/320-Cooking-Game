#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>
#include "mixing_game.h"
#include "minigame.h"
#include "../data_structs.h"

using namespace std;

MixingGame::MixingGame(SDLState& state, CookingStep step)
    : state(state), step(step), isClicked(false),
    onCooldown(false), clickTime(SDL_GetTicks())
{
    loadTextures(); //Load all textures for this minigame
}

MixingGame::~MixingGame()
{
    cleanup();
}

void MixingGame::render()
{

}

void MixingGame::update()
{

}

void MixingGame::handleEvent(const SDL_Event& event)
{

}

bool MixingGame::isComplete() const
{
    return false;
}

//Update progress based on cuts remaining
void MixingGame::updateProgress()
{

}

//Helper function to get ingredient file path, important that we follow filename conventions
SDL_Texture* MixingGame::getIngrTexture(SDL_Renderer* renderer, Ingredient ingr) {
    string filepath = "src/res/sprites/ingredients/" + ingr.name + ".png";
    SDL_Texture* texture;
    try {
        texture = IMG_LoadTexture(renderer, filepath.c_str());
    }
    catch (int e) {
        cout << "Error loading texture: " << e;
        texture = IMG_LoadTexture(renderer, "src/res/sprites/no_texture.png");
    }
    return texture;
}

//Load textures need for minigame
void MixingGame::loadTextures() {
    
}

void MixingGame::cleanup() {
    for (auto& pair : textures) {
        SDL_DestroyTexture(pair.second);
    }

    textures.clear();
}