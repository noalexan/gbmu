#pragma once

#include <APU.hpp>
#include <CPU.hpp>
#include <Cartridge.hpp>
#include <Joypad.hpp>
#include <MMU.hpp>
#include <PPU.hpp>
#include <Serial.hpp>
#include <Timer.hpp>
#include <atomic>
#include <string>
#include <thread>

#define EMULATION_SPEED 1

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

	bool              speedup{false};

	void              pollEvents();

public:
	GameBoy(const std::string &);
	virtual ~GameBoy();

	void       run();
	void       stop();

	void       compute_frame();

	APU       &getAPU() { return apu; }
	PPU       &getPPU() { return ppu; }
	Cartridge &getCartridge() { return cartridge; }
	MMU       &getMMU() { return mmu; }
	CPU       &getCPU() { return cpu; }
	Serial    &getSerial() { return serial; }
	Timer     &getTimer() { return timer; }
	Joypad    &getJoypad() { return joypad; }
};
