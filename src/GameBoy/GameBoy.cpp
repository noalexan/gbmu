#include "GameBoy.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_scancode.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

GameBoy::GameBoy(const std::string &filename)
    : cartridge(filename), mmu(*this), apu(*this), ppu(*this), cpu(*this), serial(*this),
      timer(*this), joypad(*this)
{
	std::cerr << "\033[1;33m" << cartridge.getTitle() << "\033[0m" << std::endl
	          << "  Type: " << cartridge.getCartridgeTypeString() << std::endl
	          << "  ROM Size: 0x" << std::hex << std::setw(2) << std::setfill('0')
	          << (int)cartridge.getRomSize() << std::dec << " ("
	          << cartridge.getRomDataSize() / 1024 << " KB)" << std::endl
	          << "  RAM Size: 0x" << std::hex << std::setw(2) << std::setfill('0')
	          << (int)cartridge.getRamSize() << std::dec << " ("
	          << cartridge.getRamDataSize() / 1024 << " KB)" << std::endl;
}

GameBoy::~GameBoy()
{
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
			switch (event.type) {
			case SDL_QUIT:
				stop();
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_W:
					joypad.press(Joypad::Input::UP);
					break;
				case SDL_SCANCODE_A:
					joypad.press(Joypad::Input::LEFT);
					break;
				case SDL_SCANCODE_S:
					joypad.press(Joypad::Input::DOWN);
					break;
				case SDL_SCANCODE_D:
					joypad.press(Joypad::Input::RIGHT);
					break;
				case SDL_SCANCODE_Q:
					joypad.press(Joypad::Input::SELECT);
					break;
				case SDL_SCANCODE_E:
					joypad.press(Joypad::Input::START);
					break;
				case SDL_SCANCODE_SEMICOLON:
					joypad.press(Joypad::Input::A);
					break;
				case SDL_SCANCODE_L:
					joypad.press(Joypad::Input::B);
					break;
				case SDL_SCANCODE_SPACE:
					speedup = true;
					break;
				default:
					break;
				}
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_W:
					joypad.release(Joypad::Input::UP);
					break;
				case SDL_SCANCODE_A:
					joypad.release(Joypad::Input::LEFT);
					break;
				case SDL_SCANCODE_S:
					joypad.release(Joypad::Input::DOWN);
					break;
				case SDL_SCANCODE_D:
					joypad.release(Joypad::Input::RIGHT);
					break;
				case SDL_SCANCODE_Q:
					joypad.release(Joypad::Input::SELECT);
					break;
				case SDL_SCANCODE_E:
					joypad.release(Joypad::Input::START);
					break;
				case SDL_SCANCODE_SEMICOLON:
					joypad.release(Joypad::Input::A);
					break;
				case SDL_SCANCODE_L:
					joypad.release(Joypad::Input::B);
					break;
				case SDL_SCANCODE_SPACE:
					speedup = false;
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
	}
}

void GameBoy::stop() { running = false; }

void GameBoy::run()
{
	running               = true;
	const auto FRAME_TIME = std::chrono::milliseconds(16) / EMULATION_SPEED;

	event_thread          = std::thread(&GameBoy::pollEvents, this);

	while (running) {
		auto frame_start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < 70224; i++) {
			cpu.tick();
			ppu.tick();
			timer.tick();
		}

		ppu.render();

		auto frame_elapsed = std::chrono::high_resolution_clock::now() - frame_start;
		if (speedup == false && frame_elapsed < FRAME_TIME) {
			std::this_thread::sleep_for(FRAME_TIME - frame_elapsed);
		}
	}

	if (event_thread.joinable()) {
		event_thread.join();
	}
}
