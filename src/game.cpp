#include <iostream>
#include <cstdlib>
#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include<SDL3_ttf/SDL_ttf.h>
#include "game.h"
#include "menu.h"
#include "level_manager.h"
#include "data_structs.h"

using namespace std;

Game::Game()
{
	if (!initialize()) {
		exit(1);
	}
	startGame();
}

void Game::startGame()
{
	//Initialize menu and level manager
	Menu mainMenu(state);
	LevelManager levelManager(state);


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
			case SDL_EVENT_KEY_DOWN:
				if (event.key.key == SDLK_F11) { // Toggle fullscreen
					SDL_WindowFlags flags = SDL_GetWindowFlags(state.window);
					bool isFullscreen = flags & SDL_WINDOW_FULLSCREEN;
					SDL_SetWindowFullscreen(state.window, !isFullscreen);
				}
				break;
			}

			if (state.gameState == GameState::MAIN_MENU) {
				mainMenu.handleEvent(event);
			}
			else if (state.gameState == GameState::PLAYING) {
				levelManager.handleEvent(event);
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
				levelManager.update();
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
			levelManager.render();
		}

		//swap buffers and present
		SDL_RenderPresent(state.renderer);;
	}
	cleanup();
	exit(0);
}

bool Game::initialize() {

	bool initSuccess = true;

	//Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
		initSuccess = false;
	}

	//create the window
	state.window = SDL_CreateWindow("Cooking Mama Clone", state.width, state.height, SDL_WINDOW_RESIZABLE);
	if (!state.window) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
		cleanup();
		initSuccess = false;
	}
	else {
		SDL_SetWindowAspectRatio(state.window, 16.0f / 9.0f, 16.0f / 9.0f);
	}

	//Create the renderer
	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	SDL_SetRenderVSync(state.renderer, 1);  // Enable VSync
	if (!state.renderer) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
		cleanup();
		initSuccess = false;
	}

	//Initialize SDL_ttf
	if (!TTF_Init()) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL_ttf", state.window);
		cleanup();
		initSuccess = false;
	}

	//Load the font
	state.font = TTF_OpenFont("src/res/fonts/BloodyModes.ttf", 28);  // 32 is font size
	if (!state.font) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL_ttf", state.window);
		TTF_Quit();
		cleanup();
		initSuccess = false;
	}

	//Load the smaller font for non-selected cards
	state.fontSmall = TTF_OpenFont("src/res/fonts/BloodyModes.ttf", 20); 
	if (!state.fontSmall) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL_ttf for small font", state.window);
		TTF_Quit();
		cleanup();
		initSuccess = false;
	}

	//Configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_OVERSCAN);

	return initSuccess;
}

void Game::cleanup() {
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	TTF_CloseFont(state.font);
	TTF_CloseFont(state.fontSmall);
	SDL_Quit();
}
