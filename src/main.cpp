#include <iostream>
#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<SDL3_image/SDL_image.h>
#include<SDL3_ttf/SDL_ttf.h>
#include "menu.h"
#include "minigames.h"
#include "data_structs.h"

using namespace std;

bool initialize(SDLState& state);
void cleanup(SDLState &state);

int main(int argc, char* argv[]) {

	SDLState state;
	if (!initialize(state)) {
		return 1;
	}

	//load game assets
	Menu mainMenu(state);
	CuttingGame cutGame(state);

	//setup game data
	const bool* keys = SDL_GetKeyboardState(nullptr);

	const int TICKS_PER_SECOND = 50;
	const int SKIP_TICKS = 1000 / TICKS_PER_SECOND;
	const int MAX_FRAMESKIP = 10;

	uint64_t nextGameTick = SDL_GetTicks();
	int loops;

	//start the game loop
	bool running = true;
	while (running) {
		//Event Handling
		SDL_Event event{ 0 };
		while (SDL_PollEvent(&event)) {
			// Convert event coordinates to logical render coordinates
			SDL_ConvertEventToRenderCoordinates(state.renderer, &event);
			switch (event.type) {
			case SDL_EVENT_QUIT:
				running = false;
				break;
			}

			if (state.gameState == GameState::MAIN_MENU) {
				mainMenu.handleEvent(event); 
			}
		}

		//Executes TICKS_PER_SECOND times per second
		loops = 0;
		while (SDL_GetTicks() > nextGameTick && loops < MAX_FRAMESKIP) {
			//Update game logic (anything not tied to visuals, eg. physics)
			if (state.gameState == GameState::MAIN_MENU) {
				mainMenu.update();
			}
			else if (state.gameState == GameState::PLAYING) {

			}
			nextGameTick += SKIP_TICKS;
			loops++;
		}

		//Game Renderering

		//Set color to white and clear screen
		SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(state.renderer);

		if (state.gameState == GameState::MAIN_MENU) {
			mainMenu.render();
		}
		else if (state.gameState == GameState::PLAYING) {
			cutGame.render();
		}

		//swap buffers and present
		SDL_RenderPresent(state.renderer);;
	}

	cleanup(state);
	return 0;
}

bool initialize(SDLState& state) {
	
	bool initSuccess = true;

	//Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
		initSuccess = false;
	}

	//create the window
	state.window = SDL_CreateWindow("Cooking Mama Clone", state.width, state.height, 0);
	if (!state.window) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
		cleanup(state);
		initSuccess = false;
	}

	//Create the renderer
	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	SDL_SetRenderVSync(state.renderer, 1);  // Enable VSync
	if (!state.renderer) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
		cleanup(state);
		initSuccess = false;
	}

	//Initialize SDL_ttf
	if (!TTF_Init()) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL_ttf", state.window);
		cleanup(state);
		initSuccess = false;
	}

	//Load the font
	state.font = TTF_OpenFont("src/res/fonts/BloodyModes.ttf", 24);  // 24 is font size
	if (!state.font) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL_ttf", state.window);
		TTF_Quit();
		cleanup(state);
		initSuccess = false;
	}

	//Configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_OVERSCAN);
	
	return initSuccess;
}

void cleanup(SDLState &state) {
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	TTF_CloseFont(state.font);
	SDL_Quit();
}