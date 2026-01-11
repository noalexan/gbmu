#pragma once

#include "APU/APU.hpp"
#include "CPU/CPU.hpp"
#include "Cartridge/Cartridge.hpp"
#include "Joypad/Joypad.hpp"
#include "MMU/MMU.hpp"
#include "PPU/PPU.hpp"
#include "Serial/Serial.hpp"
#include "Timer/Timer.hpp"
#include <atomic>
#include <string>
#include <thread>

class GameBoy {
private:
	Cartridge         cartridge;
	MMU               mmu;
	APU               apu;
	PPU               ppu;
	CPU               cpu;
	Serial            serial;
	Timer             timer;
	Joypad            joypad;

	std::thread       event_thread;
	std::atomic<bool> running{false};

	bool 						speedup{false};

	void              pollEvents();

public:
	GameBoy(const std::string &);
	virtual ~GameBoy();

	void       run();

	APU       &getAPU() { return apu; }
	PPU       &getPPU() { return ppu; }
	Cartridge &getCartridge() { return cartridge; }
	MMU       &getMMU() { return mmu; }
	CPU       &getCPU() { return cpu; }
	Serial    &getSerial() { return serial; }
	Timer     &getTimer() { return timer; }
	Joypad    &getJoypad() { return joypad; }
};
