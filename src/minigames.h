#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <unordered_map>
#include <vector>
#include "data_structs.h"

using namespace std;

//Helper function to get textures easily
static SDL_Texture* getIngrTexture(SDL_Renderer* renderer, Ingredient ingr);

// Base class for all minigames
class Minigame {
public:
	virtual ~Minigame() = default;
	virtual void render() = 0;
	virtual void update() = 0;
	virtual void handleEvent(const SDL_Event& event) = 0;
	virtual bool isComplete() const = 0;  // Check if minigame finished
};

class CuttingGame : public Minigame {
public:
	CuttingGame(SDLState& state, Ingredient ingr);
	~CuttingGame();
	void render() override;
	void update() override;
	void handleEvent(const SDL_Event& event) override;
	bool isComplete() const override;

private:
	void loadTextures();
	void cleanup();
	void onClick();

	SDLState& state;
	Ingredient ingr;
	unordered_map<string, SDL_Texture*> textures;
	SDL_FRect knifeRect;
	SDL_FRect ingrRect;
	uint64_t clickTime;
	bool isClicked;
	bool onCooldown;
	const int cooldownDuration = 100;
};
