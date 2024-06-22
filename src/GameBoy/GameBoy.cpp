#include <iostream>
#include <fstream>
#include <GameBoy/GameBoy.hpp>

GameBoy::GameBoy(char const *path_to_rom, char const *path_to_bios) : mmu(*this), cpu(*this), ppu(*this), apu(*this)
{
	// std::cout << "new GameBoy" << std::endl;

	rom = loadFile(path_to_rom);
	bios = loadFile(path_to_bios);
}

GameBoy::~GameBoy()
{
	// std::cout << "GameBoy deleted" << std::endl;
}

std::vector<u8> GameBoy::loadFile(const char *path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << path << std::endl;
		return std::vector<u8>();
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<u8> buffer(size);
	if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
	{
		std::cerr << "Failed to read file: " << path << std::endl;
		return std::vector<u8>();
	}

	return buffer;
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
