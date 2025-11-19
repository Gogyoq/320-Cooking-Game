#include <vector>
#include <memory>
#include "level_manager.h"
#include "minigames/minigame.h"
#include "minigames/cutting_game.h"
#include "minigames/mixing_game.h"
#include "minigames/frying_game.h"
#include "data_structs.h"
#include <iostream>

using namespace std;

//Level Manager Implementation
LevelManager::LevelManager(SDLState& state) 
: state(state), currentMinigame(nullptr), recipeStarted(false),
  selectButton(300, 320, 200, 60, "Select", [this]() { onSelectClick(); })
{
    //Load all available recipes
    loadRecipes();
}

void LevelManager::render() {
    if (recipeStarted && currentMinigame != nullptr) {
        currentMinigame->render();
    }
    else { //Render the level select screen
        // Set background color
        SDL_SetRenderDrawColor(state.renderer, 40, 40, 60, 255);
        SDL_RenderClear(state.renderer);

        //Find center of screen
        float centerX = (float)state.logW / 2;
        float centerY = (float)state.logH / 2;

        // Render each recipe card
        for (size_t i = 0; i < recipes.size(); i++) {
            // Calculate position using smooth scroll position
            float cardOffset = i * (CARD_WIDTH + CARD_SPACING) - currentScrollPosition;
            float xPos = centerX + cardOffset;

            // Only render cards that are visible
            if (xPos < -CARD_WIDTH || xPos > state.logW + CARD_WIDTH) {
                continue;
            }

            // Scale based on distance from center
            float distanceFromCenter = abs(cardOffset) / (float)CARD_WIDTH;
            float scale = max(0.7f, 1.0f - distanceFromCenter * 0.3f);

            float scaledWidth = CARD_WIDTH * scale;
            float scaledHeight = 400 * scale;

            SDL_FRect cardRect = {
                (xPos - scaledWidth / 2),
                (centerY - scaledHeight / 2),
                scaledWidth,
                scaledHeight
            };

            // Highlight selected card
            if (i == selectedRecipeIndex) {
                SDL_SetRenderDrawColor(state.renderer, 100, 200, 255, 255);
            }
            else {
                SDL_SetRenderDrawColor(state.renderer, 80, 80, 100, 255);
            }
            SDL_RenderFillRect(state.renderer, &cardRect);

            //Render text
            string text = recipes[selectedRecipeIndex].name;
            SDL_Color textColor = { 0, 0, 0, SDL_ALPHA_OPAQUE };
            SDL_Surface* textSurface = TTF_RenderText_Solid(state.font, text.c_str(), 0, textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(state.renderer, textSurface);

                if (textTexture) {
                    float textW, textH;
                    SDL_GetTextureSize(textTexture, &textW, &textH);

                    SDL_FRect textRect = {
                        (xPos - textW/2),
                        (centerY - scaledHeight / 2) + 10.0f,
                        textW,
                        textH
                    };

                    SDL_RenderTexture(state.renderer, textTexture, nullptr, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_DestroySurface(textSurface);
            }
        }//End of card render loop

        //Render select button if not switching cards
        if (!isCarouselAnimating()) {
            selectButton.render(state);
        }
    }
}

void LevelManager::update() {
    if (recipeStarted && currentMinigame != nullptr) {
        currentMinigame->update();
    }
    else {  //update the level select screen
        // Interpolate current position toward target
        float distance = targetScrollPosition - currentScrollPosition;
        currentScrollPosition += distance * SCROLL_SPEED;

        // Snap to target when very close (prevents infinite tiny movements)
        if (abs(distance) < 0.5f) {
            currentScrollPosition = targetScrollPosition;
        }
    }
}

void LevelManager::handleEvent(const SDL_Event& event) {
    if (recipeStarted && currentMinigame != nullptr) {
        currentMinigame->handleEvent(event);
    }
    else {  //Event handle the level select screen
        if (event.type == SDL_EVENT_KEY_DOWN) {
            switch (event.key.key) {
            case SDLK_RIGHT:
                if (selectedRecipeIndex < recipes.size() - 1) {
                    selectedRecipeIndex++;
                    targetScrollPosition = selectedRecipeIndex * (CARD_WIDTH + CARD_SPACING);
                }
                break;

            case SDLK_LEFT:
                if (selectedRecipeIndex > 0) {
                    selectedRecipeIndex--;
                    targetScrollPosition = selectedRecipeIndex * (CARD_WIDTH + CARD_SPACING);
                }
                break;
            case SDLK_ESCAPE:
                state.gameState = GameState::MAIN_MENU;
                break;
            }

        }
        // Handle mouse wheel scrolling
        else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            // event.wheel.y is positive when scrolling away from user (up/forward)
            // and negative when scrolling toward user (down/backward)
            if (event.wheel.y > 0) { // Scroll up/forward = move left
                if (selectedRecipeIndex > 0) {
                    selectedRecipeIndex--;
                    targetScrollPosition = selectedRecipeIndex * (CARD_WIDTH + CARD_SPACING);
                }
            }
            else if (event.wheel.y < 0) { // Scroll down/backward = move right
                if (selectedRecipeIndex < recipes.size() - 1) {
                    selectedRecipeIndex++;
                    targetScrollPosition = selectedRecipeIndex * (CARD_WIDTH + CARD_SPACING);
                }
            }
        }

        //Event handle select button
        if (!isCarouselAnimating()) {
            selectButton.handleEvent(event);
        }
    }
}

void LevelManager::startRecipe()
{   
    string action = currentRecipe->steps[0].action;
    if (action == "cut") {
        currentMinigame = make_unique<CuttingGame>(state, (*currentRecipe).steps[0]);
    }
    else if (action == "mix") { //Placeholders
        currentMinigame = make_unique<MixingGame>(state, (*currentRecipe).steps[0]);
    }
    else if (action == "fry") {
        currentMinigame = make_unique<FryingGame>(state, (*currentRecipe).steps[0]);
    }
    recipeStarted = true;
}

void LevelManager::advanceStep()
{

}

void LevelManager::loadRecipes() {
    //Test Frying Recipe
    Recipe fryingTest;
    fryingTest.name = "Frying Test";
    fryingTest.difficulty = 1;
    fryingTest.description = "Test recipe for Frying minigame";

    Ingredient carrot = {
        .name = "carrot",
        .quantity = 1,
        .unit = "whole"
    };

    CookingStep fry = {
        .action = "fry",
        .ingredients = {carrot},
        .duration = 15.0f,
        .perfectWindow = 1.5f
    };

    fryingTest.steps.push_back(fry);

    recipes.push_back(fryingTest);

    //Test Frying Recipe
    Recipe mixingTest;
    mixingTest.name = "Mixing Test";
    mixingTest.difficulty = 1;
    mixingTest.description = "Test recipe for Mixing minigame";

    CookingStep mix = {
        .action = "mix",
        .ingredients = {carrot},
        .duration = 5.0f,
        .perfectWindow = 1.5f
    };

    mixingTest.steps.push_back(mix);

    recipes.push_back(mixingTest);

    //Replace later with JSON recipe loading 
    //Test recipe for cutting minigame
    Recipe cuttingTest;
    cuttingTest.name = "Cutting Test";
    cuttingTest.difficulty = 1;
    cuttingTest.description = "Test recipe for Cutting minigame";

    CookingStep cut = {
        .action = "cut",
        .ingredients = {carrot},
        .duration = 5.0f,
        .perfectWindow = 1.5f
    };

    cuttingTest.steps.push_back(cut);

    recipes.push_back(cuttingTest);
}

void LevelManager::onSelectClick()
{
    cout << "Select Button Clicked! Recipe Index: " << selectedRecipeIndex << endl;
    currentRecipe = &recipes[selectedRecipeIndex];
    startRecipe();
}

bool LevelManager::isCarouselAnimating() const
{
    const float THRESHOLD = 0.5f;
    return abs(targetScrollPosition - currentScrollPosition) > THRESHOLD;
}

Recipe* LevelManager::getCurrentRecipe() {
    return nullptr;
}

bool LevelManager::isRecipeComplete() {
    return false;
}
