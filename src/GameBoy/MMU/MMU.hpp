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
	u8 is_bios_disabled;
	u8 hram[0x7E];
};

