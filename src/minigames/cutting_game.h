#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <unordered_map>
#include <vector>
#include "minigame.h"
#include "../data_structs.h"

using namespace std;

class CuttingGame : public Minigame { //Base your minigame off of this one
public:
	CuttingGame(SDLState& state, CookingStep step);
	~CuttingGame();
	void render() override;
	void update() override;
	void handleEvent(const SDL_Event& event) override;
	bool isComplete() const override;

private:
	void spaceRectangles();
	void loadTextures();
	SDL_Texture* getIngrTexture(SDL_Renderer* renderer, Ingredient ingr);
	void cleanup();
	void onClick();
	void updateProgress();

	//Rectangles
	struct Rectangles {
		SDL_FRect destRect;
		SDL_FRect sourceRect;
	};
	SDL_FRect knifeRect;
	SDL_FRect progressBarBG; //Background rect for progress bar
	SDL_FRect progressBar; //rect for progress bar
	vector<Rectangles> ingrRects;

	SDLState& state;
	const CookingStep step;
	Ingredient ingr;
	unordered_map<string, SDL_Texture*> textures;
	uint64_t clickTime;
	bool isClicked;
	bool onCooldown;
	const int cooldownDuration = 100;
	int cutsMade = 0;
};