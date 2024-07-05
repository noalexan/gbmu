#pragma once

#include <vector>
#include <GameBoy/MMU/MMU.hpp>
#include <GameBoy/CPU/CPU.hpp>
#include <GameBoy/PPU/PPU.hpp>
#include <GameBoy/APU/APU.hpp>
#include <GameBoy/Serial/Serial.hpp>

class GameBoy
{
public:
	GameBoy(const std::vector<u8> &bios, const std::vector<u8> &rom);
	~GameBoy();

	void run();

private:
	friend MMU;
	friend CPU;
	friend PPU;
	friend APU;
	friend Serial;

	MMU mmu;
	CPU cpu;
	PPU ppu;
	APU apu;
	Serial serial;

	std::vector<u8> _bios;
	std::vector<u8> _rom;

	bool _running;
};
