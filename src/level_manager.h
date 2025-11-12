#pragma once

#include <vector>
#include <memory>
#include "data_structs.h"
#include "minigames/minigame.h"
#include "minigames/cutting_game.h"

class LevelManager {
public:
    LevelManager(SDLState& state);

    void loadRecipes();
    Recipe* getCurrentRecipe();
    bool advanceStep();
    bool isRecipeComplete();

    void render();
    void update();
    void handleEvent(const SDL_Event& event);

private:
    unique_ptr<Minigame> currentMinigame;
    SDLState& state;
    std::vector<Recipe> recipes;
    Recipe* currentRecipe = nullptr;
    int currentRecipeIndex = 0;
};

