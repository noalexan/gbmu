#pragma once

#include <utils/types.hpp>

class GameBoy;

class APU
{
public:
	APU(GameBoy &);
	~APU();

	u8 registers[0x16];

private:
	GameBoy &_gb;
};
