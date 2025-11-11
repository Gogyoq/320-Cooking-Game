#pragma once

#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>

using namespace std;

enum class GameState {
	MAIN_MENU,
	LEVEL_SELECT,
	PLAYING,
	RESULT
};

struct SDLState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	GameState gameState = GameState::MAIN_MENU;
	TTF_Font* font;
	int width = 1600;
	int height = 900;
	int logW = 800;
	int logH = 450;
};

struct Ingredient {
    string name;
    int quantity;
    string unit; // "cups", "grams", etc.
};

struct CookingStep {
    string action;      // "chop", "mix", "bake"
    vector<Ingredient> ingredients;
    float duration;          // # of cuts/spins/seconds to complete the task. usage depends on minigame
    float perfectWindow;     // timing tolerance
};

struct Recipe {
    string name;
    string description;
    int difficulty = 0;          // 1-5 stars
    vector<CookingStep> steps;
    int currentStep = 0;
};

