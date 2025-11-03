#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "data_structs.h"
#include "minigames.h"

//Cutting Minigame Implementation
CuttingGame::CuttingGame(SDLState& state)
	: state(state)
{
	loadTextures();
}

void CuttingGame::render() {
	SDL_Renderer* renderer = state.renderer;

	SDL_RenderTexture(renderer, textures["background"], nullptr, nullptr);
}

void CuttingGame::update() {

}

void CuttingGame::handleEvent(const SDL_Event& event) {

}

void CuttingGame::loadTextures() {
	textures["background"] = IMG_LoadTexture(state.renderer, "src/res/sprites/cutting_game/ai_slop.png");
}

void CuttingGame::cleanup() {
	for (auto& pair : textures) {
		SDL_DestroyTexture(pair.second);
	}

	textures.clear();
}

