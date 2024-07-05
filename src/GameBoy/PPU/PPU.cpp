#include <iostream>
#include <iomanip>
#include <GameBoy/GameBoy.hpp>
#include <GameBoy/PPU/PPU.hpp>

PPU::PPU(GameBoy &gb) : _gb(gb), dots(0), mode(MODE::OAM)
{
#ifndef NDEBUG
	std::cout << "new PPU" << std::endl;
#endif

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

	map_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);
	if (map_texture == nullptr)
	{
		std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

#ifndef NDEBUG

	tiles_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
	if (tiles_texture == nullptr)
	{
		std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyTexture(map_texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

#endif
}

PPU::~PPU()
{
#ifndef NDEBUG
	std::cout << "PPU deleted" << std::endl;

	SDL_DestroyTexture(tiles_texture);
#endif

	SDL_DestroyTexture(map_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

u32 PPU::get_color(u8 color_index)
{
	switch ((registers[0x07] >> (color_index * 2)) & 0b11)
	{
	case 0:
		return 0xFFFFAAAA;

	case 1:
		return 0xFF181818;

	case 2:
		return 0xFFFE4A22;

	default:
		return 0xFF34621C;
	}
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

				SDL_UpdateTexture(map_texture, nullptr, pixels, GB_SCREEN_WIDTH * sizeof(u32));
				SDL_RenderClear(renderer);

#ifndef NDEBUG
				for (int i = 0; i < 16; i++)
				{
					for (int j = 0; j < 16; j++)
					{
						// u8 *tile = &vram[(i * 16 + j) * 16];
						u8 *tile = (registers[0x00] & (1 << 4)) ? &vram[(i * 16 + j) * 16] : &vram[0x1000 + (s8)(i * 16 + j) * 16];

						// u8 tile[16] = {
						// 	0x3C, 0x7E,
						// 	0x42, 0x42,
						// 	0x42, 0x42,
						// 	0x42, 0x42,
						// 	0x7E, 0x5E,
						// 	0x7E, 0x0A,
						// 	0x7C, 0x56,
						// 	0x38, 0x7C
						// };

						for (int y = 0; y < 8; y++)
						{
							for (int x = 0; x < 8; x++)
							{
								u8 a = tile[y * 2];
								u8 b = tile[y * 2 + 1];

								u8 color_index = (((b >> (7 - x)) & 1) << 1) | ((a >> (7 - x)) & 1);

								// std::cout << "i: " << static_cast<int>(i) << ", j: " << static_cast<int>(j) << ", y: " << static_cast<int>(y) << ", x: " << static_cast<int>(x) << std::endl;

								tiles_pixels[(i * 8 + y) * 128 + (j * 8 + x)] = get_color(color_index);
							}
						}
					}
				}

				SDL_UpdateTexture(tiles_texture, nullptr, tiles_pixels, 128 * sizeof(u32));

				SDL_RenderCopy(renderer, map_texture, nullptr, &map_rect);
				SDL_RenderCopy(renderer, tiles_texture, nullptr, &tiles_rect);
#else
				SDL_RenderCopy(renderer, map_texture, nullptr, nullptr);
#endif

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
					u8 tile_index = vram[tile_map_area + (((scy + y) >> 3) % 32) * 32 + (((scx + x) >> 3) % 32)];
					u8 *tile = (registers[0x00] & (1 << 4)) ? &vram[tile_index * 16] : &vram[0x1000 + (s8)tile_index * 16];

					// u8 tile[16] = {
					// 	0x3C, 0x7E,
					// 	0x42, 0x42,
					// 	0x42, 0x42,
					// 	0x42, 0x42,
					// 	0x7E, 0x5E,
					// 	0x7E, 0x0A,
					// 	0x7C, 0x56,
					// 	0x38, 0x7C
					// };

					u8 tile_y = (scy + y) % 8;
					u8 tile_x = (scx + x) % 8;

					u8 a = tile[tile_y * 2];
					u8 b = tile[tile_y * 2 + 1];

					u8 color_index = (((b >> (7 - tile_x)) & 1) << 1) | ((a >> (7 - tile_x)) & 1);

					pixels[y * GB_SCREEN_WIDTH + x] = get_color(color_index);
				}
			}

			mode = MODE::HBLANK;
		}

		break;
	}
}
