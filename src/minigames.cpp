#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "data_structs.h"
#include "minigames.h"

//Cutting Minigame Implementation
CuttingGame::CuttingGame(SDLState& state)
    : state(state), isClicked(false), onCooldown(false), angle(0), clickTime(SDL_GetTicks())
{ 
	loadTextures();
    knifeRect = {
        .x = 400,
        .y = 100,
        .w = 510,
        .h = 300
    };
}

void CuttingGame::render() {
	SDL_Renderer* renderer = state.renderer;

	SDL_RenderTexture(renderer, textures["background"], nullptr, nullptr);
    //SDL_RenderTexture(renderer, textures["knife"], nullptr, &knifeRect);
    SDL_RenderTextureRotated(renderer, textures["knife"], nullptr, &knifeRect, angle, nullptr, SDL_FLIP_NONE);
}

void CuttingGame::update() {
    SDL_HideCursor();

    if (isClicked && !onCooldown) {
        // This code runs once on click, only if not on cooldown
        clickTime = SDL_GetTicks();
        angle = -30;
        onCooldown = true;
        isClicked = false;
    }

    if (onCooldown && SDL_GetTicks() - clickTime >= cooldownDuration) {
        // Cooldown is over
        angle = 0;
        onCooldown = false;
    }
}

void CuttingGame::handleEvent(const SDL_Event& event) {
	float mouseX, mouseY;

    switch (event.type) {
    case SDL_EVENT_MOUSE_MOTION:
        mouseX = event.motion.x;
        mouseY = event.motion.y;
        knifeRect.x = mouseX - knifeRect.w/2;
        knifeRect.y = mouseY - knifeRect.h/2;
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        mouseX = event.button.x;
        mouseY = event.button.y;
        if (event.button.button == SDL_BUTTON_LEFT && !onCooldown) {
            isClicked = true;
        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        mouseX = event.button.x;
        mouseY = event.button.y;
        isClicked = false;
        break;
    }

}

void CuttingGame::onClick()
{

}

void CuttingGame::loadTextures() {
	textures["background"] = IMG_LoadTexture(state.renderer, "src/res/sprites/cutting_game/ai_slop.png");
    textures["knife"] = IMG_LoadTexture(state.renderer, "src/res/sprites/cutting_game/knife.png");
    //SDL_SetTextureScaleMode(textures["knife"], scalemode)
}

void CuttingGame::cleanup() {
	for (auto& pair : textures) {
		SDL_DestroyTexture(pair.second);
	}

	textures.clear();
}

