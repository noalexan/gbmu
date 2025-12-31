#pragma once

#include "../Cartridge/Cartridge.hpp"
#include <array>
#include <cstdint>
#include <types.h>

class GameBoy;

class MMU {
private:
	GameBoy &gameboy;

	std::array<u8 *, 0x10000> ram;

	u8 bios_disabled = 0;
	u8 wram[0x2000]; // 8KB Work RAM (0xC000-0xDFFF)
	u8 hram[0x7F];

	inline void register_address(u16 addr, u8 *ptr) { ram[addr] = ptr; }

	inline void register_address_range(u16 start, u16 end, u8 *array)
	{
		for (u16 addr = start; addr <= end; addr++)
			register_address(addr, array + addr - start);
	}

	inline void unregister_address(u16 addr) { ram[addr] = nullptr; }

	inline void unregister_address_range(u16 start, u16 end)
	{
		for (u16 addr = start; addr <= end; addr++)
			unregister_address(addr);
	}

public:
	MMU(GameBoy &);
	virtual ~MMU();

	u8 &access(u16 address);
};
