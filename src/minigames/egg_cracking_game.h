#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <random>
#include <string>
#include "../data_structs.h"
#include "minigame.h"

class EggCrackingGame : public Minigame {
public:
    // Game mode
    enum class Mode {
        Normal, // fixed number of eggs, 0–3 score
        Endless // keeps going faster until you miss an egg
    };

    EggCrackingGame(SDLState& state, const CookingStep& step, Mode mode = Mode::Normal);
    ~EggCrackingGame();

    void render() override;
    void update() override;
    void handleEvent(const SDL_Event& event) override;
    bool isComplete() const override;

    // optional hook for LevelManager later
    int getScore() const {return finalScore;}

private:
    enum class State {
        Countdown,
        Active,
        EggResult,
        Done
    };

    enum class CrackAnimState {
        Idle,
        CrackDown,
        Impact,
        CrackUp
    };

    // One fading hit marker on the bar
    struct HitFeedback {
        SDL_FRect rect;
        SDL_Color color;
        uint64_t spawnTime;
        uint32_t lifetimeMs;
    };

    SDLState& state;
    const CookingStep step;
    Mode mode;   // Normal vs Endless
    State stateMachine = State::Countdown;
    CrackAnimState crackAnimState  = CrackAnimState::Idle;

    // Eggs / scoring
    int totalEggs = 3;   // Normal: #eggs; Endless: just a big cap
    int currentEgg = 0;
    int pressesPerEgg = 3;   // max presses per egg
    int pressesThisEgg = 0;
    int hitsThisEgg = 0;
    int totalHits = 0;
    int finalScore = 0;   // Normal: 0–3, Endless: eggs survived

    int endlessPerfectEggs = 0;

    bool eggFailed = false;

    // Timing
    uint32_t passDurationMs = 3200;  // ms for one marker pass
    const uint32_t minPassDurationMs = 800;   // clamp for Endless speed-up
    const float    speedMultiplier = 0.88f; // each success: passDuration *= speedMultiplier

    uint32_t countdownDurationMs = 1000; // 1 second
    uint32_t resultDurationMs = 900;  // per-egg result overlay display

    uint64_t countdownStartTick = 0;
    uint64_t passStartTick = 0;
    uint64_t resultStartTick = 0;
    uint64_t crackAnimStartMs = 0;

    // Layout rects
    SDL_FRect barRect{};
    SDL_FRect markerRect{};
    SDL_FRect bowlRect{};
    SDL_FRect handIdlePos{};
    SDL_FRect handCrackPos{};
    SDL_FRect handRect{};
    SDL_FRect yolkRect{}; // not currently animated

    // Textures
    SDL_Texture* texBackground = nullptr;
    SDL_Texture* texBowl = nullptr;
    SDL_Texture* texHandIdle = nullptr;
    SDL_Texture* texHandCrack = nullptr;
    SDL_Texture* texYolk = nullptr;
    SDL_Texture* texResult[4] = { nullptr, nullptr, nullptr, nullptr };

    // Zones on the bar
    std::vector<SDL_FRect> zoneRects;
    std::vector<bool>      zoneHit;

    // Yolk flags (currently unused)
    bool yolkActive      = false;
    bool yolkSuccessful  = false;

    // RNG for zone jitter
    std::mt19937 rng;

    // Fading hit markers
    std::vector<HitFeedback> hitFeedbacks;

    // -------- Internal helpers --------

    // Layout / assets
    void configureLayout();
    void loadTextures();
    void cleanupTextures();

    // Zones & eggs
    void generateZonesForCurrentEgg();
    void startNewEgg();
    void finishCurrentEgg();
    void finishMinigame();

    // Logic
    void updateCrackAnimation(uint64_t now);
    void updateHitFeedbacks(uint64_t now);
    void handleSpacePress();
    bool markerInZone(int zoneIndex) const;

    // Render
    void renderBackground();
    void renderKitchen();
    void renderBar();
    void renderZones();
    void renderMarker();
    void renderHitFeedback();
    void renderUI();
    void renderCountdown();
    void renderEggResultOverlay();
    void renderDoneText();
    void renderTextCentered(const std::string& text, float y, SDL_Color color);
};
