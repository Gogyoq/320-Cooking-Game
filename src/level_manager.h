#pragma once

#include <vector>
#include <memory>
#include "data_structs.h"
#include "button.h"
#include "minigames/minigame.h"
#include "minigames/cutting_game.h"
#include "minigames/egg_cracking_game.h"

class LevelManager {
public:
    LevelManager(SDLState& state);

    void loadRecipes();
    Recipe* getCurrentRecipe();
    void advanceStep();
    bool isRecipeComplete();

    void render();
    void update();
    void handleEvent(const SDL_Event& event);

private:
    void renderResults();
    void startResultsDisplay(const std::vector<int>& scores);
    void onSelectClick();
    bool isCarouselAnimating() const;
    void resetToLevelSelect();

    unique_ptr<Minigame> currentMinigame;
    SDLState& state;
    std::vector<Recipe> recipes;
    Recipe* currentRecipe = nullptr;
    bool recipeStarted, recipeFinished, playStartAnimation, playFinishAnimation;
    int animationTickCounter = 0;
    const int ANIMATION_DURATION_TICKS = 100; // 2 seconds at 50 updates/second

    //Variables for level select screen
    Button selectButton;
    int selectedRecipeIndex = 0;
    float currentScrollPosition = 0.0f;  // Current visual position
    float targetScrollPosition = 0.0f;   // Where we're scrolling to
    const float SCROLL_SPEED = 0.15f;    // Interpolation speed (0.0-1.0)
    const float CARD_WIDTH = 300.0f;
    const float CARD_SPACING = 50.0f;

    // Results handling
    bool showingResults = false;
    uint64_t resultsStartTick = 0;
    const uint64_t RESULTS_DURATION_MS = 3000;
    std::vector<int> resultScores;
};
