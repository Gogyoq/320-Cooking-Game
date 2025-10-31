#pragma once

#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

using namespace std;

enum class GameState {
	MAIN_MENU,
	PLAYING
};

struct SDLState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	GameState gameState = GameState::MAIN_MENU;
	TTF_Font* font;
	int width = 1600;
	int height = 900;
	int logW = 640;
	int logH = 320;
};
