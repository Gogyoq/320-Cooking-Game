#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>
#include "data_structs.h"
#include "minigames.h"

//Helper function to get textures easily
static SDL_Texture* getIngrTexture(SDL_Renderer* renderer, Ingredient ingr) {
    string filepath = "src/res/sprites/ingredients/" + ingr.name + ".png";
    SDL_Texture* texture;
    try {
        texture = IMG_LoadTexture(renderer, filepath.c_str());
    }
    catch (int e){
        cout << "Error loading texture: " << e;
        texture = IMG_LoadTexture(renderer, "src/res/sprites/no_texture.png");
    }
    return texture;
}

//Cutting Minigame Implementation
CuttingGame::CuttingGame(SDLState& state, Ingredient ingr)
    : state(state), ingr(ingr), isClicked(false), 
    onCooldown(false), clickTime(SDL_GetTicks())
{ 
	loadTextures();
    knifeRect = {.x = 400, .y = 80, .w = 5, .h = 250};
    ingrDestRect = { .x = 200, .y = 100, .w = 400, .h = 200 };
    ingrRects.push_back(ingrDestRect);
}

CuttingGame::~CuttingGame()
{
    cleanup();
}

void CuttingGame::render() {
	SDL_Renderer* renderer = state.renderer;

	SDL_RenderTexture(renderer, textures["background"], nullptr, nullptr);
    for (SDL_FRect ingrRect : ingrRects) {
        SDL_RenderFillRect(renderer, &ingrRect);
        //SDL_RenderTexture(renderer, textures[ingr.name], &ingrRect, &ingrRect);
    }
    
    //SDL_RenderFillRect(renderer, &knifeRect);
    SDL_RenderTexture(renderer, textures["knife"], nullptr, &knifeRect);
}

void CuttingGame::update() {
    SDL_HideCursor();

    if (isClicked && !onCooldown) {
        // This code runs once on click, only if not on cooldown
        clickTime = SDL_GetTicks();
        onCooldown = true;
        isClicked = false;
        onClick();
    }

    if (onCooldown && SDL_GetTicks() - clickTime >= cooldownDuration) {
        // Cooldown is over
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

bool CuttingGame::isComplete() const {
    return false;
}

void CuttingGame::onClick() {
    bool knifeOverRect;
    bool cutMade = false;
    int index = 0;
    SDL_FRect leftHalf;
    SDL_FRect rightHalf;
    for (int i = 0; i < ingrRects.size(); i++) {
        SDL_FRect rect = ingrRects[i];
        knifeOverRect = knifeRect.x + knifeRect.w >= rect.x && knifeRect.x <= rect.x + rect.w;
        if (knifeOverRect) {
            cutMade = true;
            index = i;
            float cutPoint = knifeRect.x + (knifeRect.w / 2);

            leftHalf = {
                .x = rect.x - 2,
                .y = rect.y,
                .w = cutPoint - rect.x,
                .h = rect.h
            };

            rightHalf = {
                .x = cutPoint + 2,
                .y = rect.y,
                .w = rect.w + rect.x - cutPoint,
                .h = rect.h
            };
        }
    }

    if (cutMade) {
        ingrRects.erase(ingrRects.begin() + index);
        ingrRects.push_back(leftHalf);
        ingrRects.push_back(rightHalf);
    }
}

void CuttingGame::loadTextures() {
	textures["background"] = IMG_LoadTexture(state.renderer, "src/res/sprites/cutting_game/ai_slop.png");
    textures["knife"] = IMG_LoadTexture(state.renderer, "src/res/sprites/cutting_game/dotted.png");
    textures[ingr.name] = getIngrTexture(state.renderer, ingr);
}

void CuttingGame::cleanup() {
	for (auto& pair : textures) {
		SDL_DestroyTexture(pair.second);
	}

	textures.clear();
}

