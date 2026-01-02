#include "GameBoy.hpp"
#include <SDL2/SDL.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

GameBoy::GameBoy(const std::string &filename)
    : cartridge(filename), apu(), ppu(*this), mmu(*this), cpu(*this), serial(*this), timer(*this),
      joypad(*this)
{
	// Print cartridge info
	std::cout << "\033[1;33m" << cartridge.getTitle() << "\033[0m" << std::endl;
	std::cout << "  Type: " << cartridge.getCartridgeTypeString() << std::endl;
	std::cout << "  ROM Size: 0x" << std::hex << std::setw(2) << std::setfill('0')
	          << (int)cartridge.getRomSize() << std::dec << " ("
	          << cartridge.getRomDataSize() / 1024 << " KB)" << std::endl;
	std::cout << "  RAM Size: 0x" << std::hex << std::setw(2) << std::setfill('0')
	          << (int)cartridge.getRamSize() << std::dec << std::endl;
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
