#include <vector>
#include <memory>
#include <sstream>
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
  leftButton(10, 10, 60, 60, "Settings", "", [this]() { rClick(); }),
    cardIll(10, 10, 60, 60, "Settings", "", [this]() { rClick(); })
  
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
    
    // Load card illustration textures
    ill_cooking = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/cooking_i.PNG");
    ill_cracking = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/cracking_i.PNG");
    ill_mixing = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/mixing_i.PNG");
    ill_cutting = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/cutting_i.PNG");
    ill_infinity_cracking = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/endless_i.PNG");
    ill_multiple = IMG_LoadTexture(state.renderer, "src/res/sprites/menu_graph/multiple_i.PNG");
}

void LevelManager::cleanupTextures() {
    if (rightTexture) SDL_DestroyTexture(rightTexture);
    if (leftTexture) SDL_DestroyTexture(leftTexture);
    if (selectTexture) SDL_DestroyTexture(selectTexture);

    leftTexture = nullptr;
    rightTexture = nullptr;
    selectTexture = nullptr;
    
    if (ill_cooking) SDL_DestroyTexture(ill_cooking);
    if (ill_cracking) SDL_DestroyTexture(ill_cracking);
    if (ill_mixing) SDL_DestroyTexture(ill_mixing);
    if (ill_cutting) SDL_DestroyTexture(ill_cutting);
    if (ill_infinity_cracking) SDL_DestroyTexture(ill_infinity_cracking);
    if (ill_multiple) SDL_DestroyTexture(ill_multiple);
    
    ill_cooking = nullptr;
    ill_cracking = nullptr;
    ill_mixing = nullptr;
    ill_cutting = nullptr;
    ill_infinity_cracking = nullptr;
    ill_multiple = nullptr;
}


void LevelManager::configureLayout() {
    // No further layout changes are needed since buttons are already configured in their constructors
}

void LevelManager::render() {
    if (recipeStarted && currentMinigame != nullptr) {
        currentMinigame->render();

        if (playStartAnimation || playFinishAnimation) {
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

            //Render text with wrapping
            string text = recipes[i].name;
            SDL_Color textColor = { 0, 0, 0, SDL_ALPHA_OPAQUE };
            
            // Use smaller font for non-selected cards
            TTF_Font* fontToUse = (i == selectedRecipeIndex) ? state.font : state.fontSmall;
            
            // Calculate max text width (90% of card width to leave padding)
            float maxTextWidth = scaledWidth * 0.9f;
            
            // Split text into multiple lines if it exceeds max width
            vector<string> textLines;
            string currentLine;
            stringstream ss(text);
            string word;
            
            while (ss >> word) {
                string testLine = currentLine.empty() ? word : currentLine + " " + word;
                SDL_Surface* testSurface = TTF_RenderText_Blended(fontToUse, testLine.c_str(), 0, textColor);
                if (testSurface) {
                    float testWidth = (float)testSurface->w;
                    SDL_DestroySurface(testSurface);
                    
                    if (testWidth > maxTextWidth && !currentLine.empty()) {
                        // Line is too long, save current line and start new one
                        textLines.push_back(currentLine);
                        currentLine = word;
                    } else {
                        // Add word to current line
                        currentLine = testLine;
                    }
                }
            }
            if (!currentLine.empty()) {
                textLines.push_back(currentLine);
            }
            
            // Render each line
            float lineHeight = 0;
            float currentY = (centerY - scaledHeight / 2) + 30.0f;
            
            for (size_t lineIdx = 0; lineIdx < textLines.size(); lineIdx++) {
                SDL_Surface* textSurface = TTF_RenderText_Blended(fontToUse, textLines[lineIdx].c_str(), 0, textColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(state.renderer, textSurface);

                    if (textTexture) {
                        float textW, textH;
                        SDL_GetTextureSize(textTexture, &textW, &textH);
                        
                        if (lineIdx == 0) {
                            lineHeight = textH;
                        }

                        SDL_FRect textRect = {
                            (xPos - textW/2),
                            currentY,
                            textW,
                            textH
                        };

                        SDL_RenderTexture(state.renderer, textTexture, nullptr, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_DestroySurface(textSurface);
                    currentY += lineHeight + 2.0f; // Add spacing between lines
                }
            }
            
            // Render card illustration in the center
            SDL_Texture* cardIllTexture = nullptr;
            string recipeName = recipes[i].name;
            
            // Determine which texture to use based on recipe name
            if (recipeName.find("Multiple") != string::npos) {
                cardIllTexture = ill_multiple;
            } else if (recipeName.find("Endless") != string::npos) {
                cardIllTexture = ill_infinity_cracking;
            } else if (recipeName.find("Frying") != string::npos) {
                cardIllTexture = ill_cooking;
            } else if (recipeName.find("Mixing") != string::npos) {
                cardIllTexture = ill_mixing;
            } else if (recipeName.find("Cutting") != string::npos) {
                cardIllTexture = ill_cutting;
            } else if (recipeName.find("Cracking") != string::npos) {
                cardIllTexture = ill_cracking;
            }
            
            // Render the illustrations!!!
            if (cardIllTexture) {
                // Illustration size depends on if card is highlighted
                float illWidth = (i == selectedRecipeIndex) ? 400.0f: 250.0f;
                float illHeight = (i == selectedRecipeIndex) ? 225.0f: 155.0f;
                
                SDL_FRect illRect = {
                    (xPos - illWidth / 2),
                    (centerY - scaledHeight / 2) + scaledHeight * 0.250f,
                    illWidth,
                    illHeight
                };
                
                SDL_RenderTexture(state.renderer, cardIllTexture, nullptr, &illRect);
            }
        }//End of card render loop

        //Render select button if not switching cards
        if (!isCarouselAnimating()) {
            // Update button fade-in alpha
            uint32_t elapsedMs = SDL_GetTicks() - buttonFadeStartTick;
            if (elapsedMs < BUTTON_FADE_DURATION_MS) {
                buttonFade = (float)elapsedMs / (float)BUTTON_FADE_DURATION_MS;
            } else {
                buttonFade = 1.0f;
            }
            
            // Set button opacity for rendering
            uint8_t opacity = (uint8_t)(buttonFade * 255);
            SDL_SetTextureAlphaMod(selectTexture, opacity);
            selectButton.render(state);
            SDL_SetTextureAlphaMod(selectTexture, 255);  // Reset to full opacity
        }
    }
}

void LevelManager::update() {
    // Reset button fade when carousel animation stops
    if (!isCarouselAnimating() && buttonFadeStartTick == 0) {
        buttonFadeStartTick = SDL_GetTicks();
    }
    if (recipeStarted && currentMinigame != nullptr) {
        if (playStartAnimation || playFinishAnimation) {
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
            playFinishAnimation = true;
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
        if (!playStartAnimation && !playFinishAnimation) {
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
    }
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

    Recipe eggTest;
    eggTest.name = "Egg Cracking Test";
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
    eggEndless.name = "Endless Egg Test";
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
    multipleTest.name = "Multiple Minigames Test";
    multipleTest.difficulty = 1;
    multipleTest.description = "Test recipe for chaining minigames";

    multipleTest.steps.push_back(cut);
    multipleTest.steps.push_back(fry);
    recipes.push_back(multipleTest);
}

void LevelManager::onSelectClick()
{
    cout << "Select Button Clicked! Recipe Index: " << selectedRecipeIndex << endl;
    currentRecipe = &recipes[selectedRecipeIndex];
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
