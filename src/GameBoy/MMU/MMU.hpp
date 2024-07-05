#pragma once

#include <utils/types.hpp>

class GameBoy;

class MMU
{
public:
	MMU(GameBoy &);
	~MMU();

	u8 &access(u16);

private:
	GameBoy &_gb;

	u8 is_bios_disabled = 0x00;

	u8 wram[0x2000];
	u8 hram[0x7E];

	u8 not_used;
};

