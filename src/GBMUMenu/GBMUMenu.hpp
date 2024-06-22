#pragma once

#include <SDL2/SDL.h>

#define SCREEN_WIDTH  600
#define SCREEN_HEIGHT 200

class GBMUMenu
{
public:
	GBMUMenu();
	virtual ~GBMUMenu();

	void run();

private:
	SDL_Renderer *_renderer;
	SDL_Window   *_window;

	bool _running;

	void doInput();
	void prepareScene();
	void presentScene();
};
