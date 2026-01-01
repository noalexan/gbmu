#include "PPU.hpp"
#include "../GameBoy.hpp"
#include <string>

static const u32 PALETTE_COLORS[4] = {0x9BBC0FFF, 0x8BAC0FFF, 0x306230FF, 0x0F380FFF};

PPU::PPU(GameBoy &gb)
    : lcdc(registers[0]), stat(registers[1]), scy(registers[2]), scx(registers[3]),
      ly(registers[4]), lyc(registers[5]), dma(registers[6]), bgp(registers[7]), obp0(registers[8]),
      obp1(registers[9]), wy(registers[10]), wx(registers[11])
{
	SDL_Init(SDL_INIT_VIDEO);
	std::string title = "GBMU - " + gb.getCartridge().getTitle();
	window   = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                            SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
	                            SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
	                             SCREEN_WIDTH, SCREEN_HEIGHT);
}

PPU::~PPU()
{
	if (texture)
		SDL_DestroyTexture(texture);
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);
}

void PPU::step()
{
	if (!(lcdc & LCDC::PPU_ENABLE)) {
		return;
	}

	cycles++;

	switch (mode) {
	case OAM_SEARCH:
		if (cycles >= 80) {
			cycles            = 0;
			mode              = PIXEL_TRANSFER;
			scanline_rendered = false;
		}
		break;

	case PIXEL_TRANSFER:
		if (!scanline_rendered) {
			u32 *scanline_ptr = &framebuffer[ly * SCREEN_WIDTH];

			u8  *bg_tile_map         = (lcdc & LCDC::BG_TILE_MAP) ? &vram[0x1C00] : &vram[0x1800];
			u8  *bg_tile_data        = (lcdc & LCDC::BG_TILE_DATA) ? &vram[0x0000] : &vram[0x1000];
			bool use_signed_indexing = !(lcdc & LCDC::BG_TILE_DATA);

			u8  scrolled_y = ly + scy;
			u16 tile_row   = (scrolled_y >> 3) << 5;
			u8  line       = scrolled_y & 0x07;

			for (u8 x = 0; x < SCREEN_WIDTH; x++) {
				u8 scrolled_x  = x + scx;
				u8 tile_column = scrolled_x >> 3;
				u8 tile_index  = bg_tile_map[tile_row + tile_column];

				u16 tile_address;
				if (use_signed_indexing) {
					tile_address = 0x1000 + static_cast<s8>(tile_index) * 16;
				} else {
					tile_address = tile_index * 16;
				}

				u8 byte1 = bg_tile_data[tile_address + (line << 1)];
				u8 byte2 = bg_tile_data[tile_address + (line << 1) + 1];

				u8 bit         = 7 - (scrolled_x & 0x07);
				u8 color_index = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);

				u8 palette_color = (bgp >> (color_index * 2)) & 0x03;
				scanline_ptr[x]  = PALETTE_COLORS[palette_color];
			}

			scanline_rendered = true;
		}

		if (cycles >= 172) {
			cycles = 0;
			mode   = HBLANK;
		}

		break;

	case HBLANK:
		if (cycles >= 204) {
			cycles = 0;
			ly++;

			if (ly >= SCREEN_HEIGHT) {
				mode = VBLANK;
			} else {
				mode = OAM_SEARCH;
			}
		}
		break;

	case VBLANK:
		if (cycles >= 456) {
			cycles = 0;
			ly++;

			if (ly >= 154) {
				ly   = 0;
				mode = OAM_SEARCH;
			}
		}
		break;
	}
}

void PPU::render()
{
	SDL_UpdateTexture(texture, nullptr, framebuffer, SCREEN_WIDTH * 4);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}
