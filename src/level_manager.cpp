#include <vector>
#include <memory>
#include "level_manager.h"
#include "minigames.h"
#include "data_structs.h"

using namespace std;

//Level Manager Implementation
LevelManager::LevelManager(SDLState& state) 
: state(state)
{
    loadRecipes();

    Ingredient carrot = {
        .name = "carrot",
        .quantity = 1,
        .unit = "whole"
    };

    currentMinigame = make_unique<CuttingGame>(state, carrot);
}

void LevelManager::render() {
    currentMinigame->render();
}

void LevelManager::update() {
    currentMinigame->update();
}

void LevelManager::handleEvent(const SDL_Event& event) {
    currentMinigame->handleEvent(event);
}

void LevelManager::loadRecipes() {
    Recipe test;
    test.name = "cut_test";
    test.difficulty = 1;
    test.description = "Test recipe";

    Ingredient carrot = {
        .name = "carrot",
        .quantity = 1,
        .unit = "whole"
    };

    CookingStep cut = {
        .action = "cut",
        .ingredients = {carrot},
        .duration = 5.0f,
        .perfectWindow = 1.5f
    };

    test.steps.push_back(cut);

    recipes.push_back(test);
}

Recipe* LevelManager::getCurrentRecipe() {
    return nullptr;
}

bool LevelManager::advanceStep() {
    return false;
}

bool LevelManager::isRecipeComplete() {
    return false;
}
