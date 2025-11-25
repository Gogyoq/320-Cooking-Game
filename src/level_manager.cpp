#include <vector>
#include <memory>
#include <cstdint>
#include "level_manager.h"
#include "minigames/minigame.h"
#include "minigames/cutting_game.h"
#include "minigames/mixing_game.h"
#include "minigames/frying_game.h"
#include "minigames/egg_cracking_game.h"
#include "data_structs.h"
#include <iostream>

using namespace std;

//Level Manager Implementation
LevelManager::LevelManager(SDLState& state)
    : state(state), currentMinigame(nullptr), recipeStarted(false),
    recipeFinished(false), playStartAnimation(false), playFinishAnimation(false),
  selectButton(300, 320, 200, 100, "Select","", [this]() { onSelectClick(); }),
  rightButton(10, 100, 60, 60, "Settings", "", [this]() { lClick(); }),
  leftButton(10, 10, 60, 60, "Settings", "", [this]() { rClick(); })
{
    //Load all available recipes
    loadTextures();
    configureLayout();
    loadRecipes();
}
void LevelManager::loadTextures() {
    // Load button textures
    rightTexture = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/hand_r.PNG");
    leftTexture = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/hand_l.PNG");
    selectTexture = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/select.PNG");

    // Set loaded textures to image buttons
    leftButton.setTexture(leftTexture);
    rightButton.setTexture(rightTexture);
    selectButton.setTexture(selectTexture);
}

void LevelManager::cleanupTextures() {
    if (rightTexture) SDL_DestroyTexture(rightTexture);
    if (leftTexture) SDL_DestroyTexture(leftTexture);
    if (selectTexture) SDL_DestroyTexture(selectTexture);

    leftTexture = nullptr;
    rightTexture = nullptr;
    selectTexture = nullptr;
}


void LevelManager::configureLayout() {
    // No further layout changes are needed since buttons are already configured in their constructors
}

void LevelManager::render() {
    if (recipeStarted && currentMinigame != nullptr) {
        currentMinigame->render();

        if (showingResults) {
            renderResults();
        }

        if ((playStartAnimation || playFinishAnimation) && !showingResults) {
            string text = playStartAnimation ? "Start!" : "Finished!";
            SDL_Color textColor = { 255, 255, 255, SDL_ALPHA_OPAQUE };

            SDL_Surface* textSurface = TTF_RenderText_Solid(state.font, text.c_str(), 0, textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(state.renderer, textSurface);
                if (textTexture) {
                    float textW, textH;
                    SDL_GetTextureSize(textTexture, &textW, &textH);
                    textW = textW * 4;
                    textH = textH * 4;
                    // Center the text on screen
                    SDL_FRect textRect = {
                        (state.logW - textW) / 2,
                        (state.logH - textH) / 2,
                        textW,
                        textH
                    };

                    SDL_RenderTexture(state.renderer, textTexture, nullptr, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_DestroySurface(textSurface);
            }
        }
    }
    else { //Render the level select screen
        // Set background color
        SDL_SetRenderDrawColor(state.renderer, 21, 34, 31, 1);
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
                SDL_SetRenderDrawColor(state.renderer, 66, 105, 90, 1);
            }
            else {
                SDL_SetRenderDrawColor(state.renderer, 23, 46, 42, 1);
            }
            SDL_RenderFillRect(state.renderer, &cardRect);

            //Render text
            string text = recipes[i].name;
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
        if (showingResults) {
            uint64_t elapsed = SDL_GetTicks() - resultsStartTick;
            if (elapsed >= RESULTS_DURATION_MS) {
                showingResults = false;
                playFinishAnimation = true;
                animationTickCounter = 0;
            }
        }
        else if (playStartAnimation || playFinishAnimation) {
            animationTickCounter++;

            // Check if 2 seconds have elapsed
            if (animationTickCounter >= ANIMATION_DURATION_TICKS) {
                if (playStartAnimation) {
                    playStartAnimation = false;
                }
                else {
                    playFinishAnimation = false;
                    advanceStep();
                }
                animationTickCounter = 0; // Reset counter
            }
        }
        else if (currentMinigame->isComplete() && !recipeFinished) {
            startResultsDisplay({ currentMinigame->getScore() });
        }
        else {
            currentMinigame->update();
        }
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
        if (!playStartAnimation && !playFinishAnimation && !showingResults) {
            currentMinigame->handleEvent(event);
        }
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

void LevelManager::startResultsDisplay(const std::vector<int>& scores)
{
    resultScores = scores;
    showingResults = true;
    resultsStartTick = SDL_GetTicks();
    animationTickCounter = 0;
}

void LevelManager::renderResults()
{
    SDL_BlendMode previousBlendMode = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(state.renderer, &previousBlendMode);
    SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 200);
    SDL_FRect overlay = { 0.0f, 0.0f, static_cast<float>(state.logW), static_cast<float>(state.logH) };
    SDL_RenderFillRect(state.renderer, &overlay);

    int startY = 50;
    int barHeight = 40;
    int spacing = 10;

    for (size_t i = 0; i < resultScores.size(); ++i) {
        SDL_FRect rect;
        rect.x = 50.0f;
        rect.y = static_cast<float>(startY + i * (barHeight + spacing));
        rect.w = static_cast<float>(resultScores[i] * 5);
        rect.h = static_cast<float>(barHeight);

        SDL_SetRenderDrawColor(state.renderer, 0, 200, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(state.renderer, &rect);

        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(state.renderer, &rect);

        if (state.font) {
            string scoreText = to_string(resultScores[i]);
            SDL_Color textColor = { 255, 255, 255, SDL_ALPHA_OPAQUE };
            SDL_Surface* textSurface = TTF_RenderText_Solid(state.font, scoreText.c_str(), 0, textColor);

            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(state.renderer, textSurface);
                if (textTexture) {
                    float textW, textH;
                    SDL_GetTextureSize(textTexture, &textW, &textH);

                    SDL_FRect textRect = {
                        rect.x + (rect.w - textW) / 2,
                        rect.y + (rect.h - textH) / 2,
                        textW,
                        textH
                    };

                    SDL_RenderTexture(state.renderer, textTexture, nullptr, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_DestroySurface(textSurface);
            }
        }
    }

    SDL_SetRenderDrawBlendMode(state.renderer, previousBlendMode);
}

void LevelManager::resetToLevelSelect()
{
    recipeStarted = false;
    playStartAnimation = false;
    playFinishAnimation = false;
    showingResults = false;
    animationTickCounter = 0;
    resultsStartTick = 0;
    resultScores.clear();
    currentMinigame.reset();

    if (currentRecipe) {
        currentRecipe->currentStep = 0;
    }
}

void LevelManager::advanceStep()
{
    if (currentRecipe->currentStep <= currentRecipe->steps.size() - 1) {
        string action = currentRecipe->steps[currentRecipe->currentStep].action;
        if (action == "cut") {
            currentMinigame = make_unique<CuttingGame>(state, (*currentRecipe).steps[0]);
        }
        else if (action == "mix") { 
            currentMinigame = make_unique<MixingGame>(state, (*currentRecipe).steps[0]);
        }
        else if (action == "fry") {
            currentMinigame = make_unique<FryingGame>(state, (*currentRecipe).steps[0]);
        }
        else if (action == "egg") {
            currentMinigame = make_unique<EggCrackingGame>(state, (*currentRecipe).steps[0], EggCrackingGame::Mode::Normal);
        }
        else if (action == "egg_endless") {
            currentMinigame = make_unique<EggCrackingGame>(state, (*currentRecipe).steps[0], EggCrackingGame::Mode::Endless);
        }

        currentRecipe->currentStep++;
        recipeStarted = true;
        playStartAnimation = true;
    }
    else { //No more cooking steps means recipe is complete
        recipeFinished = true;
        resetToLevelSelect();
    }
}

void LevelManager::loadRecipes() {
    //Test Frying Recipe
    Recipe fryingTest;
    fryingTest.name = "Frying";
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
        .duration = 5.0f,
        .perfectWindow = 1.5f
    };

    fryingTest.steps.push_back(fry);

    recipes.push_back(fryingTest);

    //Test Frying Recipe
    Recipe mixingTest;
    mixingTest.name = "Mixing";
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
    cuttingTest.name = "Cutting";
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

    Recipe eggTest;
    eggTest.name = "Egg Cracking";
    eggTest.difficulty = 1;
    eggTest.description = "Test recipe for Egg Cracking minigame";

    Ingredient eggIngr {
        .name = "egg",
        .quantity = 3,
        .unit = "pcs"
    };

    CookingStep crackStep {
        .action = "egg",
        .ingredients = {eggIngr},
        .duration = 3.0f,  // number of eggs to crack 
        .perfectWindow = 0.2f
    };

    eggTest.steps.push_back(crackStep);
    recipes.push_back(eggTest);

        // Endless Egg Cracking test recipe
    Recipe eggEndless;
    eggEndless.name = "Endless Egg!";
    eggEndless.difficulty = 2;
    eggEndless.description = "Endless egg timing — gets faster until you miss.";

    CookingStep endlessStep {
        .action = "egg_endless", 
        .ingredients = { eggIngr }, 
        .duration = 3.0f,           // not important in Endless
        .perfectWindow = 0.0f
    };

    eggEndless.steps.push_back(endlessStep);
    recipes.push_back(eggEndless);

    Recipe multipleTest;
    multipleTest.name = "Multiple Minigames";
    multipleTest.difficulty = 1;
    multipleTest.description = "Test recipe for chaining minigames";

    multipleTest.steps.push_back(cut);
    multipleTest.steps.push_back(fry);
    multipleTest.steps.push_back(mix);
    multipleTest.steps.push_back(crackStep);
    recipes.push_back(multipleTest);
}

void LevelManager::onSelectClick()
{
    cout << "Select Button Clicked! Recipe Index: " << selectedRecipeIndex << endl;
    recipeFinished = false;
    resetToLevelSelect();
    currentRecipe = &recipes[selectedRecipeIndex];
    currentRecipe->currentStep = 0;
    advanceStep();
}

void LevelManager::lClick()
{
    cout << "Left Button Clicked!" << endl;
    if (selectedRecipeIndex > 0) {
        selectedRecipeIndex--;
        targetScrollPosition = selectedRecipeIndex * (CARD_WIDTH + CARD_SPACING);
    }

}
void LevelManager::rClick()
{
    cout << "Right Button Clicked!" << endl;
    if (selectedRecipeIndex < recipes.size() - 1) {
        selectedRecipeIndex++;
        targetScrollPosition = selectedRecipeIndex * (CARD_WIDTH + CARD_SPACING); 
    }
}


bool LevelManager::isCarouselAnimating() const
{
    const float THRESHOLD = 0.5f;
    return abs(targetScrollPosition - currentScrollPosition) > THRESHOLD;
}

Recipe* LevelManager::getCurrentRecipe() {
    return currentRecipe;
}

bool LevelManager::isRecipeComplete() {
    return recipeFinished;
}
