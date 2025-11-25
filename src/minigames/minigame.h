#pragma once

#include <SDL3/SDL.h>

using namespace std;

// Base class for all minigames
class Minigame {
public:
	virtual ~Minigame() = default;
	virtual void render() = 0;
	virtual void update() = 0;
	virtual void handleEvent(const SDL_Event& event) = 0;
	virtual bool isComplete() const = 0;  // Check if minigame finished
	virtual int getScore() const { return 0; }  // Score out of 100 once complete
};
