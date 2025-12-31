#include "MMU.hpp"
#include "../APU/APU.hpp"
#include "../GameBoy.hpp"
#include "../PPU/PPU.hpp"
#include "../Serial/Serial.hpp"
#include "bios.h"
#include <iomanip>
#include <iostream>

MMU::MMU(GameBoy &gb) : gameboy(gb)
{
	ram.fill(nullptr);

	register_address(0xff50, &bios_disabled);
	register_address_range(0x0000, 0x3fff, const_cast<u8 *>(gameboy.getCartridge().getRomData()));
	register_address_range(0x4000, 0x7fff, const_cast<u8 *>(gameboy.getCartridge().getRomData()));
	register_address_range(0x8000, 0x9fff, gameboy.getPPU().vram);
	register_address_range(0xa000, 0xbfff, eram);
	register_address_range(0xc000, 0xdfff, wram);
	register_address_range(0xe000, 0xfdff, wram);
	register_address_range(0xfe00, 0xfe9f, gameboy.getPPU().oam);
	register_address_range(0xfea0, 0xfeff, unusable);
	register_address_range(0xff00, 0xff7f, io_registers);
	register_address(0xff01, &gameboy.getSerial().getSerialData());
	register_address(0xff02, &gameboy.getSerial().getSerialControl());
	register_address_range(0xff10, 0xff26, gameboy.getAPU().registers);
	register_address_range(0xff40, 0xff4b, gameboy.getPPU().registers);
	register_address(0xff0f, &gameboy.getCPU().getInterruptFlags());
	register_address_range(0xff80, 0xfffe, hram);
	register_address(0xffff, &gameboy.getCPU().getInterruptEnable());
}

MMU::~MMU() {}

u8 &MMU::access(u16 address)
{
#ifndef NDEBUG
	std::cout << "\033[1;36m" << "MMU access to address: 0x" << std::hex << std::setw(4)
	          << std::setfill('0') << address << std::dec << "\033[0m" << std::endl;
#endif

	if (address < 0x100 && !bios_disabled)
		return const_cast<u8 &>(dmg_bios[address]);

	if (ram[address] == nullptr)
		throw std::runtime_error("MMU access not implemented for this address range");

	return *ram[address];
}
