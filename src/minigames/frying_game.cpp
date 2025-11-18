#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>
#include "frying_game.h"
#include "minigame.h"
#include "../data_structs.h"

using namespace std;

FryingGame::FryingGame(SDLState& state, CookingStep step)
    : state(state), step(step), isClicked(false),
    onCooldown(false), clickTime(SDL_GetTicks())
{
    loadTextures(); //Load all textures for this minigame
}

FryingGame::~FryingGame()
{
    cleanup();
}

void FryingGame::render()
{
    SDL_Renderer* renderer = state.renderer;

    //Render background
    SDL_RenderTexture(renderer, textures["background"], nullptr, nullptr);
}

void FryingGame::update()
{

}

void FryingGame::handleEvent(const SDL_Event& event)
{

}

bool FryingGame::isComplete() const
{
    return false;
}

//Update progress based on cuts remaining
void FryingGame::updateProgress()
{

}

//Helper function to get ingredient file path, important that we follow filename conventions
SDL_Texture* FryingGame::getIngrTexture(SDL_Renderer* renderer, Ingredient ingr) {
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
void FryingGame::loadTextures() {
    textures["background"] = IMG_LoadTexture(state.renderer, "src/res/sprites/frying_game/background_frying.png");
}

void FryingGame::cleanup() {
    for (auto& pair : textures) {
        SDL_DestroyTexture(pair.second);
    }

    textures.clear();
}