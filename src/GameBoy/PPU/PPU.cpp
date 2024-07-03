#include <iostream>
#include <GameBoy/GameBoy.hpp>
#include <GameBoy/PPU/PPU.hpp>

PPU::PPU(GameBoy &gb) : _gb(gb), dots(0), mode(MODE::OAM)
{
	std::cout << "new PPU" << std::endl;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}

	char *title = reinterpret_cast<char *>(&_gb.mmu.access(0x0134));
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	if (window == nullptr)
	{
		std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == nullptr)
	{
		std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);
	if (texture == nullptr)
	{
		std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
}

PPU::~PPU()
{
	std::cout << "PPU deleted" << std::endl;
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void PPU::tick()
{
	if (not(registers[0x00] & 1 << 7))
		return;

	dots++;

	switch (mode)
	{
	case MODE::HBLANK:
		if (dots >= 456)
		{
			mode = (registers[0x04] < 143) ? MODE::OAM : MODE::VBLANK;
			dots = 0;
			registers[0x04]++;
		}
		break;

	case MODE::VBLANK:
		if (dots >= 456)
		{
			if (registers[0x04] >= 153)
			{
				SDL_UpdateTexture(texture, nullptr, pixels, GB_SCREEN_WIDTH * sizeof(u32));
				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, texture, nullptr, nullptr);
				SDL_RenderPresent(renderer);

				mode = MODE::OAM;
				registers[0x04] = 0;
			}
			else
				registers[0x04]++;
			dots = 0;
		}
		break;

	case MODE::OAM:
		if (dots >= 80)
			mode = MODE::DRAW;
		break;

	case MODE::DRAW:
		if (dots >= 172)
		{
			u8 y = registers[0x04];

			// BG & Window tile data area: 0 = 8800–97FF; 1 = 8000–8FFF
			u16 tile_data_area = (registers[0x00] & (1 << 4)) ? 0x0800 : 0x1000;

			// BG tile map area: 0 = 9800–9BFF; 1 = 9C00–9FFF
			u16 tile_map_area = (registers[0x00] & (1 << 3)) ? 0x1C00 : 0x1800;

			// FF42–FF43 — SCY, SCX
			u8 scy = registers[0x02], scx = registers[0x03];

			for (u8 x = 0; x < GB_SCREEN_WIDTH; x++)
			{
				// BG & Window enable
				if (registers[0x00] & 1)
				{
					// s8 tile_index = vram[tile_map_area + (((scy + y) % 256) / 8) * 32 + (((scx + x) % 256) / 8)];
					// u8 *tile = &vram[tile_data_area + tile_index * 16];

					u8 tile[16] = {
						0x3C, 0x7E,
						0x42, 0x42,
						0x42, 0x42,
						0x42, 0x42,
						0x7E, 0x5E,
						0x7E, 0x0A,
						0x7C, 0x56,
						0x38, 0x7C
					};

					u8 tile_y = (scy + y) % 8;
					u8 tile_x = (scx + x) % 8;

					u8 a = tile[tile_y * 2];
					u8 b = tile[tile_y * 2 + 1];

					u8 color_index = (((b >> (7 - tile_x)) & 1) << 1) | ((a >> (7 - tile_x)) & 1);

					pixels[y * GB_SCREEN_WIDTH + x] = 0xFFFFFFFF - 0x33333333 * color_index;

					// switch ((registers[0x07] >> (color_index * 2)) & 0b11)
					// {
					// case 0:
					// 	pixels[y * GB_SCREEN_WIDTH + x] = 0xFFFFAAAA;
					// 	break;

					// case 1:
					// 	pixels[y * GB_SCREEN_WIDTH + x] = 0xFF181818;
					// 	break;

					// case 2:
					// 	pixels[y * GB_SCREEN_WIDTH + x] = 0xFFFE4A22;
					// 	break;

					// case 3:
					// 	pixels[y * GB_SCREEN_WIDTH + x] = 0xFF34621C;
					// 	break;
					// }
				}
			}

			mode = MODE::HBLANK;
		}

		break;
	}
}
