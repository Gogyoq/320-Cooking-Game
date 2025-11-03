#include <vector>
#include <memory>
#include "level_manager.h"
#include "data_structs.h"

//Level Manager Implementation
LevelManager::LevelManager(SDLState& state) 
: state(state)
{
    loadRecipes();
}

void LevelManager::render() {

}

void LevelManager::update() {

}

void LevelManager::handleEvent(const SDL_Event& event) {

}

void LevelManager::loadRecipes() {
    Recipe test;
    test.name = "cut_test";
    test.difficulty = 1;
    test.description = "Test recipe";

    Ingredient carrot = {
        .name = "Carrot",
        .quantity = 1,
        .unit = "Whole"
    };

    test.ingredients.push_back(carrot);

    CookingStep cut = {
        .action = "cut",
        .ingredients = {"Carrot"},
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
