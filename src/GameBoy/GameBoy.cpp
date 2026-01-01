#include "GameBoy.hpp"
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>

GameBoy::GameBoy(const std::string &filename)
    : cartridge(filename), apu(), ppu(*this), mmu(*this), cpu(*this), serial(*this), timer(*this), joypad(*this)
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
	}
}

void GameBoy::run()
{
	running               = true;
	const auto FRAME_TIME = std::chrono::milliseconds(16);

	event_thread = std::thread(&GameBoy::pollEvents, this);

	while (running) {
		auto frame_start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < 70224; i++) {
			cpu.step();
			ppu.step();
		}

		ppu.render();

		auto frame_elapsed = std::chrono::high_resolution_clock::now() - frame_start;
		if (frame_elapsed < FRAME_TIME) {
			std::this_thread::sleep_for(FRAME_TIME - frame_elapsed);
		}
	}

	if (event_thread.joinable()) {
		event_thread.join();
	}
}
