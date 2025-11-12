#pragma once

#include <iostream>
#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include<SDL3_ttf/SDL_ttf.h>
#include "game.h"
#include "menu.h"
#include "level_manager.h"
#include "data_structs.h"

using namespace std;

class Game {
public: 
	Game();
private:
	bool initialize();
	void cleanup();
	void startGame();

	SDLState state;
	const bool* keys = SDL_GetKeyboardState(nullptr);
	const int TICKS_PER_SECOND = 50;
	const int SKIP_TICKS = 1000 / TICKS_PER_SECOND;
	const int MAX_FRAMESKIP = 10;
};
