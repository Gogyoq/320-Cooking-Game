#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <unordered_map>
#include <vector>
#include "minigame.h"
#include "../data_structs.h"

class FryingGame : public Minigame { //Base your minigame off of this one
public:
	FryingGame(SDLState& state, CookingStep step);
	~FryingGame();
	void render() override;
	void update() override;
	void handleEvent(const SDL_Event& event) override;
	bool isComplete() const override;

private:
	void loadTextures();
	SDL_Texture* getIngrTexture(SDL_Renderer* renderer, Ingredient ingr);
	void cleanup();
	void updateProgress();
	void updateSafeZone();
	void updateDials();
	bool inSafeZone();
	SDL_FRect getAspectRatioRect(SDL_Texture* texture, const SDL_FRect& targetRect);

	SDLState& state;
	const CookingStep step;
	Ingredient ingr;
	unordered_map<string, SDL_Texture*> textures;

	//Members for minigame functionality
	SDL_FRect gameField, safeZone, mouseRect, dialRectX, dialRectY;
	SDL_FRect progressBarBG, progressBar; //rects for progress bar
	SDL_FRect ingrRect; //rect for displaying ingredient
	uint64_t startTime, currentTime;
	float progressTime;
	double dialAngleX, dialAngleY;

	//Members for safezone logic
	float safeZoneVX, safeZoneVY;  // Velocity components
	float safeZoneSpeed;            // Current speed
	const float MAX_SPEED = 300.0f; // Maximum speed (pixels per second)
	const float BASE_SPEED = 100.0f; // Starting speed
	const float ACCELERATION = 10.0f; // Speed increase per second
};