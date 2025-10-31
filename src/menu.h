#pragma once

#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<SDL3_image/SDL_image.h>
#include<SDL3_ttf/SDL_ttf.h>

using namespace std;

class Menu {
public:
	void render(SDL_Renderer& renderer);
	void update();
private:

};

class Button {
public:
	void render(SDL_Renderer& renderer);
	void update();
private:

};