#include "PPU.hpp"
#include "../GameBoy.hpp"
#include <string>

static const u32 PALETTE_COLORS[4] = {0x9BBC0FFF, 0x8BAC0FFF, 0x306230FF, 0x0F380FFF};

PPU::PPU(GameBoy &_gb) : gb(_gb)
{
	vram.fill(0);
	oam.fill(0);

	SDL_Init(SDL_INIT_VIDEO);
	std::string title = "GBMU - " + gb.getCartridge().getTitle();
	window   = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                            SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
	                            SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
	                             SCREEN_WIDTH, SCREEN_HEIGHT);

	gb.getMMU().register_handler_range(
	    0x8000, 0x9fff, [this](u16 addr) { return read_byte(addr); },
	    [this](u16 addr, u8 value) { write_byte(addr, value); });
	gb.getMMU().register_handler_range(
	    0xfe00, 0xfe9f, [this](u16 addr) { return read_byte(addr); },
	    [this](u16 addr, u8 value) { write_byte(addr, value); });
	gb.getMMU().register_handler_range(
	    0xff40, 0xff4b, [this](u16 addr) { return read_byte(addr); },
	    [this](u16 addr, u8 value) { write_byte(addr, value); });

	sprites = std::span<struct Sprite>(reinterpret_cast<struct Sprite *>(oam.data()), 0x28);
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

void PPU::perform_dma()
{
	u16 base = static_cast<u16>(dma) << 8;
	for (u8 &i : oam)
		i = gb.getMMU().read_byte(base++);
}

inline u16 PPU::compute_tile_address(u8 tile_index)
{
	if ((lcdc & LCDC::BG_TILE_DATA) == 0)
		return 0x1000 + static_cast<s8>(tile_index) * 16;
	else
		return tile_index * 16;
}

void PPU::tick()
{
	if (!(lcdc & LCDC::PPU_ENABLE)) {
		return;
	}

	cycles++;

	switch (stat & 0b11) {
	case OAM_SEARCH:
		if (cycles >= 80) {
			cycles            = 0;
			stat              = (stat & ~0b11) | PIXEL_TRANSFER;
			scanline_rendered = false;
		}
		break;

	case PIXEL_TRANSFER:
		if (!scanline_rendered) {
			u32 *scanline_ptr  = &framebuffer[ly * SCREEN_WIDTH];

			u8  *bg_tile_map   = &vram[(lcdc & LCDC::BG_TILE_MAP) ? 0x1C00 : 0x1800];
			u8  *win_tile_map  = &vram[(lcdc & LCDC::WINDOW_TILE_MAP) ? 0x1C00 : 0x1800];

			u8   bg_y          = ly + scy;
			u16  bg_tile_row   = (bg_y >> 3) << 5;
			u8   bg_line       = bg_y % 8;

			u8   win_y         = ly - wy;
			u16  win_tile_row  = (win_y >> 3) << 5;
			u8   win_line      = win_y % 8;

			bool obj_long_mode = lcdc & LCDC::OBJ_HEIGHT;

			for (u8 x = 0; x < SCREEN_WIDTH; x++) {
				u8 color_index;

				if (lcdc & LCDC::WINDOW_ENABLE && ly >= wy && x + 7 >= wx) {
					/* Window */
					u8  win_x        = x + 7 - wx;

					u8  tile_column  = win_x >> 3;
					u8  tile_index   = win_tile_map[win_tile_row + tile_column];
					u16 tile_address = compute_tile_address(tile_index);

					u8  byte1        = vram[tile_address + win_line * 2];
					u8  byte2        = vram[tile_address + win_line * 2 + 1];

					u8  bit          = 7 - (win_x % 8);
					color_index      = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
				}

				else {
					/* Background */
					u8  bg_x         = x + scx;

					u8  tile_column  = bg_x >> 3;
					u8  tile_index   = bg_tile_map[bg_tile_row + tile_column];
					u16 tile_address = compute_tile_address(tile_index);

					u8  byte1        = vram[tile_address + bg_line * 2];
					u8  byte2        = vram[tile_address + bg_line * 2 + 1];

					u8  bit          = 7 - (bg_x % 8);
					color_index      = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
				}

				u8 palette_color = (bgp >> (color_index << 1)) & 0x03;
				scanline_ptr[x]  = PALETTE_COLORS[palette_color];

				auto sprite = sprites.end();
				do {
					sprite--;

					u8 sprite_y = ly + 16 - sprite->y;
					u8 sprite_x = x + 8 - sprite->x;

					if (((sprite_y | sprite_x) & (obj_long_mode ? 0xF0 : 0xF8)) ||
					    (sprite->attr & 0x80 && color_index))
						continue;

					u16 tile_address;
					if (obj_long_mode) {
						tile_address  = (sprite->index & 0xFE) * 16;
						tile_address += ((sprite->attr & 0x40) ? (15 - sprite_y) : sprite_y) * 2;
					} else {
						tile_address  = sprite->index * 16;
						tile_address += ((sprite->attr & 0x40) ? (7 - sprite_y) : sprite_y) * 2;
					}

					u8 byte1    = vram[tile_address];
					u8 byte2    = vram[tile_address + 1];

					u8 bit      = (sprite->attr & 0x20) ? sprite_x : (7 - sprite_x);

					color_index = ((byte2 >> bit) & 1) << 1 | ((byte1 >> bit) & 1);
					if (color_index) {
						u8 palette_color =
						    (((sprite->attr & 1 << 4) ? obp1 : obp0) >> (color_index << 1)) & 0x03;
						scanline_ptr[x] = PALETTE_COLORS[palette_color];
					}
				} while (sprite != sprites.begin());
			}

			scanline_rendered = true;
		}

		if (cycles >= 172) {
			cycles = 0;
			stat   = (stat & ~0b11) | HBLANK;
			if (stat & STAT::MODE0) {
				gb.getCPU().requestInterrupt(CPU::Interrupt::LCD);
			}
		}

		break;

	case HBLANK:
		if (cycles >= 204) {
			cycles = 0;
			ly++;

			if (ly == lyc) {
				stat |= 1 << 2;
				if (stat & STAT::LYC) {
					gb.getCPU().requestInterrupt(CPU::Interrupt::LCD);
				}
			}

			else {
				stat &= ~(1 << 2);
			}

			if (ly >= SCREEN_HEIGHT) {
				gb.getCPU().requestInterrupt(CPU::Interrupt::VBLANK);
				stat = (stat & ~0b11) | VBLANK;
				if (stat & STAT::MODE1) {
					gb.getCPU().requestInterrupt(CPU::Interrupt::LCD);
				}
			} else {
				stat = (stat & ~0b11) | OAM_SEARCH;
				if (stat & STAT::MODE2) {
					gb.getCPU().requestInterrupt(CPU::Interrupt::LCD);
				}
			}
		}
		break;

	case VBLANK:
		if (cycles >= 456) {
			cycles = 0;
			ly++;

			if (ly >= 154) {
				ly   = 0;
				stat = (stat & ~0b11) | OAM_SEARCH;
			}
		}
		break;
	}
}

void PPU::render()
{
	SDL_UpdateTexture(texture, nullptr, framebuffer.data(), SCREEN_WIDTH * 4);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

u8 PPU::read_byte(u16 address)
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

void PPU::write_byte(u16 address, u8 value)
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
			stat = value & 0x78;
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
			perform_dma();
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
