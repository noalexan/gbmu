#include <iostream>
#include <GameBoy/GameBoy.hpp>

GameBoy::GameBoy(const std::vector<u8> &bios, const std::vector<u8> &rom) :
	_bios(bios),
	_rom(rom),
	mmu(*this),
	cpu(*this),
	ppu(*this),
	apu(*this)
{
#ifndef NDEBUG
	std::cout << "new GameBoy" << std::endl;
#endif
}

GameBoy::~GameBoy()
{
#ifndef NDEBUG
	std::cout << "GameBoy deleted" << std::endl;
#endif
}

void GameBoy::run()
{
	_running = true;
	while (_running)
	{
		cpu.tick();
		ppu.tick();
	}
}
