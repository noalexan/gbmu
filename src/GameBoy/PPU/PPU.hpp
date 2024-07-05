#pragma once

#include <utils/types.hpp>
#include <SDL2/SDL.h>

#ifndef NDEBUG

#define SCREEN_WIDTH 1160
#define SCREEN_HEIGHT 500

#else

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#endif

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
	u8 oam[0x80];

private:
	GameBoy &_gb;

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *map_texture;

	u32 pixels[GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH];

#ifndef NDEBUG
	SDL_Texture *tiles_texture;

	SDL_Rect map_rect = {10, 10, 640, 480};
	SDL_Rect tiles_rect = {670, 10, 480, 480};

	u32 tiles_pixels[128 * 128];
#endif

	u32 get_color(u8 color_index);

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
