#include <iostream>
#include <iomanip>
#include <GameBoy/GameBoy.hpp>
#include <GameBoy/MMU/MMU.hpp>

MMU::MMU(GameBoy &gb) : _gb(gb), is_bios_disabled(0x00)
{
#ifndef NDEBUG
	std::cout << "new MMU" << std::endl;
#endif
}

MMU::~MMU()
{
#ifndef NDEBUG
	std::cout << "MMU deleted" << std::endl;
#endif
}

u8 &MMU::access(u16 address)
{
#ifndef NDEBUG
	std::cout << "MMU: Accessing 0x" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(address) << std::endl;
#endif

	if (0x0000 <= address && address <= 0x00FF && !is_bios_disabled)
		return _gb._bios[address];

	if (0x0000 <= address && address <= 0x3FFF)
		return _gb._rom[address];

	if (0x8000 <= address && address <= 0x9FFF)
		return _gb.ppu.vram[address - 0x8000];

	if (0xFF10 <= address && address <= 0xFF26)
		return _gb.apu.registers[address - 0xFF10];

	if (0xFF40 <= address && address <= 0xFF4B)
		return _gb.ppu.registers[address - 0xFF40];

	if (address == 0xFF50)
		return is_bios_disabled;

	if (0xFF80 <= address && address <= 0xFFFE)
		return hram[address - 0xFF80];

	throw std::runtime_error("MMU: Out of range");
}
