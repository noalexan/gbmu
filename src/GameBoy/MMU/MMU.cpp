#include "MMU.hpp"
#include "../GameBoy.hpp"
#include "bios.h"
#include <iomanip>
#include <iostream>

MMU::MMU(GameBoy &gb) : gameboy(gb)
{
	read_handlers.fill(nullptr);
	write_handlers.fill(nullptr);

	for (u16 addr = 0x0000; addr <= 0x7fff; addr++) {
		register_handler(
		    addr, [this, addr]() { return gameboy.getCartridge().read(addr); },
		    [this, addr](u8 value) { gameboy.getCartridge().write(addr, value); });
	}
	for (u16 addr = 0xa000; addr <= 0xbfff; addr++) {
		register_handler(
		    addr, [this, addr]() { return gameboy.getCartridge().read(addr); },
		    [this, addr](u8 value) { gameboy.getCartridge().write(addr, value); });
	}
	for (u16 addr = 0xc000; addr <= 0xcfff; addr++) {
		register_handler(
		    addr, [this, addr]() { return wram[addr - 0xc000]; },
		    [this, addr](u8 value) { wram[addr - 0xc000] = value; });
	}
	for (u16 addr = 0xd000; addr <= 0xdfff; addr++) {
		register_handler(
		    addr, [this, addr]() { return wram[addr - 0xc000]; },
		    [this, addr](u8 value) { wram[addr - 0xc000] = value; });
	}
	for (u16 addr = 0xe000; addr <= 0xefff; addr++) {
		register_handler(
		    addr, [this, addr]() { return wram[addr - 0xe000]; },
		    [this, addr](u8 value) { wram[addr - 0xe000] = value; });
	}
	for (u16 addr = 0xf000; addr <= 0xfdff; addr++) {
		register_handler(
		    addr, [this, addr]() { return wram[addr - 0xe000]; },
		    [this, addr](u8 value) { wram[addr - 0xe000] = value; });
	}
	register_handler(
	    0xff50, [this]() { return bios_disabled; }, [this](u8 value) { bios_disabled = value; });
	for (u16 addr = 0xff80; addr <= 0xfffe; addr++) {
		register_handler(
		    addr, [this, addr]() { return hram[addr - 0xff80]; },
		    [this, addr](u8 value) { hram[addr - 0xff80] = value; });
	}
}

MMU::~MMU() {}

u8 MMU::read(u16 address)
{
#ifndef NDEBUG
	std::cout << "\033[1;36m" << "MMU access to address: 0x" << std::hex << std::setw(4)
	          << std::setfill('0') << address << std::dec << "\033[0m" << std::endl;
#endif

	if (address < 0x100 && bios_disabled == 0)
		return dmg_bios[address];

	if (read_handlers[address]) {
		return read_handlers[address]();
	}

	return 0xff;
}

void MMU::write(u16 address, u8 value)
{
	if (write_handlers[address]) {
		write_handlers[address](value);
	}
}

void MMU::register_handler(u16 address, ReadHandler read_handler, WriteHandler write_handler)
{
	read_handlers[address]  = read_handler;
	write_handlers[address] = write_handler;
}

void MMU::register_handler_range(u16 start, u16 end, ReadHandler read_handler,
                                 WriteHandler write_handler)
{
	for (u16 addr = start; addr <= end; addr++) {
		read_handlers[addr]  = read_handler;
		write_handlers[addr] = write_handler;
	}
}
