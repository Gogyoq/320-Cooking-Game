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
    : state(state), step(step), startTime(SDL_GetTicks()),
    currentTime(SDL_GetTicks()), progressTime(0),
    safeZoneSpeed(100.0f), safeZoneVX(100.0f), safeZoneVY(100.0f),
    dialAngleX(0), dialAngleY(0),
    gameField{450, 40, 250, 250}, safeZone{570, 70, 50, 50}, mouseRect{450, 40, 10, 10},
    progressBar{735, 405, 25, 0}, progressBarBG{730, 40, 35, 370},
    dialRectX{525,325,60,60}, dialRectY{625,325,60,60}, ingrRect{160,90,230,230}
{
    ingr = step.ingredients[0]; //Maybe update this to check if the array is empty later im too lazy
    loadTextures(); //Load all textures for this minigame
    SDL_SetWindowRelativeMouseMode(state.window, true);
}

FryingGame::~FryingGame()
{
    SDL_SetWindowRelativeMouseMode(state.window, false);
    cleanup();
}

void FryingGame::render()
{
    SDL_Renderer* renderer = state.renderer;

    //Render background
    SDL_RenderTexture(renderer, textures["background"], nullptr, nullptr);

    //Render gamefield
    SDL_RenderFillRect(renderer, &gameField);
    //Render safezone
    SDL_SetRenderDrawColor(renderer, 130, 170, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &safeZone);
    //Render mouse rect
    SDL_SetRenderDrawColor(renderer, 255, 170, 130, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &mouseRect);

    //render the progress bar
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &progressBarBG);
    SDL_SetRenderDrawColor(renderer, 255, 170, 130, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &progressBar);

    //Render dials
    SDL_RenderTextureRotated(renderer, textures["dial"], nullptr, &dialRectX, dialAngleX, nullptr, SDL_FLIP_NONE);
    SDL_RenderTextureRotated(renderer, textures["dial"], nullptr, &dialRectY, dialAngleY, nullptr, SDL_FLIP_NONE);

    // Render ingredient texture with aspect ratio preserved
    SDL_FRect aspectRect = getAspectRatioRect(textures[ingr.name], ingrRect);
    SDL_RenderTexture(renderer, textures[ingr.name], nullptr, &aspectRect);
}

void FryingGame::update()
{
    currentTime = SDL_GetTicks(); //update current time
    updateProgress();
    updateSafeZone();
    updateDials();
}

void FryingGame::handleEvent(const SDL_Event& event)
{
    float mouseX, mouseY, tempX, tempY;

    switch (event.type) {
    case SDL_EVENT_MOUSE_MOTION:
        mouseX = event.motion.x;
        mouseY = event.motion.y;
        
        tempX = mouseX - mouseRect.w / 2;
        tempY = mouseY - mouseRect.h / 2;

        if (tempX >= gameField.x && tempX + mouseRect.w <= gameField.x + gameField.w) {
            mouseRect.x = tempX;
        }
        if (tempY >= gameField.y && tempY + mouseRect.h <= gameField.y + gameField.h) {
            mouseRect.y = tempY;
        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        mouseX = event.button.x;
        mouseY = event.button.y;
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        mouseX = event.button.x;
        mouseY = event.button.y;
        break;
    }
}

bool FryingGame::isComplete() const
{
    return progressTime > step.duration;
}

//Update progress based on cuts remaining
void FryingGame::updateProgress()
{
    const float DELTA_TIME = 0.02f; //seconds between an update
    if (!isComplete()) {//Dont update progress if game is over
        if (inSafeZone()) {
            progressTime += DELTA_TIME;
        }
        else if (progressTime - DELTA_TIME / 2 >= 0) {
            progressTime -= DELTA_TIME / 2;
        }
    }

    step.score = min(100, (int)(100 * (progressTime / ((currentTime-startTime) / 1000))));

    float oldBarHeight = progressBar.h;

    int maxHeight = progressBarBG.h - 10; //max height for progress bar
    //Update progress bar as long as it wont go out of bounds
    if (progressBar.h < maxHeight && maxHeight * (progressTime / step.duration) <= maxHeight) {
        progressBar.h = maxHeight * (progressTime / step.duration);
        progressBar.y -= progressBar.h - oldBarHeight;
    }
}

//update size and posistion of safe zone
void FryingGame::updateSafeZone() 
{
    const float DELTA_TIME = 0.02f;
    float elapsedTime = (currentTime - startTime) / 1000.0f;

    // Size shrinks over time with minimum cap
    const float START_SIZE = 80.0f;     // Starting size
    const float MIN_SIZE = 40.0f;       // Minimum size
    const float SHRINK_DURATION = 60.0f; // Time to reach minimum (30 seconds)

    // Linear interpolation from START_SIZE to MIN_SIZE over SHRINK_DURATION
    float t = elapsedTime / SHRINK_DURATION;  // Progress from 0 to 1
    t = std::min(t, 1.0f);  // Clamp to maximum of 1.0
    float newSize = START_SIZE + t * (MIN_SIZE - START_SIZE);  // Lerp formula

    safeZone.w = newSize;
    safeZone.h = newSize;

    // Update speed
    safeZoneSpeed = BASE_SPEED + (elapsedTime * ACCELERATION);
    if (safeZoneSpeed > MAX_SPEED) {
        safeZoneSpeed = MAX_SPEED;
    }

    // Add spin to velocity (rotate by small random amount each frame)
    float spinAngle = ((rand() % 20) - 10) * 3.14159f / 180.0f; // -10 to +10 degrees
    float cosA = cos(spinAngle);
    float sinA = sin(spinAngle);
    float newVX = safeZoneVX * cosA - safeZoneVY * sinA;
    float newVY = safeZoneVX * sinA + safeZoneVY * cosA;
    safeZoneVX = newVX;
    safeZoneVY = newVY;

    // Normalize velocity
    float magnitude = sqrt(safeZoneVX * safeZoneVX + safeZoneVY * safeZoneVY);
    if (magnitude > 0) {
        safeZoneVX = (safeZoneVX / magnitude) * safeZoneSpeed;
        safeZoneVY = (safeZoneVY / magnitude) * safeZoneSpeed;
    }

    // Update position
    safeZone.x += safeZoneVX * DELTA_TIME;
    safeZone.y += safeZoneVY * DELTA_TIME;

    // Bounce off boundaries
    if (safeZone.x < gameField.x) {
        safeZone.x = gameField.x;
        safeZoneVX = -safeZoneVX;
    }
    if (safeZone.x + safeZone.w > gameField.x + gameField.w) {
        safeZone.x = gameField.x + gameField.w - safeZone.w;
        safeZoneVX = -safeZoneVX;
    }
    if (safeZone.y < gameField.y) {
        safeZone.y = gameField.y;
        safeZoneVY = -safeZoneVY;
    }
    if (safeZone.y + safeZone.h > gameField.y + gameField.h) {
        safeZone.y = gameField.y + gameField.h - safeZone.h;
        safeZoneVY = -safeZoneVY;
    }
}

void FryingGame::updateDials()
{
    // Calculate center of gameField
    float centerX = gameField.x + gameField.w / 2.0f;
    float centerY = gameField.y + gameField.h / 2.0f;

    // Calculate mouse position relative to center
    float relativeX = (mouseRect.x - centerX) / (gameField.w / 2.0f);
    float relativeY = (mouseRect.y - centerY) / (gameField.h / 2.0f);

    // Clamp to [-1.0, 1.0] range
    relativeX = std::max(-1.0f, std::min(1.0f, relativeX));
    relativeY = std::max(-1.0f, std::min(1.0f, relativeY));

    // Map to dial angles (-90� to +90�)
    // relativeX = -1.0 (left edge) -> -90�
    // relativeX =  0.0 (center)    ->   0�
    // relativeX = +1.0 (right edge)-> +90�
    dialAngleX = relativeX * 90.0;
    dialAngleY = relativeY * 90.0;
}

bool FryingGame::inSafeZone()
{
    if (mouseRect.x >= safeZone.x && (mouseRect.x + mouseRect.w) <= (safeZone.x + safeZone.w)) {
        if (mouseRect.y >= safeZone.y && (mouseRect.y + mouseRect.h) <= (safeZone.y + safeZone.h)) {
            return true;
        }
    }
    return false;
}

SDL_FRect FryingGame::getAspectRatioRect(SDL_Texture* texture, const SDL_FRect& targetRect)
{
    // Get the original texture dimensions
    float textureWidth, textureHeight;
    SDL_GetTextureSize(texture, &textureWidth, &textureHeight);

    // Calculate aspect ratios
    float textureAspect = textureWidth / textureHeight;
    float targetAspect = targetRect.w / targetRect.h;

    SDL_FRect resultRect;

    // If texture is wider than target, fit to width
    if (textureAspect > targetAspect) {
        resultRect.w = targetRect.w;
        resultRect.h = targetRect.w / textureAspect;
        resultRect.x = targetRect.x;
        resultRect.y = targetRect.y + (targetRect.h - resultRect.h) / 2.0f; // Center vertically
    }
    // If texture is taller than target, fit to height
    else {
        resultRect.h = targetRect.h;
        resultRect.w = targetRect.h * textureAspect;
        resultRect.x = targetRect.x + (targetRect.w - resultRect.w) / 2.0f; // Center horizontally
        resultRect.y = targetRect.y;
    }

    return resultRect;
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
    textures["dial"] = IMG_LoadTexture(state.renderer, "src/res/sprites/frying_game/dial.png");
    textures[ingr.name] = getIngrTexture(state.renderer, ingr);
}

void FryingGame::cleanup() {
    for (auto& pair : textures) {
        SDL_DestroyTexture(pair.second);
    }

    textures.clear();
}