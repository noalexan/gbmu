#pragma once

#include <SDL2/SDL.h>
#include <types.h>

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 144
#define WINDOW_SCALE  4

class GameBoy;

class PPU {
private:
	GameBoy &gb;
	SDL_Window   *window   = nullptr;
	SDL_Renderer *renderer = nullptr;
	SDL_Texture  *texture  = nullptr;
	u32           framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

	enum Mode { HBLANK = 0, VBLANK = 1, OAM_SEARCH = 2, PIXEL_TRANSFER = 3 } mode = OAM_SEARCH;

	enum LCDC {
		BG_ENABLE       = 1 << 0,
		OBJ_ENABLE      = 1 << 1,
		OBJ_HEIGHT      = 1 << 2,
		BG_TILE_MAP     = 1 << 3,
		BG_TILE_DATA    = 1 << 4,
		WINDOW_ENABLE   = 1 << 5,
		WINDOW_TILE_MAP = 1 << 6,
		PPU_ENABLE      = 1 << 7
	};

	int  cycles            = 0;
	bool scanline_rendered = false;

	u8 &lcdc;
	u8 &stat;
	u8 &scy;
	u8 &scx;
	u8 &ly;
	u8 &lyc;
	u8 &dma;
	u8 &bgp;
	u8 &obp0;
	u8 &obp1;
	u8 &wy;
	u8 &wx;

public:
	PPU(GameBoy &);
	virtual ~PPU();

	void step();
	void render();

	SDL_Window   *getWindow() const { return window; }
	SDL_Renderer *getRenderer() const { return renderer; }
	SDL_Texture  *getTexture() const { return texture; }
	u32          *getFramebuffer() { return framebuffer; }

	u8 vram[0x2000];
	u8 registers[0x0C];
	u8 oam[0xa0];
};
