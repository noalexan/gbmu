#include "GameBoy.hpp"
#include <SDL2/SDL.h>

GameBoy::GameBoy(const std::string &filename)
    : cartridge(filename), apu(), ppu(*this), mmu(*this), cpu(*this)
{
}

GameBoy::~GameBoy()
{
	running = false;
	if (event_thread.joinable()) {
		event_thread.join();
	}
	SDL_Quit();
}

void GameBoy::pollEvents()
{
	while (running) {
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
		}
		SDL_Delay(1);
	}
}

void GameBoy::run()
{
	running                 = true;
	const int FRAME_TIME_MS = 16;

	event_thread = std::thread(&GameBoy::pollEvents, this);

	while (running) {
		u32 frame_start = SDL_GetTicks();

		for (int i = 0; i < 70224; i++) {
			cpu.step();
			ppu.step();
		}

		ppu.render();

		u32 frame_time = SDL_GetTicks() - frame_start;
		if (frame_time < FRAME_TIME_MS) {
			SDL_Delay(FRAME_TIME_MS - frame_time);
		}
	}

	if (event_thread.joinable()) {
		event_thread.join();
	}
}
