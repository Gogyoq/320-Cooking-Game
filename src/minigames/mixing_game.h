#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <unordered_map>
#include <string>
#include <vector>
#include "minigame.h"
#include "../data_structs.h"

class MixingGame : public Minigame { //Base your minigame off of this one
public:
	MixingGame(SDLState& state, CookingStep step);
	~MixingGame();
	void render() override;
	void update() override;
	void handleEvent(const SDL_Event& event) override;
	bool isComplete() const override;
	int getScore() const override;

private:
	void loadTextures();
	SDL_Texture* getIngrTexture(SDL_Renderer* renderer, Ingredient ingr);
	SDL_FRect getAspectRatioRect(SDL_Texture* texture, const SDL_FRect& targetRect);
	bool isInBowl(float x, float y) const;
	void applyStir(float amount);
	void cleanup();
	void updateProgress();
	void finalizeScoreIfComplete();
	float getElapsedSeconds() const;
	bool hasTimeExpired() const;

	SDLState& state;
	const CookingStep step;
	Ingredient ingr;
	unordered_map<string, SDL_Texture*> textures;
	SDL_FRect bowlRect;
	SDL_FRect ingredientRect;
	SDL_FRect progressBarBG;
	SDL_FRect progressBar;
	SDL_FPoint bowlCenter;
	float bowlRadius;
	float lastAngle;
	float progress;
	bool trackingCircle;
	Uint64 startTicks;
	Uint64 completionTicks;
	int score;
};
