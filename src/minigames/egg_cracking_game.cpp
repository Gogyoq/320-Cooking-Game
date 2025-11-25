#include "egg_cracking_game.h"

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

EggCrackingGame::EggCrackingGame(SDLState& state, const CookingStep& step, Mode mode)
    : state(state),
      step(step),
      mode(mode)
{
    // Normal mode: use step.duration as "number of eggs"
    if (mode == Mode::Normal) {
        totalEggs = static_cast<int>(std::round(step.duration));
        if (totalEggs <= 0) {
            totalEggs = 3;
        }
    } else {
        // Large cap that will never be reached
        totalEggs = 9999;
    }

    currentEgg = 0;

    // Base speed: ~3.2 s per pass (can be tweaked per recipe)
    passDurationMs = 3200;

    // Seed RNG for zone layout
    auto seed = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );
    rng.seed(seed);

    configureLayout();
    loadTextures();

    zoneRects.resize(3);
    zoneHit.assign(3, false);

    countdownStartTick = SDL_GetTicks();
    stateMachine = State::Countdown;
}

EggCrackingGame::~EggCrackingGame() {
    cleanupTextures();
}

bool EggCrackingGame::isComplete() const {
    return stateMachine == State::Done;
}

// -------- Layout & Textures --------

void EggCrackingGame::configureLayout() {
    float screenW = static_cast<float>(state.logW);
    float screenH = static_cast<float>(state.logH);

    // Bar in the lower-middle
    float barWidth = screenW * 0.7f;
    float barHeight = 18.0f;

    barRect.x = (screenW - barWidth) / 2.0f;
    barRect.y = screenH * 0.80f;
    barRect.w = barWidth;
    barRect.h = barHeight;

    // Reticle marker - small vertical rectangle moving along the bar
    float markerWidth  = 10.0f;
    float markerHeight = barHeight + 8.0f; // slightly taller than bar

    markerRect.w = markerWidth;
    markerRect.h = markerHeight;
    markerRect.x = barRect.x;
    markerRect.y = barRect.y - 4.0f; // centered vertically over bar

    // Bowl somewhere under the hand
    float bowlWidth  = screenW * 0.40f;
    float bowlHeight = bowlWidth * 0.6f;
    bowlRect.w = bowlWidth;
    bowlRect.h = bowlHeight;
    bowlRect.x = (screenW - bowlWidth) / 2.0f;
    bowlRect.y = screenH * 0.35f;

    // Hand idle / crack poses
    float handWidth = bowlWidth * 0.9f;
    float handHeight = handWidth * 0.7f;

    handIdlePos.w = handWidth;
    handIdlePos.h = handHeight;
    handIdlePos.x = bowlRect.x + bowlRect.w * 0.05f;
    handIdlePos.y = bowlRect.y - handHeight - 10.0f; // above bowl

    handCrackPos  = handIdlePos;
    handCrackPos.y = bowlRect.y - handHeight * 0.45f; // lower, at rim

    handRect = handIdlePos;

    // Yolk starts above bowl, falls into it (feature currently disabled but kept)
    yolkRect.w = bowlWidth * 0.2f;
    yolkRect.h = yolkRect.w * 0.7f;
    yolkRect.x = bowlRect.x + (bowlRect.w - yolkRect.w) / 2.0f;
    yolkRect.y = handCrackPos.y + handCrackPos.h * 0.4f;
}

void EggCrackingGame::loadTextures() {
    // images placed in src/res/sprites/egg_game/

    texBackground = IMG_LoadTexture(state.renderer, "src/res/sprites/egg_game/bg_kitchen.png");
    texBowl = IMG_LoadTexture(state.renderer, "src/res/sprites/egg_game/bowl.png");
    texHandIdle = IMG_LoadTexture(state.renderer, "src/res/sprites/egg_game/hand_idle.png");
    texHandCrack = IMG_LoadTexture(state.renderer, "src/res/sprites/egg_game/hand_crack.png");
    texYolk = IMG_LoadTexture(state.renderer, "src/res/sprites/egg_game/yolk.png");

    texResult[0] = IMG_LoadTexture(state.renderer, "src/res/sprites/egg_game/egg_result_0.png");
    texResult[1] = IMG_LoadTexture(state.renderer, "src/res/sprites/egg_game/egg_result_1.png");
    texResult[2] = IMG_LoadTexture(state.renderer, "src/res/sprites/egg_game/egg_result_2.png");
    texResult[3] = IMG_LoadTexture(state.renderer, "src/res/sprites/egg_game/egg_result_3.png");
}

void EggCrackingGame::cleanupTextures() {
    if (texBackground) SDL_DestroyTexture(texBackground);
    if (texBowl) SDL_DestroyTexture(texBowl);
    if (texHandIdle) SDL_DestroyTexture(texHandIdle);
    if (texHandCrack) SDL_DestroyTexture(texHandCrack);
    if (texYolk) SDL_DestroyTexture(texYolk);

    for (int i = 0; i < 4; ++i) {
        if (texResult[i]) SDL_DestroyTexture(texResult[i]);
        texResult[i] = nullptr;
    }

    texBackground = nullptr;
    texBowl = nullptr;
    texHandIdle = nullptr;
    texHandCrack = nullptr;
    texYolk = nullptr;
}

// -------- Zone layout --------

void EggCrackingGame::generateZonesForCurrentEgg() {
    const float zoneWidthFrac = 0.10f; // constant sweet spot size
    float zoneWidth = barRect.w * zoneWidthFrac;

    vector<float> centers = {0.2f, 0.5f, 0.8f};
    uniform_real_distribution<float> jitter(-0.08f, 0.08f);

    for (size_t i = 0; i < centers.size(); ++i) {
        float c = centers[i] + jitter(rng);
        c = std::clamp(c, 0.05f, 0.95f);

        float centerX = barRect.x + c * barRect.w;
        SDL_FRect& zr = zoneRects[i];

        zr.w = zoneWidth;
        zr.h = barRect.h + 4.0f;
        zr.x = centerX - zr.w / 2.0f;
        zr.y = barRect.y - 2.0f;
    }

    std::fill(zoneHit.begin(), zoneHit.end(), false);
    pressesThisEgg = 0;
    hitsThisEgg = 0;
    eggFailed = false;

    yolkActive = false;
    yolkSuccessful = false;
}

// -------- Game flow / scoring --------

void EggCrackingGame::startNewEgg() {
    generateZonesForCurrentEgg();
    passStartTick = SDL_GetTicks();
    stateMachine = State::Active;
    crackAnimState = CrackAnimState::Idle;
    handRect = handIdlePos;
    yolkActive = false;
}

void EggCrackingGame::finishCurrentEgg() {
    totalHits += hitsThisEgg;

    if (mode == Mode::Endless) {
        // must hit all zones to survive this egg
        eggFailed = (hitsThisEgg < static_cast<int>(zoneRects.size()));
    } else {
        eggFailed = false; // ignored in Normal mode
    }

    resultStartTick = SDL_GetTicks();
    stateMachine = State::EggResult;
}


void EggCrackingGame::finishMinigame() {
    stateMachine = State::Done;

    if (mode == Mode::Normal) {
        float maxHits = static_cast<float>(totalEggs * pressesPerEgg);
        float ratio = (maxHits > 0.0f)
            ? (static_cast<float>(totalHits) / maxHits)
            : 0.0f;

        // Score out of 100 for results screen
        int rawScore = static_cast<int>(std::round(100.0f * ratio));
        finalScore = std::clamp(rawScore, 0, 100);

        std::cout << "EggCrackingGame (Normal) finished. totalHits=" << totalHits
                  << " maxHits=" << maxHits
                  << " ratio=" << ratio
                  << " finalScore=" << finalScore << std::endl;
    } else {
        // Endless score = how many eggs survived
        finalScore = currentEgg; // number of completed eggs

        std::cout << "EggCrackingGame (Endless) finished. eggs survived="
                  << finalScore << std::endl;
    }
}

// -------- Update --------

void EggCrackingGame::update() {
    uint64_t now = SDL_GetTicks();

    // Update / clean fading hit markers
    updateHitFeedbacks(static_cast<uint32_t>(now));

    switch (stateMachine) {
    case State::Countdown: {
        uint64_t elapsed = now - countdownStartTick;
        if (elapsed >= countdownDurationMs) {
            startNewEgg();
        }
        break;
    }

    case State::Active: {
        uint64_t elapsed = now - passStartTick;

        // Move marker along bar in a single pass
        float t = std::min(1.0f,
            static_cast<float>(elapsed) / static_cast<float>(passDurationMs));
        markerRect.x = barRect.x + t * (barRect.w - markerRect.w);

        // Update crack animation while active
        updateCrackAnimation(now);

        if (elapsed >= passDurationMs) {
            finishCurrentEgg();
        }
        break;
    }

    case State::EggResult: {
        updateCrackAnimation(now); // continue animation if still playing
        uint64_t elapsed = now - resultStartTick;

        if (elapsed >= resultDurationMs) {
            if (mode == Mode::Normal) {
                // Step through fixed egg count
                currentEgg++;
                if (currentEgg >= totalEggs) {
                    finishMinigame();
                } else {
                    countdownStartTick = SDL_GetTicks();
                    stateMachine = State::Countdown;
                    crackAnimState = CrackAnimState::Idle;
                    handRect = handIdlePos;
                    yolkActive = false;
                }
            } else { // Endless
                if (eggFailed) {
                    // Missed egg, then run ends
                    finishMinigame();
                } else {
                    // Survived egg, then increase speed & keep going
                    currentEgg++;

                    // Speed up but clamp
                    passDurationMs = std::max(
                        minPassDurationMs,
                        static_cast<uint32_t>(passDurationMs * speedMultiplier)
                    );

                    countdownStartTick = SDL_GetTicks();
                    stateMachine = State::Countdown;
                    crackAnimState = CrackAnimState::Idle;
                    handRect = handIdlePos;
                    yolkActive = false;
                }
            }
        }
        break;
    }

    case State::Done:
        // LevelManager will check isComplete() and move on
        break;
    }
}

void EggCrackingGame::updateHitFeedbacks(uint32_t now) {
    // Remove any feedback whose lifetime has expired
    auto it = std::remove_if(
        hitFeedbacks.begin(),
        hitFeedbacks.end(),
        [now](const HitFeedback& fb) {
            return now - fb.spawnTime >= fb.lifetimeMs;
        }
    );
    hitFeedbacks.erase(it, hitFeedbacks.end());
}

void EggCrackingGame::updateCrackAnimation(uint64_t now) {
    const uint32_t downDuration = 90;   // ms
    const uint32_t impactDuration = 90;   // ms
    const uint32_t upDuration = 120;  // ms

    switch (crackAnimState) {
    case CrackAnimState::Idle:
        handRect = handIdlePos;
        break;

    case CrackAnimState::CrackDown: {
        uint64_t elapsed = now - crackAnimStartMs;
        float t = std::min(1.0f, elapsed / static_cast<float>(downDuration));
        // LERP hand from idle to crack pose
        handRect.x = handIdlePos.x + (handCrackPos.x - handIdlePos.x) * t;
        handRect.y = handIdlePos.y + (handCrackPos.y - handIdlePos.y) * t;

        if (elapsed >= downDuration) {
            crackAnimState = CrackAnimState::Impact;
            crackAnimStartMs = now;
        }
        break;
    }

    case CrackAnimState::Impact: {
        handRect = handCrackPos;

        uint64_t elapsed = now - crackAnimStartMs;
        if (elapsed >= impactDuration) {
            crackAnimState = CrackAnimState::CrackUp;
            crackAnimStartMs = now;
        }
        break;
    }

    case CrackAnimState::CrackUp: {
        uint64_t elapsed = now - crackAnimStartMs;
        float t = std::min(1.0f, elapsed / static_cast<float>(upDuration));

        handRect.x = handCrackPos.x + (handIdlePos.x - handCrackPos.x) * t;
        handRect.y = handCrackPos.y + (handIdlePos.y - handCrackPos.y) * t;

        if (elapsed >= upDuration) {
            crackAnimState = CrackAnimState::Idle;
            handRect = handIdlePos;
        }
        break;
    }
    }
}

// -------- Input --------

void EggCrackingGame::handleEvent(const SDL_Event& event) {
    if (stateMachine != State::Active) return;

    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE) {
        handleSpacePress();
    }
}

void EggCrackingGame::handleSpacePress() {
    if (pressesThisEgg >= pressesPerEgg) {
        return;
    }

    pressesThisEgg++;

    bool hitZone = false;
    for (size_t i = 0; i < zoneRects.size(); ++i) {
        if (zoneHit[i]) continue;

        if (markerInZone(static_cast<int>(i))) {
            zoneHit[i] = true;
            hitsThisEgg++;
            hitZone = true;
            break;
        }
    }

    // Fading marker for hit or miss
    {
        HitFeedback fb;
        float centerX = markerRect.x + markerRect.w * 0.5f;
        fb.rect.w = markerRect.w * 1.2f;
        fb.rect.h = barRect.h + 8.0f;
        fb.rect.x = centerX - fb.rect.w * 0.5f;
        fb.rect.y = barRect.y - 4.0f;  // centered on bar

        if (hitZone) {
            fb.color = SDL_Color{ 255, 10, 225, SDL_ALPHA_OPAQUE};  // pink for good (temporary colours)
        } else {
            fb.color = SDL_Color{255, 80, 80, 220};    // red for miss
        }

        fb.spawnTime  = SDL_GetTicks();
        fb.lifetimeMs = 400; // fades over 0.4s

        hitFeedbacks.push_back(fb);
    }

    // Trigger crack animation
    crackAnimState = CrackAnimState::CrackDown;
    crackAnimStartMs = SDL_GetTicks();

    // If we've already used all presses or hit all zones, end this egg
    if (hitsThisEgg >= static_cast<int>(zoneRects.size()) ||
        pressesThisEgg >= pressesPerEgg) {
        finishCurrentEgg();
    }
}

bool EggCrackingGame::markerInZone(int zoneIndex) const {
    const SDL_FRect& zr = zoneRects[zoneIndex];

    float markerLeft = markerRect.x;
    float markerRight = markerRect.x + markerRect.w;
    float zoneLeft = zr.x;
    float zoneRight = zr.x + zr.w;

    bool overlap = !(markerRight < zoneLeft || markerLeft > zoneRight);
    return overlap;
}

// -------- Render --------

void EggCrackingGame::render() {
    renderBackground();
    renderKitchen();
    renderBar();
    renderZones();
    renderHitFeedback(); // draw fading hit markers on top of zones
    renderMarker();
    renderUI();

    switch (stateMachine) {
    case State::Countdown:
        renderCountdown();
        break;
    case State::EggResult:
        renderEggResultOverlay();
        break;
    case State::Done:
        renderDoneText();
        break;
    default:
        break;
    }
}

void EggCrackingGame::renderBackground() {
    SDL_Renderer* renderer = state.renderer;

    if (texBackground) {
        SDL_RenderTexture(renderer, texBackground, nullptr, nullptr);
        return;
    }

    // Fallback to solid color
    SDL_FRect bg{
        0.0f, 0.0f,
        static_cast<float>(state.logW),
        static_cast<float>(state.logH)
    };
    SDL_SetRenderDrawColor(renderer, 40, 25, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &bg);
}

void EggCrackingGame::renderKitchen() {
    SDL_Renderer* renderer = state.renderer;

    // Bowl
    if (texBowl) {
        SDL_RenderTexture(renderer, texBowl, nullptr, &bowlRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 120, 80, 80, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &bowlRect);
    }

    // Hand
    SDL_Texture* handTex = nullptr;
    if (crackAnimState == CrackAnimState::CrackDown ||
        crackAnimState == CrackAnimState::Impact ||
        crackAnimState == CrackAnimState::CrackUp) {
        handTex = texHandCrack ? texHandCrack : texHandIdle;
    } else {
        handTex = texHandIdle;
    }

    if (handTex) {
        SDL_RenderTexture(renderer, handTex, nullptr, &handRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 200, 200, 220, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &handRect);
    }
}

void EggCrackingGame::renderBar() {
    SDL_Renderer* renderer = state.renderer;

    SDL_FRect rect = barRect;

    // Fill
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 200);
    SDL_RenderFillRect(renderer, &rect);

    // Outline
    SDL_SetRenderDrawColor(renderer, 220, 220, 240, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer, &rect);
}

void EggCrackingGame::renderZones() {
    SDL_Renderer* renderer = state.renderer;

    for (size_t i = 0; i < zoneRects.size(); ++i) {
        SDL_FRect zr = zoneRects[i];  // copy

        // Fill color
        if (zoneHit[i]) {
            SDL_SetRenderDrawColor(renderer, 255, 105, 180, 180); // hit: pink
        } else {
            SDL_SetRenderDrawColor(renderer, 100, 255, 220, 130); // idle: teal
        }
        SDL_RenderFillRect(renderer, &zr);

        // Border
        SDL_SetRenderDrawColor(renderer, 220, 220, 240, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(renderer, &zr);
    }
}

void EggCrackingGame::renderMarker() {
    SDL_Renderer* renderer = state.renderer;

    SDL_FRect rect = markerRect;

    SDL_SetRenderDrawColor(renderer, 255, 105, 180, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);
}

void EggCrackingGame::renderHitFeedback() {
    SDL_Renderer* renderer = state.renderer;
    uint32_t now = SDL_GetTicks();

    for (const auto& fb : hitFeedbacks) {
        uint32_t age = now - fb.spawnTime;
        if (age >= fb.lifetimeMs) continue;

        float t = age / static_cast<float>(fb.lifetimeMs); // 0 to 1
        float fade = 1.0f - t;

        uint8_t alpha = static_cast<uint8_t>(fb.color.a * fade);
        SDL_SetRenderDrawColor(renderer, fb.color.r, fb.color.g, fb.color.b, alpha);
        SDL_RenderFillRect(renderer, &fb.rect);

        // subtle outline that also fades
        SDL_SetRenderDrawColor(renderer, 240, 240, 250, alpha);
        SDL_RenderRect(renderer, &fb.rect);
    }
}

void EggCrackingGame::renderUI() {
    SDL_Color white{255, 255, 255, SDL_ALPHA_OPAQUE};
    SDL_Color ghost{170, 210, 255, SDL_ALPHA_OPAQUE};

    std::string eggText;

    if (mode == Mode::Normal) {
        eggText = "EGG " + to_string(currentEgg + 1) + " / " + to_string(totalEggs);
    } else {
        // Endless: show how many eggs you’ve survived so far
        eggText = "EGGS CRACKED: " + to_string(currentEgg);
    }

    renderTextCentered(eggText, state.logH * 0.10f, white);

    std::string info;
    if (mode == Mode::Normal) {
        info = "Press SPACE to crack!";
    } else {
        info = "Press SPACE to crack! Don't miss!";
    }

    renderTextCentered(info, state.logH * 0.16f, ghost);
}

void EggCrackingGame::renderCountdown() {
    uint64_t now = SDL_GetTicks();
    uint64_t elapsed = now - countdownStartTick;

    uint64_t remaining = (elapsed >= countdownDurationMs)
        ? 0
        : (countdownDurationMs - elapsed);

    int secondsLeft = static_cast<int>(std::ceil(remaining / 1000.0f));

    string text;
    if (secondsLeft > 0) {
        text = "Ready? " + to_string(secondsLeft);
    } else {
        text = "Go!";
    }

    SDL_Color yellow{ 255, 230, 150, SDL_ALPHA_OPAQUE };
    renderTextCentered(text, state.logH * 0.30f, yellow);
}

void EggCrackingGame::renderEggResultOverlay() {
    SDL_Renderer* renderer = state.renderer;
    uint64_t now = SDL_GetTicks();

    float t = std::min(1.0f,
        (now - resultStartTick) / static_cast<float>(resultDurationMs));

    // 1. Dim the whole screen
    SDL_FRect fullScreen{
        0.0f, 0.0f,
        static_cast<float>(state.logW),
        static_cast<float>(state.logH)
    };

    uint8_t alpha = static_cast<uint8_t>(std::min(200.0f, t * 240.0f));
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
    SDL_RenderFillRect(renderer, &fullScreen);

    // 2. Choose result tier based on hitsThisEgg
    int tier = 0;
    if (hitsThisEgg >= 3) tier = 3;
    else if (hitsThisEgg == 2) tier = 2;
    else if (hitsThisEgg == 1) tier = 1;
    else tier = 0;

    string text;
    if (tier == 3) text = "PERFECT CRACK!";
    else if (tier == 2) text = "Almost EGGcelent!";
    else if (tier == 1) text = "You're messy...";
    else text = "You got shell everywhere!";

    // 3. Zoom animation factor (0.6 -> 1.1 -> 1.0)
    float scale;
    if (t < 0.5f) {
        float u = t / 0.5f;
        scale = 0.6f + u * (1.1f - 0.6f);
    } else {
        float u = (t - 0.5f) / 0.5f;
        scale = 1.1f + (1.0f - 1.1f) * u;
    }

    // Base sizes relative to screen
    float baseEggW = state.logW * 0.35f;
    float baseEggH = state.logH * 0.35f;
    float eggW = baseEggW * scale;
    float eggH = baseEggH * scale;

    float centerX = state.logW * 0.5f;
    float centerY = state.logH * 0.55f;  // a bit lower than center

    // 4. Draw egg result sprite in the center
    SDL_FRect eggRect{
        centerX - eggW / 2.0f,
        centerY - eggH / 2.0f,
        eggW,
        eggH
    };

    SDL_Texture* resTex = texResult[tier];

    if (resTex) {
        SDL_RenderTexture(renderer, resTex, nullptr, &eggRect);
    } else {
        // fallback colours if no texture
        if (tier == 3) SDL_SetRenderDrawColor(renderer, 255, 230, 80, SDL_ALPHA_OPAQUE);
        else if (tier == 2) SDL_SetRenderDrawColor(renderer, 180, 255, 140, SDL_ALPHA_OPAQUE);
        else if (tier == 1) SDL_SetRenderDrawColor(renderer, 255, 180, 120, SDL_ALPHA_OPAQUE);
        else SDL_SetRenderDrawColor(renderer, 255, 80, 80, SDL_ALPHA_OPAQUE);

        SDL_RenderFillRect(renderer, &eggRect);
    }

    // 5. Draw text above the image, centered
    if (state.font) {
        SDL_Color txtColor{ 255, 255, 255, SDL_ALPHA_OPAQUE };
        SDL_Surface* surf = TTF_RenderText_Solid(state.font, text.c_str(), 0, txtColor);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                float tw, th;
                SDL_GetTextureSize(tex, &tw, &th);

                SDL_FRect dst{
                    centerX - tw / 2.0f,
                    eggRect.y - th - 20.0f,  // above egg
                    tw,
                    th
                };

                SDL_RenderTexture(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_DestroySurface(surf);
        }
    }
}

void EggCrackingGame::renderDoneText() {
    SDL_Color gold{255, 215, 0, SDL_ALPHA_OPAQUE};

    string text;
    if (mode == Mode::Normal) {
        text = "All eggs cracked! Score: " + to_string(finalScore) + "/100";
    } else {
        text = "Game over! Eggs cracked: " + to_string(finalScore);
    }

    renderTextCentered(text, state.logH * 0.32f, gold);
}

void EggCrackingGame::renderTextCentered(const string& text, float y, SDL_Color color) {
    if (!state.font) return;

    SDL_Renderer* renderer = state.renderer;

    SDL_Surface* surf = TTF_RenderText_Solid(state.font, text.c_str(), 0, color);
    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (!tex) {
        SDL_DestroySurface(surf);
        return;
    }

    float tw, th;
    SDL_GetTextureSize(tex, &tw, &th);

    SDL_FRect dst{
        (state.logW - tw) / 2.0f,
        y - th / 2.0f,
        tw,
        th
    };

    SDL_RenderTexture(renderer, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_DestroySurface(surf);
}
