#pragma once

#include <SDL2/SDL.h>
#include <types.h>

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 144
#define WINDOW_SCALE  4

class GameBoy;

class PPU {
private:
	GameBoy      &gb;
	SDL_Window   *window   = nullptr;
	SDL_Renderer *renderer = nullptr;
	SDL_Texture  *texture  = nullptr;
	u32           framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

	enum Mode { HBLANK = 0, VBLANK = 1, OAM_SEARCH = 2, PIXEL_TRANSFER = 3 };
	enum Mode mode = OAM_SEARCH;

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

	u8 lcdc = 0x91; // LCDC - LCD Control
	u8 stat = 0x00; // STAT - LCD Status
	u8 scy  = 0x00; // SCY - Scroll Y
	u8 scx  = 0x00; // SCX - Scroll X
	u8 ly   = 0x00; // LY - LCD Y-Coordinate
	u8 lyc  = 0x00; // LYC - LY Compare
	u8 dma  = 0x00; // DMA - OAM DMA Transfer
	u8 bgp  = 0xFC; // BGP - BG Palette Data
	u8 obp0 = 0xFF; // OBP0 - Object Palette 0 Data
	u8 obp1 = 0xFF; // OBP1 - Object Palette 1 Data
	u8 wy   = 0x00; // WY - Window Y Position
	u8 wx   = 0x00; // WX - Window X Position minus 7

public:
	PPU(GameBoy &);
	virtual ~PPU();

	void tick();
	void render();

	u8   read(u16 address);
	void write(u16 address, u8 value);

	SDL_Window   *getWindow() const { return window; }
	SDL_Renderer *getRenderer() const { return renderer; }
	SDL_Texture  *getTexture() const { return texture; }
	u32          *getFramebuffer() { return framebuffer; }

	u8 vram[0x2000];
	u8 oam[0xa0];
};
