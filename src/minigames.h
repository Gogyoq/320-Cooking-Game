#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <unordered_map>
#include "data_structs.h"

using namespace std;

class CuttingGame {
public:
	CuttingGame(SDLState& state);

	void render();
	void update();
	void handleEvent(const SDL_Event& event);

	void loadTextures();
private:
	void cleanup();
	void onClick();

	SDLState& state;
	unordered_map<string, SDL_Texture*> textures;
	SDL_FRect knifeRect;
	double angle;
	uint64_t clickTime;
	bool isClicked;
	bool onCooldown;
	const int cooldownDuration = 100;
};
