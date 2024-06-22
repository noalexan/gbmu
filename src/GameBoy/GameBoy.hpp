#pragma once

#include <vector>
#include <GameBoy/MMU/MMU.hpp>
#include <GameBoy/CPU/CPU.hpp>
#include <GameBoy/PPU/PPU.hpp>
#include <GameBoy/APU/APU.hpp>

class GameBoy
{
public:
	GameBoy(char const *path_to_rom, char const *path_to_bios);
	~GameBoy();

	void run();

private:
	friend MMU;
	friend CPU;
	friend PPU;
	friend APU;

	MMU mmu;
	CPU cpu;
	PPU ppu;
	APU apu;

	std::vector<u8> rom;
	std::vector<u8> bios;

	std::vector<u8> loadFile(const char *path);

	bool _running;
};
