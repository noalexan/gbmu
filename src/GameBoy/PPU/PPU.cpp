#include "PPU.hpp"
#include "../GameBoy.hpp"
#include <string>

static const u32 PALETTE_COLORS[4] = {0x9BBC0FFF, 0x8BAC0FFF, 0x306230FF, 0x0F380FFF};

PPU::PPU(GameBoy &_gb) : gb(_gb)
{
	std::fill(std::begin(vram), std::end(vram), 0);
	std::fill(std::begin(oam), std::end(oam), 0);

	SDL_Init(SDL_INIT_VIDEO);
	std::string title = "GBMU - " + gb.getCartridge().getTitle();
	window   = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                            SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
	                            SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
	                             SCREEN_WIDTH, SCREEN_HEIGHT);

	gb.getMMU().register_handler_range(
	    0x8000, 0x9fff,
	    [this](u16 addr) { return read(addr); },
	    [this](u16 addr, u8 value) { write(addr, value); });
	gb.getMMU().register_handler_range(
	    0xfe00, 0xfe9f,
	    [this](u16 addr) { return read(addr); },
	    [this](u16 addr, u8 value) { write(addr, value); });
	gb.getMMU().register_handler_range(
	    0xff40, 0xff4b,
	    [this](u16 addr) { return read(addr); },
	    [this](u16 addr, u8 value) { write(addr, value); });
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

void PPU::tick()
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

				u8 byte1 = vram[tile_address + (line << 1)];
				u8 byte2 = vram[tile_address + (line << 1) + 1];

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
				gb.getCPU().requestInterrupt(CPU::Interrupt::VBLANK);
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

u8 PPU::read(u16 address)
{
	if (address >= 0x8000 && address <= 0x9fff) {
		return vram[address - 0x8000];
	} else if (address >= 0xfe00 && address <= 0xfe9f) {
		return oam[address - 0xfe00];
	} else if (address >= 0xff40 && address <= 0xff4b) {
		switch (address) {
		case 0xff40:
			return lcdc;
		case 0xff41:
			return stat;
		case 0xff42:
			return scy;
		case 0xff43:
			return scx;
		case 0xff44:
			return ly;
		case 0xff45:
			return lyc;
		case 0xff46:
			return dma;
		case 0xff47:
			return bgp;
		case 0xff48:
			return obp0;
		case 0xff49:
			return obp1;
		case 0xff4a:
			return wy;
		case 0xff4b:
			return wx;
		}
	}
	return 0xff;
}

void PPU::write(u16 address, u8 value)
{
	if (address >= 0x8000 && address <= 0x9fff) {
		vram[address - 0x8000] = value;
	} else if (address >= 0xfe00 && address <= 0xfe9f) {
		oam[address - 0xfe00] = value;
	} else if (address >= 0xff40 && address <= 0xff4b) {
		switch (address) {
		case 0xff40:
			lcdc = value;
			break;
		case 0xff41:
			stat = value;
			break;
		case 0xff42:
			scy = value;
			break;
		case 0xff43:
			scx = value;
			break;
		case 0xff44:
			ly = value;
			break;
		case 0xff45:
			lyc = value;
			break;
		case 0xff46:
			dma = value;
			break;
		case 0xff47:
			bgp = value;
			break;
		case 0xff48:
			obp0 = value;
			break;
		case 0xff49:
			obp1 = value;
			break;
		case 0xff4a:
			wy = value;
			break;
		case 0xff4b:
			wx = value;
			break;
		}
	}
}
