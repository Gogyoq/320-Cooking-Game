#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include "mixing_game.h"
#include "minigame.h"
#include "../data_structs.h"

using namespace std;

namespace {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float SCORE_TIME_LIMIT = 10.0f; // seconds
}

MixingGame::MixingGame(SDLState& state, CookingStep step)
    : state(state),
    step(step),
    // ingr(step.ingredients.empty() ? Ingredient{ "carrot", 1, "" } : step.ingredients[0]),
    bowlRect{ state.logW / 2 - 220.0f, state.logH / 2 - 150.0f, 440.0f, 300.0f },
    ingredientRect{},
    progressBarBG{ state.logW / 2 - 250.0f, 40.0f, 500.0f, 35.0f },
    progressBar{ progressBarBG.x + 5, progressBarBG.y + 5, 0.0f, progressBarBG.h - 10 },
    bowlCenter{ bowlRect.x + bowlRect.w / 2, bowlRect.y + bowlRect.h / 2 - 10 },
    bowlRadius(min(bowlRect.w, bowlRect.h) * 0.35f),
    lastAngle(0.0f),
    progress(0.0f),
    trackingCircle(false),
    startTicks(SDL_GetTicks()),
    completionTicks(0),
    score(0)
{
    loadTextures();

    if (textures[ingr.name]) {
        ingredientRect = getAspectRatioRect(textures[ingr.name], {
            bowlCenter.x - bowlRadius * 0.8f,
            bowlCenter.y - bowlRadius * 0.6f,
            bowlRadius * 1.6f,
            bowlRadius * 1.2f
            });
    }
    else {
        ingredientRect = { bowlCenter.x - bowlRadius * 0.7f, bowlCenter.y - bowlRadius * 0.5f, bowlRadius * 1.4f, bowlRadius };
    }
}

MixingGame::~MixingGame()
{
    cleanup();
}

void MixingGame::render()
{
    SDL_Renderer* renderer = state.renderer;

    if (textures["background"]) {
        SDL_RenderTexture(renderer, textures["background"], nullptr, nullptr);
    }

    if (textures[ingr.name]) {
        SDL_RenderTexture(renderer, textures[ingr.name], nullptr, &ingredientRect);
    }

    SDL_FRect bowlTextureRect = getAspectRatioRect(textures["bowl"], bowlRect);
    if (textures["bowl"]) {
        SDL_RenderTexture(renderer, textures["bowl"], nullptr, &bowlTextureRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 200, 220, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(renderer, &bowlTextureRect);
    }

    SDL_FRect mixArea = { bowlCenter.x - bowlRadius, bowlCenter.y - bowlRadius, bowlRadius * 2, bowlRadius * 2 };
    SDL_SetRenderDrawColor(renderer, 90, 140, 200, 80);
    SDL_RenderRect(renderer, &mixArea);

    SDL_SetRenderDrawColor(renderer, 200, 180, 140, SDL_ALPHA_OPAQUE);
    float spoonLength = bowlRadius * 0.9f;
    float spoonEndX = bowlCenter.x + cos(lastAngle) * spoonLength;
    float spoonEndY = bowlCenter.y + sin(lastAngle) * spoonLength;
    SDL_RenderLine(renderer, (int)bowlCenter.x, (int)bowlCenter.y, (int)spoonEndX, (int)spoonEndY);
    SDL_FRect spoonTip = { spoonEndX - 4, spoonEndY - 4, 8, 8 };
    SDL_RenderFillRect(renderer, &spoonTip);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &progressBarBG);
    SDL_SetRenderDrawColor(renderer, 130, 170, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &progressBar);

    if (state.font) {
        Uint64 nowTicks = completionTicks != 0 ? completionTicks : SDL_GetTicks();
        float elapsedSeconds = static_cast<float>(nowTicks - startTicks) / 1000.0f;
        float remainingSeconds = max(0.0f, SCORE_TIME_LIMIT - elapsedSeconds);

        ostringstream timerStream;
        timerStream << fixed << setprecision(1) << remainingSeconds;
        string timerText = "Time left: " + timerStream.str() + "s";

        int percent = step.duration > 0 ? (int)round((progress / step.duration) * 100.0f) : 100;
        percent = max(0, min(100, percent));
        string text = "Mix progress: " + to_string(percent) + "%";
        SDL_Color textColor = { 0, 0, 0, SDL_ALPHA_OPAQUE };
        SDL_Surface* textSurface = TTF_RenderText_Solid(state.font, text.c_str(), 0, textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

            if (textTexture) {
                float textW, textH;
                SDL_GetTextureSize(textTexture, &textW, &textH);

                SDL_FRect textRect = {
                    progressBarBG.x + (progressBarBG.w - textW) / 2,
                    progressBarBG.y - textH - 6,
                    textW,
                    textH
                };

                SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_DestroySurface(textSurface);
        }

        SDL_Surface* timerSurface = TTF_RenderText_Solid(state.font, timerText.c_str(), 0, textColor);
        if (timerSurface) {
            SDL_Texture* timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface);

            if (timerTexture) {
                float textW, textH;
                SDL_GetTextureSize(timerTexture, &textW, &textH);

                SDL_FRect timerRect = {
                    progressBarBG.x + (progressBarBG.w - textW) / 2,
                    progressBarBG.y + progressBarBG.h + 6,
                    textW,
                    textH
                };

                SDL_RenderTexture(renderer, timerTexture, nullptr, &timerRect);
                SDL_DestroyTexture(timerTexture);
            }
            SDL_DestroySurface(timerSurface);
        }

        string instruct = "Circle the bowl or press Spacebar to mix!";
        SDL_Surface* instructSurface = TTF_RenderText_Solid(state.font, instruct.c_str(), 0, textColor);
        if (instructSurface) {
            SDL_Texture* instructTexture = SDL_CreateTextureFromSurface(renderer, instructSurface);

            if (instructTexture) {
                float textW, textH;
                SDL_GetTextureSize(instructTexture, &textW, &textH);

                SDL_FRect textRect = {
                    bowlRect.x + (bowlRect.w - textW) / 2,
                    bowlRect.y + bowlRect.h + 10,
                    textW,
                    textH
                };

                SDL_RenderTexture(renderer, instructTexture, nullptr, &textRect);
                SDL_DestroyTexture(instructTexture);
            }
            SDL_DestroySurface(instructSurface);
        }
    }
}

void MixingGame::update()
{
    updateProgress();
    if (hasTimeExpired()) {
        trackingCircle = false;
    }
    finalizeScoreIfComplete();
}

void MixingGame::handleEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_EVENT_MOUSE_MOTION: {
        float mouseX = event.motion.x;
        float mouseY = event.motion.y;
        if (isInBowl(mouseX, mouseY) && !isComplete()) {
            float angle = atan2(mouseY - bowlCenter.y, mouseX - bowlCenter.x);
            if (!trackingCircle) {
                trackingCircle = true;
                lastAngle = angle;
            }
            else {
                float delta = angle - lastAngle;
                if (delta > PI) delta -= 2.0f * PI;
                if (delta < -PI) delta += 2.0f * PI;
                float contribution = fabs(delta) / (2.0f * PI);
                if (contribution > 0.0005f) {
                    applyStir(contribution);
                }
                lastAngle = angle;
            }
        }
        else {
            trackingCircle = false;
        }
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        float mouseX = event.button.x;
        float mouseY = event.button.y;
        if (event.button.button == SDL_BUTTON_LEFT && isInBowl(mouseX, mouseY) && !isComplete()) {
            trackingCircle = true;
            lastAngle = atan2(mouseY - bowlCenter.y, mouseX - bowlCenter.x);
        }
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event.button.button == SDL_BUTTON_LEFT) {
            trackingCircle = false;
        }
        break;
    case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_SPACE && !isComplete()) {
            applyStir(0.2f);
        }
        break;
    }
}

bool MixingGame::isComplete() const
{
    return progress >= step.duration || hasTimeExpired();
}

void MixingGame::updateProgress()
{
    float maxWidth = progressBarBG.w - 10;
    float ratio = step.duration > 0 ? min(progress / step.duration, 1.0f) : 1.0f;
    progressBar.x = progressBarBG.x + 5;
    progressBar.y = progressBarBG.y + 5;
    progressBar.w = maxWidth * ratio;
    progressBar.h = progressBarBG.h - 10;
}

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

SDL_FRect MixingGame::getAspectRatioRect(SDL_Texture* texture, const SDL_FRect& targetRect)
{
    if (!texture) {
        return targetRect;
    }

    float textureWidth, textureHeight;
    SDL_GetTextureSize(texture, &textureWidth, &textureHeight);

    float textureAspect = textureWidth / textureHeight;
    float targetAspect = targetRect.w / targetRect.h;

    SDL_FRect resultRect;

    if (textureAspect > targetAspect) {
        resultRect.w = targetRect.w;
        resultRect.h = targetRect.w / textureAspect;
        resultRect.x = targetRect.x;
        resultRect.y = targetRect.y + (targetRect.h - resultRect.h) / 2.0f;
    }
    else {
        resultRect.h = targetRect.h;
        resultRect.w = targetRect.h * textureAspect;
        resultRect.x = targetRect.x + (targetRect.w - resultRect.w) / 2.0f;
        resultRect.y = targetRect.y;
    }

    return resultRect;
}

bool MixingGame::isInBowl(float x, float y) const
{
    float dx = x - bowlCenter.x;
    float dy = y - bowlCenter.y;
    return (dx * dx + dy * dy) <= (bowlRadius * bowlRadius);
}

void MixingGame::applyStir(float amount)
{
    if (isComplete()) return;

    progress = min(step.duration, progress + amount);
    updateProgress();
    finalizeScoreIfComplete();
}

void MixingGame::loadTextures() {
    textures["background"] = IMG_LoadTexture(state.renderer, "src/res/sprites/mixing_game/background_mixing.png");
    textures["bowl"] = IMG_LoadTexture(state.renderer, "src/res/sprites/mixing_game/bowl.png");
    textures[ingr.name] = getIngrTexture(state.renderer, ingr);
}

void MixingGame::cleanup() {
    for (auto& pair : textures) {
        SDL_DestroyTexture(pair.second);
    }

    textures.clear();
}

void MixingGame::finalizeScoreIfComplete()
{
    if (completionTicks != 0 || !isComplete()) {
        return;
    }

    completionTicks = SDL_GetTicks();
    float elapsedSeconds = getElapsedSeconds();
    float clampedTime = min(elapsedSeconds, SCORE_TIME_LIMIT);
    float remaining = max(0.0f, SCORE_TIME_LIMIT - clampedTime);
    score = static_cast<int>(round((remaining / SCORE_TIME_LIMIT) * 100.0f));
    score = max(0, min(100, score));
}

int MixingGame::getScore() const
{
    return score;
}

float MixingGame::getElapsedSeconds() const
{
    Uint64 nowTicks = completionTicks != 0 ? completionTicks : SDL_GetTicks();
    return static_cast<float>(nowTicks - startTicks) / 1000.0f;
}

bool MixingGame::hasTimeExpired() const
{
    return getElapsedSeconds() >= SCORE_TIME_LIMIT;
}