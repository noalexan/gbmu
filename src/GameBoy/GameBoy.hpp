#pragma once

#include "APU/APU.hpp"
#include "CPU/CPU.hpp"
#include "Cartridge/Cartridge.hpp"
#include "MMU/MMU.hpp"
#include "PPU/PPU.hpp"
#include <atomic>
#include <string>
#include <thread>

class GameBoy {
private:
	APU       apu;
	PPU       ppu;
	Cartridge cartridge;
	MMU       mmu;
	CPU       cpu;

	std::thread       event_thread;
	std::atomic<bool> running{false};

	void pollEvents();

public:
	GameBoy(const std::string &);
	virtual ~GameBoy();

	void run();

	APU       &getAPU() { return apu; }
	PPU       &getPPU() { return ppu; }
	Cartridge &getCartridge() { return cartridge; }
	MMU       &getMMU() { return mmu; }
	CPU       &getCPU() { return cpu; }
};
