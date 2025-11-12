#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <string>
#include "cutting_game.h"
#include "minigame.h"
#include "../data_structs.h"

using namespace std;

//Cutting Minigame Implementation
CuttingGame::CuttingGame(SDLState& state, CookingStep step)
    : state(state), step(step), isClicked(false),
    onCooldown(false), clickTime(SDL_GetTicks())
{
    ingr = step.ingredients[0]; //Maybe update this to check if the array is empty later im too lazy

    loadTextures(); //Load all textures for this minigame

    knifeRect = { .x = 400, .y = 80, .w = 5, .h = 250 };
    progressBar = { .x = 100, .y = 350, .w = 0, .h = 25 };
    progressBarBG = { .x = 95, .y = 345, .w = 610, .h = 35 };
    Rectangles initRect{
        .destRect = {.x = 200, .y = 100, .w = 400, .h = 200 },
        .sourceRect = {.x = 0, .y = 0, .w = (float)textures[ingr.name]->w, .h = (float)textures[ingr.name]->h}
    };
    ingrRects.push_back(initRect);
}

CuttingGame::~CuttingGame()
{
    cleanup();
}

void CuttingGame::render() {
    SDL_Renderer* renderer = state.renderer;

    //Render background
    SDL_RenderTexture(renderer, textures["background"], nullptr, nullptr);

    //Render each cutup section of the ingredient
    for (const Rectangles& rects : ingrRects) {
        SDL_RenderTexture(renderer, textures[ingr.name], &rects.sourceRect, &rects.destRect);
    }

    //Render the dotted line for the knife
    SDL_RenderTexture(renderer, textures["knife"], nullptr, &knifeRect); //knife is the dotted line

    //render the progress bar
    SDL_RenderFillRect(renderer, &progressBarBG);
    SDL_SetRenderDrawColor(renderer, 130, 170, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &progressBar);

    //Render text
    string text = "Cuts Remaining: " + to_string((int)(step.duration - cutsMade));
    SDL_Color textColor = { 0, 0, 0, SDL_ALPHA_OPAQUE };
    SDL_Surface* textSurface = TTF_RenderText_Solid(state.font, text.c_str(), 0, textColor);
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        if (textTexture) {
            float textW, textH;
            SDL_GetTextureSize(textTexture, &textW, &textH);

            SDL_FRect textRect = {
                progressBarBG.x + (progressBarBG.w - textW)/2,
                progressBarBG.y + 3,
                textW,
                textH
            };

            SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_DestroySurface(textSurface);
    }
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

    updateProgress();
}

void CuttingGame::handleEvent(const SDL_Event& event) {
    float mouseX, mouseY;

    switch (event.type) {
    case SDL_EVENT_MOUSE_MOTION:
        mouseX = event.motion.x;
        mouseY = event.motion.y;
        knifeRect.x = mouseX - knifeRect.w / 2;
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
    return cutsMade >= step.duration;
}

//This function splits the ingredient when called
void CuttingGame::onClick() {
    bool knifeOverRect;
    bool cutMade = false;
    int index = 0;
    SDL_FRect leftDisplayRect, rightDisplayRect;
    SDL_FRect leftSourceRect, rightSourceRect;

    for (int i = 0; i < ingrRects.size(); i++) {
        SDL_FRect rect = ingrRects[i].destRect;
        knifeOverRect = knifeRect.x >= rect.x && knifeRect.x + knifeRect.w <= rect.x + rect.w;

        if (knifeOverRect && !isComplete()) {
            cutMade = true;
            index = i;
            float cutPoint = knifeRect.x + (knifeRect.w / 2);

            // Display rectangles (for positioning)
            leftDisplayRect = {
                .x = rect.x,
                .y = rect.y,
                .w = cutPoint - rect.x,
                .h = rect.h
            };

            rightDisplayRect = {
                .x = cutPoint,
                .y = rect.y,
                .w = rect.w + rect.x - cutPoint,
                .h = rect.h
            };

            // Source texture rectangles (proportional to original)
            float originalTextureWidth = ingrRects[i].sourceRect.w;
            float cutRatio = (cutPoint - rect.x) / rect.w;

            leftSourceRect = {
                .x = ingrRects[i].sourceRect.x,
                .y = ingrRects[i].sourceRect.y,
                .w = originalTextureWidth * cutRatio,
                .h = ingrRects[i].sourceRect.h
            };

            rightSourceRect = {
                .x = ingrRects[i].sourceRect.x + leftSourceRect.w,
                .y = ingrRects[i].sourceRect.y,
                .w = originalTextureWidth * (1.0f - cutRatio),
                .h = ingrRects[i].sourceRect.h
            };

            break;
        }
    }

    if (cutMade) {
        ingrRects.erase(ingrRects.begin() + index);

        Rectangles leftIngredient = {
            .destRect = leftDisplayRect,
            .sourceRect = leftSourceRect
        };
        Rectangles rightIngredient = {
            .destRect = rightDisplayRect,
            .sourceRect = rightSourceRect
        };

        ingrRects.insert(ingrRects.begin() + index, leftIngredient);
        ingrRects.insert(ingrRects.begin() + index + 1, rightIngredient);

        spaceRectangles();
        cutsMade++;
    }
}

//This is to space out each rectangle after a cut
void CuttingGame::spaceRectangles() {
    if (ingrRects.empty()) return;

    const float cutWidth = 3.0f; //Change this to alter cut width

    // Calculate total width needed
    float totalWidth = 0.0f;
    for (const auto& ingredient : ingrRects) {
        totalWidth += ingredient.destRect.w;
    }

    float totalGaps = (ingrRects.size() - 1) * cutWidth;
    float contentWidth = totalWidth + totalGaps;

    float startX = (state.logW - contentWidth) / 2.0f;

    // Reposition display rects only, keep texture source rects intact
    float currentX = startX;
    for (auto& ingredient : ingrRects) {
        ingredient.destRect.x = currentX;
        currentX += ingredient.destRect.w + cutWidth;
    }
}

//Update progress based on cuts remaining
void CuttingGame::updateProgress()
{
    int totalWidth = 600;
    progressBar.w = totalWidth * (cutsMade / step.duration);
}

//Helper function to get ingredient file path, important that we follow filename conventions
SDL_Texture* CuttingGame::getIngrTexture(SDL_Renderer* renderer, Ingredient ingr) {
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