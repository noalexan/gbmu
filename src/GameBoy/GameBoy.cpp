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
	std::cout << "new GameBoy" << std::endl;
}

GameBoy::~GameBoy()
{
	std::cout << "GameBoy deleted" << std::endl;
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
