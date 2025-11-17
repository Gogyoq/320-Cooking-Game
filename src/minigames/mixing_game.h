#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <unordered_map>
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

private:
	void loadTextures();
	SDL_Texture* getIngrTexture(SDL_Renderer* renderer, Ingredient ingr);
	void cleanup();
	void updateProgress();

	SDLState& state;
	const CookingStep step;
	unordered_map<string, SDL_Texture*> textures;
	uint64_t clickTime;
	bool isClicked;
	bool onCooldown;
};