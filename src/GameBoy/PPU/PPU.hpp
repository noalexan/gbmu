#pragma once

#include <utils/types.hpp>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144

class GameBoy;

class PPU
{
public:
	PPU(GameBoy &);
	~PPU();

	void tick();

	u8 registers[0x0B];
	u8 vram[0x2000];

private:
	GameBoy &_gb;

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;

	u32 pixels[GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH];

	int dots;

	enum MODE
	{
		HBLANK,
		VBLANK,
		OAM,
		DRAW,
	};

	u8 mode;
};
