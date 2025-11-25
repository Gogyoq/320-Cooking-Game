#pragma once

#include <vector>
#include <memory>
#include "data_structs.h"
#include "button.h"
#include "image_button.h"
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
    void onSelectClick();
    void lClick();
    void rClick();
    void configureLayout();
    bool isCarouselAnimating() const;

    unique_ptr<Minigame> currentMinigame;
    SDLState& state;
    std::vector<Recipe> recipes;
    Recipe* currentRecipe = nullptr;
    bool recipeStarted, recipeFinished, playStartAnimation, playFinishAnimation;
    int animationTickCounter = 0;
    const int ANIMATION_DURATION_TICKS = 100; // 2 seconds at 50 updates/second

    //Variables for level select screen
    ImageButton selectButton;
	ImageButton leftButton;
	ImageButton rightButton;

    SDL_Texture* leftTexture = nullptr;
    SDL_Texture* rightTexture = nullptr;
    SDL_Texture* selectTexture = nullptr;

    SDL_Texture* ill_cooking = nullptr;
    SDL_Texture* ill_cracking = nullptr;
    SDL_Texture* ill_mixing = nullptr;
    SDL_Texture* ill_cutting = nullptr;

    // Layout / assets
    void loadTextures();
    void cleanupTextures();

    int selectedRecipeIndex = 0;
    float currentScrollPosition = 0.0f;  // Current visual position
    float targetScrollPosition = 0.0f;   // Where we're scrolling to
    const float SCROLL_SPEED = 0.15f;    // Interpolation speed (0.0-1.0)
    const float CARD_WIDTH = 300.0f;
    const float CARD_SPACING = 50.0f;
};

