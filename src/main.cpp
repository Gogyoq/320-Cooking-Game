#include <iostream>
#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<SDL3_image/SDL_image.h>

using namespace std;

struct SDLState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height, logW, logH;
};

enum class GameState {
	MAIN_MENU,
	PLAYING
};

bool initialize(SDLState& state);
void cleanup(SDLState &state);

int main(int argc, char* argv[]) {

	SDLState state;
	state.width = 1600;
	state.height = 900;
	state.logW = 640;
	state.logH = 320;

	if (!initialize(state)) {
		return 1;
	}

	GameState gameState = GameState::PLAYING;

	//load game assets

	//setup game data
	const bool* keys = SDL_GetKeyboardState(nullptr);
	float playerX = 100;
	float playerY = 100;
	
	uint64_t prevTime = SDL_GetTicks();
	int fps = 60;
	float frameInterval = 1000.0f / fps;

	//start the game loop
	bool running = true;
	while (running) {
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = nowTime - prevTime;
		prevTime = nowTime;
		float deltaTimeMultiplier = deltaTime / frameInterval;

		SDL_Event event{ 0 };
		while (SDL_PollEvent(&event)) {

			switch (event.type) {
			case SDL_EVENT_QUIT:
				running = false;
				break;
			}
		}

		if (gameState == GameState::MAIN_MENU) {

		}
		else if (gameState == GameState::PLAYING) {
			//move around (temporary for example purposes)
			float moveSpeed = 5.0f * deltaTimeMultiplier;
			if (keys[SDL_SCANCODE_A])
				playerX += -moveSpeed;
			if (keys[SDL_SCANCODE_D])
				playerX += moveSpeed;
			if (keys[SDL_SCANCODE_W])
				playerY += -moveSpeed;
			if (keys[SDL_SCANCODE_S])
				playerY += moveSpeed;

			//perform drawing commands
			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
			SDL_RenderClear(state.renderer);

			SDL_FRect player{
				.x = playerX,
				.y = playerY,
				.w = 20,
				.h = 20
			};
			SDL_SetRenderDrawColor(state.renderer, 100, 100, 180, SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(state.renderer, &player);

			//swap buffers and present
			SDL_RenderPresent(state.renderer);
		}
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
	if (!state.renderer) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
		cleanup(state);
		initSuccess = false;
	}

	//Configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	
	return initSuccess;
}

void cleanup(SDLState &state) {
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}